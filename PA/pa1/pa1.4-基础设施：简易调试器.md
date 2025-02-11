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

上表中：  
(1)代表命令已实现  
(2)代表该命令相比GDB中命令做了简化处理，更改了命令的格式  
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

摘要
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
strtok()是C语言中的标准库函数。有关strtok()函数功能信息，查阅：  
```Bash
man strtok
```
strtok()函数部分功能信息如下：
```
名称
       strtok, strtok_r - 从字符串中提取标记（tokens）

库
       Standard C library (libc, -lc)  这些函数属于标准C库（Libc），可以通过链接-lc来使用

摘要
       #include <string.h>

       char *strtok(char *restrict str, const char *restrict delim);
       char *strtok_r(char *restrict str, const char *restrict delim, char **restrict saveptr);

       Feature Test Macro Requirements for glibc (see feature_test_macros(7)):

       strtok_r():
           _POSIX_C_SOURCE
               || /* glibc <= 2.19: */ _BSD_SOURCE || _SVID_SOURCE

描述
       strtok() 函数将一个字符串分解成零个或多个非空标记。在第一次调用 strtok() 时，应指定要解析的字符串作为参数 str。
       在同一字符串上进行后续调用时，str 必须为 NULL。
       参数 delim 指定了用于分隔字符串中标记的一组字节。调用者可以在每次对同一字符串进行解析时指定不同的字符串作为 delim。
       每个 strtok() 调用返回一个指向包含下一个标记的以 null 结尾字符串的指针。此字符串不包括定界符字节。
       如果没有找到更多标记，则 strtok() 返回 NULL。
       对同一字符串操作的一系列 strtok() 调用维护了一个指针，该指针确定查找下一个标记的起点。
       第一个 strtok() 调用将此指针设置为指向字符串的第一个字节。
       接下来标记的开始位置通过向前扫描直到找到下一个非定界符字节来确定。
       如果找到了这样的字节，则将其视为下一个标记的开始。如果没有找到这样的字节，则没有更多标记，并且 strtok() 将返回 NULL。
       （一个为空或仅包含定界符的字符串将在首次调用时导致 strtok() 返回 NULL。）
       每个标记的结束位置通过向前扫描直到找到下一个定界符字节或遇到终止的 null 字节 ('\0') 来确定。
       如果找到了定界符字节，则将其覆盖为 null 字节以终止当前标记，并让 strtok() 保存对随后字节的指针；
       该指针将在搜索下一个标记时使用。在这种情况下，strtok() 返回所找到标记的起始位置的指针。
       根据上述描述，连续两个或更多的定界符字节序列被视为单个定界符，而字符串开头或结尾的定界符字节则被忽略。
       换句话说，由 strtok() 返回的标记总是非空字符串。
       例如，给定字符串 "aaa;;bbb,"，并且指定分隔符字符串为 ";," 的一系列 strtok() 调用将依次返回字符串 "aaa" 和 "bbb"，然后是空指针。
       strtok_r() 是 strtok() 的可重入版本。
       参数 saveptr 是一个指向 char * 变量的指针，该变量在内部被 strtok_r() 使用以在解析相同字符串的连续调用之间保持上下文。
       在第一次调用 strtok_r() 时，str 应指向要解析的字符串，而 *saveptr 的值会被忽略（但请参见注意事项）。
       在后续调用中，str 应为 NULL，并且 saveptr（以及它所指向的缓冲区）应自上次调用以来保持不变。
       可以使用具有不同 saveptr 参数的一系列 strtok_r() 调用来并发地解析不同的字符串。

返回值
       strtok() 和 strtok_r() 函数返回一个指向下一个标记的指针，如果没有更多标记，则返回 NULL。
```
**strtok()函数功能说明：**    
  - 第一次调用:strtok(str, delim)：指定要解析的字符串 str 和分隔符 delim。  
  - 后续调用:strtok(NULL, delim)需要继续解析相同的字符串，str 必须为 NULL。  
  - 参数 delim:指定了用于分隔字符串中标记的一组字节。可以每次调用时更改分隔符。  
  - 返回值:strtok()函数返回一个指向下一个标记(token)的指针，如果没有更多标记，则返回 NULL。 
  - 内部指针:  
    - 维护一个内部指针，确定查找下一个标记的起点。  
      第一次调用设置该指针为字符串的第一个字节。  
      下一个标记的开始位置通过向前扫描直到找到下一个非定界符字节来确定。  
      如果找到了这样的字节，则将其视为下一个标记的开始；否则返回 NULL。  
      标记的结束位置通过向前扫描直到找到下一个定界符字节或遇到终止的 null 字节 ('\0') 来确定。  
      如果找到了定界符字节，则将其覆盖为 null 字节以终止当前标记，并保存对随后字节的指针。  
      连续两个或更多的定界符字节序列被视为单个定界符，而字符串开头或结尾的定界符字节则被忽略。  
      例如，给定字符串 "aaa;;bbb,"，并且指定分隔符字符串delim为 ";," 。这意味着“;”和“,”都是分隔符。  
      因此，该strtok() 调用将依次返回字符串 "aaa" 和 "bbb"，然后是空指针。  

