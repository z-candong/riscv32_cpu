/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

// 全局变量声明
CPU_state cpu = {}; // CPU状态结构体实例化
uint64_t g_nr_guest_inst = 0; // 记录执行的指令数量
static uint64_t g_timer = 0; // 定时器，单位为微秒
static bool g_print_step = false; // 是否打印每一步执行信息

// 设备更新函数声明
void device_update();

// 追踪和差异测试函数
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); } // 根据条件记录日志
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); } // 如果开启步进模式，则打印日志
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc)); // 差异测试步骤
}

// 执行一次指令
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc; // 设置当前PC值
  s->snpc = pc; // 设置下一条指令的PC值
  isa_exec_once(s); // 执行一次指令
  cpu.pc = s->dnpc; // 更新CPU PC值
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc); // 将PC值写入日志缓冲区
  int ilen = s->snpc - s->pc; // 指令长度
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst; // 获取指令字节码
#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i ++) { // x86架构下的指令字节顺序
#else
  for (i = ilen - 1; i >= 0; i --) { // 非x86架构下的指令字节顺序
#endif
    p += snprintf(p, 4, " %02x", inst[i]); // 将指令字节码写入日志缓冲区
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4); // 最大指令长度
  int space_len = ilen_max - ilen; // 计算空格长度
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte); // 反汇编函数声明
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p, // 反汇编指令
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
#endif
}

// 执行n条指令
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) { // 循环执行n条指令
    exec_once(&s, cpu.pc); // 执行一次指令
    g_nr_guest_inst ++; // 增加指令计数
    trace_and_difftest(&s, cpu.pc); // 跟踪和差异测试
    if (nemu_state.state != NEMU_RUNNING) break; // 如果状态不是运行中则退出循环
    IFDEF(CONFIG_DEVICE, device_update()); // 更新设备状态
  }
}

// 统计信息函数
static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, "")); // 设置数字格式
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64 // 数字格式宏定义
  Log("host time spent = " NUMBERIC_FMT " us", g_timer); // 输出主机花费的时间
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst); // 输出总指令数
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer); // 输出仿真频率
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency"); // 如果时间小于1us则无法计算频率
}

// 断言失败消息处理函数
void assert_fail_msg() {
  isa_reg_display(); // 显示寄存器信息
  statistic(); // 输出统计信息
}

/* Simulate how the CPU works. */
// CPU执行函数
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT); // 判断是否需要打印每一步执行信息
  switch (nemu_state.state) { // 根据当前状态进行不同的操作
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n"); // 程序结束提示
      return;
    default: nemu_state.state = NEMU_RUNNING; // 默认情况下设置状态为运行中
  }

  uint64_t timer_start = get_time(); // 记录开始时间

  execute(n); // 执行n条指令

  uint64_t timer_end = get_time(); // 记录结束时间
  g_timer += timer_end - timer_start; // 更新定时器

  switch (nemu_state.state) { // 根据最终状态进行不同的操作
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break; // 如果仍在运行则设置状态为停止

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD, // 结束或异常终止的日志输出
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : // 异常终止
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : // 正常陷阱
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))), // 错误陷阱
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic(); // 输出统计信息
  }
}

