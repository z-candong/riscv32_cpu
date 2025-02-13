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

//nemu模拟器入口点代码：
#include <common.h> // 包含一个名为 common.h 的头文件，这个文件中可能包含了项目所需的通用声明和宏定义

// 声明了一个名为 init_monitor 的函数，该函数接受命令行参数的数量和值数组作为参数
// 这个函数的作用可能是初始化监控器（monitor）
void init_monitor(int, char *[]); //输入参数：int - 表示命令行传递给程序的参数总数； char *[] - 等价于char **，表示一个存储若干个字符指针的数组，每个字符指针代表一个命令行参数

// 声明了一个名为 am_init_monitor 的函数，没有参数
// 这个函数的作用可能是为特定目标平台（可能是某种硬件抽象层）初始化监控器
void am_init_monitor(); 

// 声明了一个名为 engine_start 的函数，没有参数
// 这个函数的作用可能是启动仿真引擎或开始执行被调试的程序
void engine_start();

// 声明了一个名为 is_exit_status_bad 的函数，没有参数
// 返回一个整数，这个函数的作用可能是检查退出状态是否为错误状态
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  // 主函数，C/C++ 程序的入口点
  // argc 是命令行参数的数量，argv 是一个指向字符串数组的指针，每个字符串代表一个命令行参数
  /* Initialize the monitor. 初始化监控器 */
#ifdef CONFIG_TARGET_AM // 如果定义了 CONFIG_TARGET_AM 宏，则调用 am_init_monitor 函数来初始化监控器
  am_init_monitor();
#else                   // 否则，调用 init_monitor 函数来初始化监控器，并传入命令行参数
  init_monitor(argc, argv);
#endif

  /* Start engine. 开启仿真引擎 */
  engine_start(); // 调用 engine_start 函数，开始运行仿真引擎或被调试的程序

  return is_exit_status_bad(); // 根据 is_exit_status_bad 函数的返回值决定程序的退出状态
                               // 如果返回非零值，表示有错误发生；如果返回零，表示正常退出
}