**strtok_r()函数功能说明：**
  - 可重入版本:strtok_r(str, delim, &saveptr)：与 strtok 类似，但需要传递一个额外的 saveptr 参数。  
  - 第一次调用:strtok_r(str, delim, &saveptr)：指定要解析的字符串 str 和分隔符 delim，并初始化 saveptr。  
  - 后续调用:strtok_r(NULL, delim, &saveptr)：继续解析相同的字符串，str 必须为 NULL，saveptr 保持不变。  
  - 并发解析:可以使用具有不同 saveptr 参数的一系列 strtok_r() 调用来并发地解析不同的字符串。  
  - 返回值：strtok_r() 函数返回一个指向下一个标记的指针，如果没有更多标记，则返回 NULL。

注：
  - 可重入版本（Reentrant Version）是指函数在多线程环境中可以安全调用的版本。  
    可重入函数不会使用静态数据或全局数据来存储状态，而是通过参数传递所有必要的信息，从而避免了并发调用时的数据竞争和不一致性。
  - 在 strtok_r 函数中，saveptr 是一个指向 char* 类型变量的指针，用于保存解析过程中的上下文信息。  
    具体来说，saveptr 用于记录当前解析位置，使得 strtok_r 在多次调用之间能够保持正确的解析状态。

strtok()函数和strtok_r()函数代码示例：  
```C
#include <stdio.h>
#include <string.h>

int main() {
    // 定义第一个字符串及其分隔符
    char str1[] = "apple,banana,cherry";
    const char delim1[] = ",";

    // 定义第二个字符串及其分隔符
    char str2[] = "dog;cat;bird";
    const char delim2[] = ";";

    // 使用 strtok 解析第一个字符串
    printf("Using strtok:\n");
    // 第一次调用 strtok，指定要解析的字符串 str1 和分隔符 delim1
    char *token = strtok(str1, delim1);
    // 循环遍历所有标记
    while (token != NULL) {
        // 打印当前标记
        printf("%s\n", token);
        // 继续解析下一个标记，传入 NULL 表示继续解析相同的字符串
        token = strtok(NULL, delim1);
    }

    // 使用 strtok_r 解析第二个字符串
    printf("\nUsing strtok_r:\n");
    // 声明一个指向 char* 的指针 saveptr，用于保存解析状态
    char *saveptr;
    // 第一次调用 strtok_r，指定要解析的字符串 str2、分隔符 delim2 和 saveptr
    token = strtok_r(str2, delim2, &saveptr);
    // 循环遍历所有标记
    while (token != NULL) {
        // 打印当前标记
        printf("%s\n", token);
        // 继续解析下一个标记，传入 NULL 表示继续解析相同的字符串，并使用相同的 saveptr 保存状态
        token = strtok_r(NULL, delim2, &saveptr);
    }

    return 0;
}
```
另外，cmd_help()函数中也给出了使用strtok()的例子:
```C
static int cmd_help(char *args) {
  /* 提取第一个参数 */
  char *arg = strtok(NULL, " ");
  int i;

  /* 检查是否有提供额外的参数 */
  if (arg == NULL) {
    /* 如果没有提供额外的参数，则列出所有命令及其描述 */
    for (i = 0; i < NR_CMD; i++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  } else {
    /* 如果提供了额外的参数，则查找并显示指定命令的描述 */
    for (i = 0; i < NR_CMD; i++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0; // 找到匹配的命令后返回 0 表示成功
      }
    }
    /* 如果没有找到匹配的命令，则提示“Unknown command” */
    printf("Unknown command '%s'\n", arg);
  }
  return 0; // 返回 0 表示函数执行成功
}
```

字符串处理函数有很多，键入以下内容：  
```Bash
man 3 str<TAB><TAB>
```
其中TAB代表键盘上的TAB键.   
在这里会看到很多以str开头的函数, 如strlen(), strcpy()等函数：  
```
strcasecmp                strerrordesc_np           strict                    strpbrk                strtol
strcasestr                strerror_l                string                    strptime               strtold
strcat                    strerrorname_np           string_to_av_perm         strrchr                strtoll
strchr                    strerror_r                string_to_security_class  strsep                 strtoq
strchrnul                 strfmon                   strlen                    strsignal              strtoul
strcmp                    strfmon_l                 strncasecmp               strspn                 strtoull
strcoll                   strfromd                  strncat                   strstr                 strtoumax
strcpy                    strfromf                  strncmp                   strtod                 strtouq
strcspn                   strfroml                  strncpy                   strtof                 strverscmp
strdup                    strfry                    strndup                   strtoimax              strxfrm
strdupa                   strftime                  strndupa                  strtok                    
strerror                  strftime_l                strnlen                   strtok_r                  
```
阅读这些字符串处理函数的manual page, 了解它们的功能, 其中某些函数可帮助解析命令。  
也可自行编写字符串处理函数来解析命令。  


