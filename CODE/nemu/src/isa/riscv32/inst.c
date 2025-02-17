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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

// 定义寄存器读取宏，通过索引获取通用寄存器的值
#define R(i) gpr(i)

// 定义内存读取和写入宏
#define Mr vaddr_read
#define Mw vaddr_write

// 定义指令类型枚举
enum {
  TYPE_I, TYPE_U, TYPE_S, // I型、U型、S型指令
  TYPE_N, // 无操作类型
};

// 宏用于从源寄存器rs1读取数据到src1指针指向的位置
#define src1R() do { *src1 = R(rs1); } while (0)
// 宏用于从源寄存器rs2读取数据到src2指针指向的位置
#define src2R() do { *src2 = R(rs2); } while (0)
// 宏用于提取I型指令中的立即数，并将其符号扩展到32位
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
// 宏用于提取U型指令中的立即数，并将其符号扩展到32位后左移12位
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
// 宏用于提取S型指令中的立即数，并将其符号扩展到32位
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)

// 函数用于根据指令类型解码操作数
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst; // 获取当前指令
  int rs1 = BITS(i, 19, 15); // 提取rs1字段
  int rs2 = BITS(i, 24, 20); // 提取rs2字段
  *rd     = BITS(i, 11, 7);  // 提取rd字段（目标寄存器）
  
  switch (type) {
    case TYPE_I: src1R();          immI(); break; // I型指令：读取rs1，提取立即数
    case TYPE_U:                   immU(); break; // U型指令：提取立即数
    case TYPE_S: src1R(); src2R(); immS(); break; // S型指令：读取rs1和rs2，提取立即数
    case TYPE_N: break; // 无操作类型
    default: panic("unsupported type = %d", type); // 不支持的指令类型
  }
}

// 解码并执行指令的主要函数
static int decode_exec(Decode *s) {
  s->dnpc = s->snpc; // 设置下一个程序计数器为当前指令后的地址
  
#define INSTPAT_INST(s) ((s)->isa.inst) // 宏用于获取当前指令
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  int rd = 0; \
  word_t src1 = 0, src2 = 0, imm = 0; \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \ // 根据指令类型解码操作数
  __VA_ARGS__ ; \
}

  INSTPAT_START(); // 开始匹配指令模式
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm); // AUIPC指令处理
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1)); // LBU指令处理
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2)); // SB指令处理

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // EBREAK指令处理
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc)); // 无效指令处理
  INSTPAT_END(); // 结束匹配指令模式

  R(0) = 0; // 将$zero寄存器重置为0

  return 0;
}

// 执行一次指令的核心函数
int isa_exec_once(Decode *s) {
  s->isa.inst = inst_fetch(&s->snpc, 4); // 从存储器中获取一条指令
  return decode_exec(s); // 调用解码并执行指令的函数
}
