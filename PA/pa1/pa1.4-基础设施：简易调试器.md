# PA1.4-基础设施：简易调试器
简易调试器Simple Debugger（SDB），是NEMU中一项重要基础设施。SDB功能类似GNU Debugger（GDB）。  
为提高调试效率，需要在monitor中实现一个具有如下功能的简易调试器：  
| 命令       | 格式         | 使用举例     | 说明                                                                 |
|------------|--------------|--------------|----------------------------------------------------------------------|
| 帮助(1)    | `help`       | `help`       | 打印命令的帮助信息                                                   |
| 继续运行(1)| `c`          | `c`          | 继续运行被暂停的程序                                                 |
| 退出(1)    | `q`          | `q`          | 退出NEMU                                                             |
| 单步执行   | `si [N]`     | `si 10`      | 让程序单步执行 N 条指令后暂停执行，当 N 没有给出时, 缺省为 1           |
| 打印程序状态| `info SUBCMD`| `info r`<br>`info w`| 打印寄存器状态<br>打印监视点信息                                     |
| 扫描内存(2)| `x N EXPR`   | `x 10 $esp`  | 求出表达式 EXPR 的值, 将结果作为起始内存地址, 以十六进制形式输出连续的 N 个4字节 |
| 表达式求值 | `p EXPR`     | `p $eax + 1` | 求出表达式 EXPR 的值, EXPR 支持的运算请见**调试中的表达式求值**小节     |
| 设置监视点 | `w EXPR`     | `w *0x2000`  | 当表达式 EXPR 的值发生变化时, 暂停程序执行                           |
| 删除监视点 | `d N`        | `d 2`        | 删除序号为 N 的监视点                                                |

（简易调试器SDB相关部分的代码在nemu/src/monitor/sdb/目录下）
## 解析命令
为了让简易调试器易于使用，NEMU通过readline库与用户交互，使用readline()函数从键盘上读入命令。有关readline()函数功能信息，查阅：  
```
man readline
```  
readline()函数部分功能信息如下：  
```man
READLINE(3)

名称
       readline - 从用户获取一行输入并提供编辑功能

概要
       #include <stdio.h>
       #include <readline/readline.h>
       #include <readline/history.h>

       char *
       readline (const char *prompt);

版权
       Readline 版权所有 (C) 1989-2020 自由软件基金会。

描述
       readline 将从终端读取一行，并使用提示信息 prompt 作为提示。如果提示信息 prompt 是 NULL 或者空字符串，则不发出提示。
       返回的行是通过 malloc(3) 分配的；调用者在使用完后必须释放它。返回的行已移除最后的换行符，因此只保留了行文本。
       在用户输入行时，readline 提供了编辑能力。默认情况下，行编辑命令类似于 emacs 的命令。也提供了 vi 风格的行编辑界面。
       此手册页仅描述 readline 的最基本用途。还有更多功能可用；
       参见《GNU Readline Library》和《GNU History Library》以获取更多信息。

返回值
       readline 返回读取的行文本。
       空白行返回空字符串。如果在读取行时遇到 EOF，并且该行为空，则返回 NULL。
       如果在非空行中读取到 EOF，则将其视为换行符处理。
```
***
readline()函数使用示例1：提示信息与内存释放
```C
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>  // 如果打算使用历史功能
#include <stdlib.h>  // 为使用free()函数释放内存

int main() {
    // 定义一个字符指针用于存储用户输入
    char *input;

    // 使用非空提示信息调用 readline 函数
    printf("Example with non-empty prompt:\n");
    input = readline("Please enter something: ");
    
    if (input) {  // 检查是否成功获取了输入
        if (input[0] != '\0') {  // 如果输入不是空字符串
            printf("You entered: %s\n", input);
        } else {
            printf("You entered an empty line.\n");
        }
        free(input);  // 释放由 readline 分配的内存
    } else {
        printf("No input provided, or EOF encountered.\n");
    }

    // 使用 NULL 提示信息再次调用 readline 函数
    printf("\nExample with NULL prompt (no visible prompt):\n");
    input = readline(NULL);  // 注意这里传递的是 NULL
    
    if (input) {
        if (input[0] != '\0') {
            printf("You entered: %s\n", input);
        } else {
            printf("You entered an empty line.\n");
        }
        free(input);
    } else {
        printf("No input provided, or EOF encountered.\n");
    }

    // 示例：模拟 EOF 输入的情况
    printf("\nSimulating EOF input (try pressing Ctrl+D):\n");
    input = readline("Enter something or press Ctrl+D for EOF: ");
    
    if (input) {
        if (input[0] != '\0') {
            printf("You entered: %s\n", input);
        } else {
            printf("You entered an empty line.\n");
        }
        free(input);
    } else {
        printf("EOF encountered. No input to process.\n");
    }

    return 0;
}
``` 
将以上代码复制到文件readline_test.c后保存，使用以下编译命令编译文件：  
```Bash
gcc readline_test.c -o readline_test -lreadline -lhistory
```
注：编译器在默认情况下没有链接Readline库。因此，在使用gcc编译该程序时，需要添加-lreadline选项来链接Readline库。  
   此外，如果程序还使用了历史功能（通过<readline/history.h>使用的功能），还需要添加-lhistory选项。  
