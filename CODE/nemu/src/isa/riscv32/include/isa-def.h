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

#ifndef __ISA_RISCV_H__    // 如果没有定义宏 __ISA_RISCV_H__
#define __ISA_RISCV_H__    // 定义宏 __ISA_RISCV_H__ 以防止重复包含此头文件

#include <common.h>        // 包含一个名为 common.h 的通用头文件，可能定义了一些基础类型和宏

// 定义了一个结构体用于存储RISC-V CPU的状态。使用MUXDEF宏根据CONFIG_RVE和CONFIG_RV64的配置选择不同的成员数量或类型。
typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];  // gpr表示通用寄存器组。如果定义了CONFIG_RVE，则gpr的数量为16（RISC-V E版本），否则为32（标准版本）
  vaddr_t pc;                              // pc代表程序计数器，即当前执行指令的地址
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);  // 根据是否定义了CONFIG_RV64来选择是riscv64_CPU_state还是riscv32_CPU_state

// 解码相关的结构体定义。用于保存解码后的指令信息。
typedef struct {
  uint32_t inst;   // inst字段用来存储原始的机器码指令
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);  // 同样地，根据是否定义了CONFIG_RV64来选择是riscv64_ISADecodeInfo还是riscv32_ISADecodeInfo

// 宏定义isa_mmu_check，用于模拟MMU(内存管理单元)检查过程。这里简单返回MMU_DIRECT，意味着直接访问模式，不涉及复杂的地址转换。
#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif  // 结束对 __ISA_RISCV_H__ 的条件编译定义
