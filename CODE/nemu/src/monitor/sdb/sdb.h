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

#ifndef __SDB_H__ // 防止头文件重复包含
#define __SDB_H__

#include <common.h> // 包含项目中定义的一些通用功能和类型定义

// 声明 expr 函数
// 参数 e 是要解析和计算的表达式字符串
// 参数 success 是一个指向布尔值的指针，用于指示表达式解析是否成功
// 返回值是计算得到的结果，类型为 word_t
word_t expr(char *e, bool *success);

#endif // 结束条件编译块