编译完成后，运行生成的可执行文件：    
```
./readline_test
```
即可使用程序功能。  

说明：  
1.**非空提示信息**：首先，程序请求用户输入一些文本，并提供了一个非空的提示信息 "Please enter something: "。如果用户输入了一些文本，它将被打印出来；如果是空行，则通知用户输入为空。  
2.**NULL 提示信息**：然后，程序再次请求用户输入，但这次不显示任何提示（通过传递 NULL 给 readline）。这可以用来测试当没有提示信息时的行为。  
3.**EOF 处理**：最后，程序演示了如何处理 EOF（通常可以通过按 Ctrl+D 来触发）。如果在用户输入之前检测到 EOF，readline 将返回 NULL，表明没有输入可供处理。  
4.**内存管理**：每次调用 readline 后，检查其返回值是否为 NULL 并相应地处理。如果不是 NULL，则需要调用 free() 来释放分配给输入字符串的内存。  
***
readline()函数使用示例2：使用行编辑功能翻阅历史记录
```C
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

int main() {
    char *input;

    // Readline库函数，用于初始化历史记录（可选）
    // 它允许你在程序中使用 Readline 的历史记录特性，比如通过上下箭头键来浏览之前输入的命令。
    // 这个函数通常在你打算使用历史记录功能时调用，以确保历史记录机制被正确初始化。
    using_history();

    while (1) {
        // 使用带有提示信息的 readline 函数获取用户输入
        input = readline("Enter command: ");
        
        if (!input)
            break;  // 如果输入为 NULL，则退出循环（通常是用户按下了 Ctrl+D）

        if (*input) {
            add_history(input);  // 将输入添加到历史记录中
            
            printf("You entered: %s\n", input);
        }

        free(input);  // 释放由 readline 分配的内存
    }

    return 0;
}
```
确保已经安装了libreadline-dev开发库（对于Ubuntu/Debian系统可以通过sudo apt-get install libreadline-dev安装）。  
然后将以上代码复制到文件readline_ledit.c后保存，使用以下编译命令编译文件：
```Bash
gcc readline_ledit.c -o readline_ledit -lreadline -lhistory
```
接着运行生成的可执行文件：
```
./readline_ledit.c
```
程序运行后，可在终端看到如下提示：
```
Enter command:
```
可以输入一些命令，如hello，然后按回车键。程序将显示刚刚输入的内容：
```
You entered: hello
Enter command:
```
现在再次按下向上箭头键，会发现上次输入的hello命令重新出现在提示符后。hello命令重新出现后，还可以继续将其修改为hello world。
这展示了如何使用历史记录功能快速访问并修改之前的命令。
***
从键盘上读入命令后，NEMU需要解析该命令，然后执行相关的操作。
解析命令的目的是识别命令中的参数, 例如在si 10的命令中识别出si和10, 从而得知这是一条单步执行10条指令的命令. 
解析命令的工作是通过一系列的字符串处理函数来完成的, 例如框架代码中的strtok().
strtok()




