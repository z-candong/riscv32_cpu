# PA1 - 开天辟地的篇章: 最简单的计算机

## PA1.1 在开始愉快的PA之旅之前

## PA1.2 开天辟地的篇章

## PA1.3 RTFSC
***
>**实验必做题: 理解框架代码**
>
>你需要结合上述文字理解NEMU的框架代码.  
>如果你不知道"怎么才算是看懂了框架代码", 你可以先尝试进行后面的任务. 如果发现不知道如何下手, 再回来仔细阅读这一页面.  
>理解框架代码是一个螺旋上升的过程, 不同的阶段有不同的重点. 你不必因为看不懂某些细节而感到沮丧, 更不要试图一次把所有代码全部看明白.  

***
>**实验必做题: 优美地退出**
>
>为了测试大家是否已经理解框架代码, 我们给大家设置一个练习: 如果在运行NEMU之后直接键入q退出, 你会发现终端输出了一些错误信息.  
>请分析这个错误信息是什么原因造成的, 然后尝试在NEMU中修复它.  

A:  
  

***

## PA1.4 基础设施: 简易调试器

***
>**实验必做题: 实现单步执行, 打印寄存器, 扫描内存**
>
>熟悉了NEMU的框架之后, 这些功能实现起来都很简单, 同时我们对输出的格式不作硬性规定, 就当做是熟悉GNU/Linux编程的一次练习吧.  
>NEMU默认会把单步执行的指令打印出来(这里面埋了一些坑, 你需要RTFSC看看指令是在哪里被打印的), 这样你就可以验证单步执行的效果了.  
>不知道如何下手? 嗯, 看来你需要再阅读一遍RTFSC小节的内容了. 如果你已经忘记了某些注意事项, 重新去阅读一遍也是应该的.  
***


## PA1.5 表达式求值
***
>**实现算术表达式的词法分析**
>
>你需要完成以下的内容:
>- 为算术表达式中的各种token类型添加规则, 你需要注意C语言字符串中转义字符的存在和正则表达式中元字符的功能.
>- 在成功识别出token后, 将token的信息依次记录到tokens数组中.
***

***
>**实现算术表达式的递归求值**
>
>由于ICS不是算法课, 我们已经把递归求值的思路和框架都列出来了.   
>你需要做的是理解这一思路, 然后在框架中填充相应的内容. 实现表达式求值的功能之后, p命令也就不难实现了.
***

***
>**实现表达式生成器**
>
>根据上文内容, 实现表达式生成器. 实现后, 就可以用来生成表达式求值的测试用例了.
>```
>./gen-expr 10000 > input
>```
>将会生成10000个测试用例到input文件中, 其中每行为一个测试用例, 其格式为
>```
>结果 表达式
>```
>再稍微改造一下NEMU的main()函数, 让其读入input文件中的测试表达式后, 直接调用expr(), 并与结果进行比较.
>为了容纳长表达式的求值, 你还需要对tokens数组的大小进行修改.
>随着你的程序通过越来越多的测试, 你会对你的代码越来越有信心.
***


## PA1.6 监视点

***
>**扩展表达式求值的功能**
>
>你需要实现上述BNF中列出的功能.  
>上述BNF并没有列出C语言中所有的运算符, 例如各种位运算, <=等等. ==和&&很可能在使用监视点的时候用到, 因此要求你实现它们.  
>如果你在将来的使用中发现由于缺少某一个运算符而感到使用不方便, 到时候你再考虑实现它.  
***

***
>**实现监视点池的管理**
>  
>为了使用监视点池, 你需要编写以下两个函数(你可以根据你的需要修改函数的参数和返回值):
>```
>WP* new_wp();
>void free_wp(WP *wp);
>```
>其中new_wp()从free_链表中返回一个空闲的监视点结构, free_wp()将wp归还到free_链表中, 这两个函数会作为监视点池的接口被其它函数调用.
>需要注意的是, 调用new_wp()时可能会出现没有空闲监视点结构的情况, 为了简单起见, 此时可以通过assert(0)马上终止程序.
>框架代码中定义了32个监视点结构, 一般情况下应该足够使用, 如果你需要更多的监视点结构, 你可以修改NR_WP宏的值.  
>  
>这两个函数里面都需要执行一些链表插入, 删除的操作, 对链表操作不熟悉的同学来说, 这可以作为一次链表的练习.  
***

>**实现监视点**
>
>你需要实现上文描述的监视点相关功能, 实现了表达式求值之后, 监视点实现的重点就落在了链表操作上.
>
>由于监视点的功能需要在cpu_exec()的每次循环中都进行检查, 这会对NEMU的性能带来较为明显的开销.我们可以把监视点的检查放在trace_and_difftest()中, 并用一个新的宏 CONFIG_WATCHPOINT把检查监视点的代码包起来; 然后在nemu/Kconfig中为监视点添加一个开关选项, 最后通过menuconfig打开这个选项, 从而激活监视点的功能. 当你不需要使用监视点时, 可以在menuconfig中关闭这个开关选项来提高NEMU的性能.
>
>在同一时刻触发两个以上的监视点也是有可能的, 你可以自由决定如何处理这些特殊情况, 我们对此不作硬性规定.
***


## PA1.7 如何阅读手册

***
>**实验必做题: 尝试通过目录定位关注的问题**
>
>假设你现在需要了解一个叫selector的概念, 请通过i386手册的目录确定你需要阅读手册中的哪些地方. 即使你选择的ISA并不是x86, 也可以尝试去查阅这个概念.
***
