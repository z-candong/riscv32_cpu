# PA2 - 简单复杂的机器: 冯诺伊曼计算机系统
## PA2.1 - 不停计算的机器





## PA2.2 - RTFM
***
>**实验必做题: RTFSC理解指令执行的过程**
> 
>这一小节的细节非常多, 你可能需要多次阅读讲义和代码才能理解每一处细节.
>根据往届学长学姐的反馈, 一种有效的理解方法是通过做笔记的方式来整理这些细节. 事实上, 配合GDB食用效果更佳.  
>为了避免你长时间对代码的理解没有任何进展, 我们就增加一道必答题吧:  
>
>``请整理一条指令在NEMU中的执行过程.``  
>除了nemu/src/device和nemu/src/isa/$ISA/system之外, NEMU的其它代码你都已经有能力理解了.
>因此不要觉得讲义中没有提到的文件就不需要看, 尝试尽可能地理解每一处细节吧! 在你遇到bug的时候, 这些细节就会成为帮助你调试的线索.  
***

>**实验必做题: 准备交叉编译环境**
>
>如果你选择的ISA不是x86, 你还需要准备相应的gcc和binutils, 才能正确地进行编译.
>- mips32
>``apt-get install g++-mips-linux-gnu binutils-mips-linux-gnu``
>- riscv32(64)
>``apt-get install g++-riscv64-linux-gnu binutils-riscv64-linux-gnu``
***

>**实验必做题: 运行第一个客户程序**  
>
>在NEMU中实现上文提到的指令, 具体细节请务必参考手册. 实现成功后, 在NEMU中运行客户程序dummy, 你将会看到HIT GOOD TRAP的信息. 如果你没有看到这一信息, 说明你的指令实现不正确, 你可以使用PA1中实现的简易调试器帮助你调试.  
***


>**实验必做题: 实现更多的指令**
>
>你需要实现更多的指令, 以通过上述测试用例.  
>你可以自由选择按照什么顺序来实现指令. 经过PA1的训练之后, 你应该不会实现所有指令之后才进行测试了.  
>要养成尽早做测试的好习惯, 一般原则都是"实现尽可能少的指令来进行下一次的测试". 你不需要实现所有指令的所有形式, 只需要通过这些测试即可.  
>如果将来仍然遇到了未实现的指令, 就到时候再实现它们.    
>框架代码已经实现了部分指令, 但可能未编写相应的模式匹配规则. 此外, 部分函数的功能也并没有完全实现好(框架代码中已经插入了TODO()作为提示), 你还需要编写相应的功能.  
>由于string和hello-str还需要实现额外的内容才能运行(具体在后续小节介绍), 目前可以先使用其它测试用例进行测试.  
***

除string和hello-str以外的测试用例实现:   
1.dummy.c: 增加指令 addi, jal, jalr, sw  PASS  
2.add.c: 增加指令 lw, add, sub, sltiu, beq, bne  PASS    
3.bit.c: 增加指令 sh, srai, andi, sll, and, sltu, xori, lui, or  PASS  
4.crc32.c: 增加指令 srli, xor, bgeu, slli  PASS  
5.fib.c: PASS  
6.leap-year.c: 增加指令 rem  PASS  
7.matrix-mul.c: 增加指令 mul  PASS  
8.mersenne.c: 增加指令 blt, mulh, remu, divu, srl, bltu  PASS
9.mov-c.c: PASS  
10.mul-longlong.c: FAIL    
11.pascal.c: PASS  
12.quick-sort.c: PASS  
13.select-sort.c: PASS  
14.shuixianhua.c: FAIL  
15.prime.c: PASS
16.recursion.c: FAIL
17.shift.c: FAIL
18.string.c: 待定。  
19.sum.c: PASS。
20.to-lower-case.c: PASS  
21.wanshu.c: PASS  
22.add-longlong.c: PASS
23.bubble-sort.c: PASS
24.div.c: FAIL
25.fact.c: PASS
26.goldbach.c: PASS
27.if-else.c: PASS    
28.load-store.c: FAIL   
29.max.c: PASS         
30.min3.c: PASS      
31.movsx.c: PASS  
32.sub-longlong.c: PASS        
33.switch.c: PASS  
34.unalign.c: PASS
35.hello-str.c 待定。   















## PA2.3 - 程序，运行时环境与AM
***
>**实验必做题: 阅读Makefile**
>
>abstract-machine项目的Makefile设计得非常巧妙, 你需要把它们看成一种代码来RTFSC, 从而理解它们是如何工作的.
>这样一来, 你就知道怎么编写有一定质量的Makefile了; 同时, 如果哪天Makefile出现了非预期的行为, 你就可以尝试对Makefile进行调试了. 当然, 这少不了RTFM
***
>**实验必做题: 通过批处理模式运行NEMU**
>
>我们知道, 大部分同学很可能会这么想: 反正我不阅读Makefile, 老师助教也不知道, 总觉得不看也无所谓.  
>所以在这里我们加一道必做题: 我们之前启动NEMU的时候, 每次都需要手动键入c才能运行客户程序.  
>但如果不是为了使用NEMU中的sdb, 我们其实可以节省c的键入. NEMU中实现了一个批处理模式, 可以在启动NEMU之后直接运行客户程序.  
>请你阅读NEMU的代码并合适地修改Makefile, 使得通过AM的Makefile可以默认启动批处理模式的NEMU.
> 
>你现在仍然可以跳过这道必做题, 但很快你就会感到不那么方便了.
***

## PA2.4 - 基础设施(2)
## PA2.5 - 输入输出
