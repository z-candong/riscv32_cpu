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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

// 定义一些特殊的token类型
enum {
  TK_NOTYPE = 256, // 没有特定类型的token
  TK_EQ,           // 等于号（==）

  /* TODO: Add more token types */

};

// 定义规则结构体，包含正则表达式和对应的token类型
static struct rule {
  const char *regex;   // 正则表达式字符串
  int token_type;      // 对应的token类型
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // 匹配空格
  {"\\+", '+'},         // 匹加号
  {"==", TK_EQ},        // 匹配等于号（==）
};

// 计算rules数组的长度
#define NR_REGEX ARRLEN(rules)

// 存储编译后的正则表达式
static regex_t re[NR_REGEX] = {};

/* 规则会被多次使用，因此我们只在第一次使用前编译一次。
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  // 遍历所有规则并编译它们
  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128); // 获取错误信息
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

// 定义Token结构体，包含token类型和字符串表示
typedef struct token {
  int type;       // token类型
  char str[32];   // 字符串表示
} Token;

// 存储识别出的token
static Token tokens[32] __attribute__((used)) = {};
// 当前识别出的token数量
static int nr_token __attribute__((used))  = 0;

/* 将输入字符串e分解为token
 * 返回true如果成功，否则返回false
 */
static bool make_token(char *e) {
  int position = 0; // 当前处理的位置
  int i;
  regmatch_t pmatch;

  nr_token = 0; // 初始化token数量

  // 循环直到字符串结束
  while (e[position] != '\0') {
    /* 依次尝试匹配所有的规则 */
    for (i = 0; i < NR_REGEX; i ++) {
      // 如果当前规则匹配成功且从当前位置开始匹配
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position; // 匹配子串的起始位置
        int substr_len = pmatch.rm_eo;     // 匹配子串的长度

        // 打印日志记录匹配的信息
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len; // 更新position到下一个待处理位置

        // 根据不同的token类型执行相应的操作
        switch (rules[i].token_type) {
          case '+':
            tokens[nr_token].type = rules[i].token_type; // 设置token类型
            snprintf(tokens[nr_token++].str, sizeof(tokens[nr_token - 1].str), "%.*s", substr_len, substr_start); // 设置token字符串
            break;
          case TK_EQ:
            tokens[nr_token].type = rules[i].token_type;
            snprintf(tokens[nr_token++].str, sizeof(tokens[nr_token - 1].str), "%.*s", substr_len, substr_start);
            break;
          default:
            TODO(); // 处理其他token类型
        }

        break;
      }
    }

    // 如果没有找到匹配的规则
    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/* 解析并计算表达式的值
 * 成功时设置success为true并返回结果，失败时设置success为false并返回0
 */
word_t expr(char *e, bool *success) {
  if (!make_token(e)) { // 如果无法将表达式分解为token
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
