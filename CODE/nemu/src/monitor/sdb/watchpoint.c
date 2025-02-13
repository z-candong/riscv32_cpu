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

#include "sdb.h" // 包含调试器相关功能的头文件

//定义最多可以同时存在的监视点数量
#define NR_WP 32 

// 监视点结构体定义
typedef struct watchpoint {
  int NO; // 监视点编号
  struct watchpoint *next; // 指向下一个监视点的指针

  /* TODO: Add more members if necessary */
  char expr[64]; // 表达式字符串
  word_t old_val; // 上一次表达式的值
} WP;

// 监视点池数组
static WP wp_pool[NR_WP] = {};

// 头指针和空闲指针
static WP *head = NULL, *free_ = NULL;

// 初始化监视点池
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i; // 分配编号
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]); // 设置 next 指针
    wp_pool[i].expr[0] = '\0'; // 初始化表达式字符串为空
    wp_pool[i].old_val = 0; // 初始化旧值为 0
  }

  head = NULL; // 当前没有使用的监视点
  free_ = wp_pool; // 空闲监视点链表的头节点
}

/* TODO: Implement the functionality of watchpoint */



