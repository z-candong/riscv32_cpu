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
/* 汇编代码文件 */
/* 功能：将一个二进制文件嵌入到nemu项目代码中 */

.section .rodata  // 告诉汇编器将后续的数据放入只读数据段（.rodata）。只读数据段通常包含常量字符串和其他只读数据
.globl bin_start, bin_end  // 声明了两个全局符号 bin_start 和 bin_end。这些符号可以在整个程序中被引用，而不仅是定义它们的源文件
bin_start:  // 定义了一个名为 bin_start 的标签，这个标签表示嵌入的二进制数据的开始位置
#ifdef BIN_PATH  // 预处理指令，检查是否定义了宏BIN_PATH。如果定义了，则执行接下来的指令；如果未定义则直接跳转至#endif
.incbin BIN_PATH // 汇编指令，将BIN_PATH宏指定路径下的二进制文件包含进来
#endif // 结束#ifdef预处理模块
bin_end: // 定义名为bin_end的标签，这个标签表示嵌入的二进制数据的结束位置
