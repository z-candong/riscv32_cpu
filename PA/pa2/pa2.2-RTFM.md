# RTFM
在上一小节中已经在概念上介绍了一条指令具体如何执行。 但当往TRM中添加各种高效指令的同时, 也意味着无法回避繁琐的细节.

首先需要了解指令确切的行为, 为此需要阅读生存手册中指令集相关的章节. 当前选择ISA为riscv32, 阅读手册并找到以下内容：
1. 每一条指令具体行为的描述
2. 指令opcode的编码表格

## RTFSC(2)
本节介绍NEMU的框架代码如何实现指令的执行.
在阅读NEMU源代码过程中，会遇到用于抽象ISA差异的大部分API。[本页面](https://ysyx.oscc.cc/docs/ics-pa/nemu-isa-api.html)对API功能进行了总结，在代码中遇到可查阅。

### 取指(instruction fetch, IF)


### 译码(instruction decode, ID)


### 执行(execute, EX)


### 更新PC


### 结构化程序设计


### 运行第一个C程序


### 运行更多的程序
未测试代码永远是错的, 因此需要更多的测试用例来测试你的NEMU。  
在am-kernels/tests/cpu-tests/目录下准备了一些简单的测试用例。在该目录下执行：  
```sh
make ARCH=$ISA-nemu ALL=xxx run
```
其中xxx为测试用例的名称(不包含.c后缀)。  
上述make run的命令最终会启动NEMU, 并运行相应的客户程序. 如果你需要使用GDB来调试NEMU运行客户程序的情况, 可以执行以下命令:  
```sh
make ARCH=$ISA-nemu ALL=xxx gdb
```



