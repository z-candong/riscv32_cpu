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
// 包含CPU相关的头文件

void sdb_mainloop();
// 声明sdb_mainloop函数，该函数用于接收用户命令

void engine_start() {
#ifdef CONFIG_TARGET_AM
  // 如果定义了CONFIG_TARGET_AM宏，则执行以下代码
  cpu_exec(-1);
  // 调用cpu_exec函数，传入参数-1，表示无限循环执行CPU指令
#else
  // 如果没有定义CONFIG_TARGET_AM宏，则执行以下代码
  /* Receive commands from user. */
  // 接收用户命令
  sdb_mainloop();
  // 调用sdb_mainloop函数，开始与用户的交互式会话
#endif
}




