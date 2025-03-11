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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h> // 提供固定宽度整数类型
#include <inttypes.h> // 支持格式化输入输出函数
#include <stdbool.h> // 提供布尔类型
#include <string.h> // 字符串操作函数

#include <generated/autoconf.h> // 自动生成的配置头文件
#include <macro.h> // 自定义的宏定义

// 判断目标平台
#ifdef CONFIG_TARGET_AM
#include <klib.h> // 如果定义了CONFIG_TARGET_AM，则包含<klib.h>
#else
#include <assert.h> // 断言库
#include <stdlib.h> // 标准库函数
#endif

// 判断是否启用64位物理内存模式
#if CONFIG_MBASE + CONFIG_MSIZE > 0x100000000ul
#define PMEM64 1 // 如果基地址加上大小超过0x100000000ul，则启用64位物理内存模式
#endif

// 定义无符号和有符号的机器字长类型
typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t; 
typedef MUXDEF(CONFIG_ISA64, int64_t, int32_t)  sword_t;

// 定义格式化字符串宏
#define FMT_WORD MUXDEF(CONFIG_ISA64, "0x%016" PRIx64, "0x%08" PRIx32)

// 虚拟地址类型
typedef word_t vaddr_t;

// 物理地址类型
typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
#define FMT_PADDR MUXDEF(PMEM64, "0x%016" PRIx64, "0x%08" PRIx32)

// I/O地址类型
typedef uint16_t ioaddr_t;

#include <debug.h> // 调试相关的头文件

#endif // 结束防止重复包含



