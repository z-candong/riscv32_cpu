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

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <common.h>  // 包含项目中的通用定义和配置

// 检查寄存器索引是否有效的内联函数
static inline int check_reg_idx(int idx) {
  // 如果启用了运行时检查，则使用assert确保索引在合法范围内
  IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
  return idx;  // 返回检查后的索引
}

// 宏定义，用于通过索引访问CPU的通用寄存器数组，并在访问前进行索引检查
#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

// 获取寄存器名称的内联函数
static inline const char* reg_name(int idx) {
  extern const char* regs[];  // 声明外部字符串数组，存储所有寄存器的名称
  return regs[check_reg_idx(idx)];  // 返回检查后的索引对应的寄存器名称
}

#endif  // 结束头文件保护