***
***Q：如何测试字符串处理函数？***  
*当自行编写字符串处理函数时，如何测试自己编写的字符串处理函数?（PA2中有类似问题）*  
***A：测试用例与测试框架***  
*可编写各种类型的测试用例，包括正常情况、边界情况和异常情况。编写过程中考虑不同长度的字符串、空字符串、特殊字符等情况。*  
*使用简单的测试框架或手动编写测试代码来验证函数的行为，确保每个测试用例都能独立运行并报告结果。*  
*使用断言assert宏来检查函数返回值是否符合预期，如果断言失败时会终止程序并报告错误位置。*  
*记录每个测试用例的结果，并分析失败的原因。然后逐步修复问题并重新运行测试。*  
*使用自动化测试工具来管理和运行测试用例，确保每次更改后都能快速验证功能。*  
***

另外一个推荐的字符串函数为sscanf()，它的功能和scanf()类似。  
不同的是sscanf()可以从字符串中读入格式化的内容, 使用它有时候可以很方便地实现字符串的解析.
有关sscanf()函数功能信息，查阅：
```Bash
man sscanf
```
sscanf()函数部分功能信息如下：  
```
sscanf(3)

名称
       sscanf, vsscanf - 输入字符串格式转换

库
       标准C库 (libc, -lc)

摘要
       #include <stdio.h>

       int sscanf(const char *restrict str,
                  const char *restrict format, ...);

       #include <stdarg.h>

       int vsscanf(const char *restrict str,
                  const char *restrict format, va_list ap);

   glibc功能测试宏要求（参见feature_test_macros(7)):

       vsscanf():
           _ISOC99_SOURCE || _POSIX_C_SOURCE >= 200112L

描述
       sscanf()函数族根据下面描述的format扫描格式化的输入。  
       此格式可能包含转换说明；如果有的话，这些转换的结果将存储在其后跟随的指针参数所指向的位置。
       每个指针参数必须是指向由相应的转换说明返回值适当类型的数据。  
       如果format中的转换说明数量超过指针参数的数量，则结果未定义。
       如果指针参数的数量超过format中的转换说明数量，则多余的指针参数会被评估，但除此之外会被忽略。  
       sscanf()这些函数从str指向的字符串读取其输入。  
       vsscanf()函数类似于vsprintf(3)。  
       格式字符串由一系列指令组成，这些指令描述如何处理输入字符序列。  
       如果指令处理失败，不再读取进一步的输入，并且sscanf()返回。
       一个“失败”可以是以下两种情况之一：输入失败，意味着不可用的输入字符，或者匹配失败，这意味着输入不适当（见下文）。



```
sscanf()函数示例代码：  
```C
#include <stdio.h> // 包含标准输入输出函数的定义，比如printf和sscanf
#include <stdlib.h> // 包含动态内存分配和free函数的定义
#include <errno.h> // 包含错误号变量errno的定义

int main() {
    // 定义并初始化一个字符串str，作为sscanf函数的输入源。
    // 你可以将这个字符串改成任何你想要解析的小写字母组成的字符串
    char *str = "example"; 

    char *p = NULL; // 定义一个字符指针p，并初始化为NULL，用于存储从str读取的数据
    int n; // 定义一个整数n，用于存储sscanf函数的返回值

    errno = 0; // 初始化全局变量errno为0，以确保在调用sscanf之前没有设置错误状态

    // 使用sscanf尝试从str中按照格式"%m[a-z]"读取数据到指针p指向的位置
    // "%m[a-z]"表示匹配小写a至z中的任意字符，直到遇到不匹配的字符为止，并且分配足够的内存来存储这些字符
    n = sscanf(str, "%m[a-z]", &p);

    // 根据sscanf的返回值n判断操作是否成功
    if (n == 1) { // 如果成功读取了一个字符串
        printf("read: %s\n", p); // 打印读取到的字符串
        free(p); // 释放分配给存储字符串的内存
    } else if (errno != 0) { // 如果发生了错误（通过检查errno）
        perror("sscanf"); // 打印与最近一次错误相关的描述性信息
    } else { // 如果没有发生错误但也没有匹配任何字符
        fprintf(stderr, "No matching characters\n"); // 向标准错误流打印一条消息
    }

    return 0; // 程序正常退出，返回0表示执行成功
}
```


## 单步执行

## 打印寄存器

## 扫描内存






