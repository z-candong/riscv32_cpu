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
#include <regex.h> // 包含正则表达式处理函数的声明

// 定义一些令牌类型
enum {
  TK_NOTYPE = 256, // 无类型令牌
  TK_EQ,           // 等于号
  /* TODO: Add more token types */
};

// 规则结构体定义
static struct rule {
  const char *regex; // 正则表达式
  int token_type;    // 对应的令牌类型
} rules[] = {
  {" +", TK_NOTYPE},    // 匹配空格
  {"\\+", '+'},         // 匹配加号
  {"==", TK_EQ},        // 匹配等于号
  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
};

// 计算规则数组的长度
#define NR_REGEX ARRLEN(rules)

// 存储编译后正则表达式的数组
static regex_t re[NR_REGEX] = {};

// 初始化所有规则的正则表达式
void init_regex() {
  int i;
  char error_msg[128]; // 错误消息缓冲区
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED); // 编译正则表达式
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128); // 获取错误信息
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex); // 输出错误信息并终止程序
    }
  }
}

// 令牌结构体定义
typedef struct token {
  int type;       // 令牌类型
  char str[32];   // 字符串表示
} Token;

// 全局变量：令牌数组和当前令牌数量计数器
static Token tokens[32] __attribute__((used)) = {}; // 令牌数组
static int nr_token __attribute__((used))  = 0;     // 当前令牌数量

// 将输入字符串解析成一系列令牌
static bool make_token(char *e) {
  int position = 0; // 当前解析位置
  int i;
  regmatch_t pmatch; // 匹配结果

  nr_token = 0; // 清空令牌计数器

  while (e[position] != '\0') { // 遍历输入字符串
    for (i = 0; i < NR_REGEX; i ++) { // 尝试匹配每个规则
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) { // 成功匹配
        char *substr_start = e + position; // 匹配子字符串起始位置
        int substr_len = pmatch.rm_eo;     // 匹配子字符串长度

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start); // 日志输出

        position += substr_len; // 更新当前位置

        // 根据令牌类型执行不同操作
        switch (rules[i].token_type) {
          case '+':
            tokens[nr_token].type = '+';
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;
          default:
            // 处理其他令牌类型
            break;
        }

        break; // 跳出循环
      }
    }

    if (i == NR_REGEX) { // 没有任何规则匹配
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, ""); // 输出错误信息
      return false; // 返回错误
    }
  }

  return true; // 返回成功
}

// 解析并计算输入表达式
word_t expr(char *e, bool *success) {
  if (!make_token(e)) { // 生成令牌失败
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO(); // 计算表达式的值

  return 0;
}
