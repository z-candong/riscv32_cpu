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

#include <isa.h>  // 包含与指令集架构相关的定义
#include <cpu/cpu.h> // 包含CPU相关函数和数据结构的定义
#include <readline/readline.h> // 包含读取命令行输入的声明
#include <readline/history.h> // 包含历史记录功能的声明
#include "sdb.h" // 包含调试器（SDB）的相关定义

// 标志变量，指示是否处于批处理模式
static int is_batch_mode = false;

// 函数声明：初始化正则表达式
void init_regex();
// 函数声明：初始化监视点池
void init_wp_pool();

/* 使用 `readline' 库提供更灵活的从标准输入读取的功能 */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read); // 释放之前的输入缓冲区
    line_read = NULL;
  }

  line_read = readline("(nemu) "); // 读取一行输入

  if (line_read && *line_read) {
    add_history(line_read); // 将输入添加到历史记录
  }

  return line_read;
}

// 处理 `c` 命令，继续执行程序直到遇到断点或结束
static int cmd_c(char *args) {
  cpu_exec(-1); // 执行CPU
  return 0;
}

// 处理 `q` 命令，退出NEMU
static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;  // 设置NEMU状态为退出
  return -1;
}

// 声明 `help` 命令处理器函数
static int cmd_help(char *args);

// 命令表，包含每个命令的名称、描述和对应的处理函数
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
};

// 计算命令表的长度
#define NR_CMD ARRLEN(cmd_table)

// 实现 `help` 命令处理器函数
static int cmd_help(char *args) {
  char *arg = strtok(NULL, " "); // 提取第一个参数
  int i;

  if (arg == NULL) {
    // 没有参数给出，显示所有命令的信息
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    // 有参数给出，显示特定命令的帮助信息
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

// 设置调试器进入批处理模式
void sdb_set_batch_mode() {
  is_batch_mode = true;
}

// 调试器主循环
void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL); // 继续执行程序
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) { // 读取用户输入
    char *str_end = str + strlen(str);

    char *cmd = strtok(str, " "); // 提取命令
    if (cmd == NULL) { continue; }

    char *args = cmd + strlen(cmd) + 1; // 提取参数
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue(); // 清除SDL事件队列
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; } // 调用命令处理函数
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); } // 未知命令
  }
}

// 初始化调试器
void init_sdb() {
  init_regex(); // 初始化正则表达式
  init_wp_pool(); // 初始化监视点池
}
