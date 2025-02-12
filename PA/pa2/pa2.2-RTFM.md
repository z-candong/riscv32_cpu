# RTFM
在上一小节中已经在概念上介绍了一条指令具体如何执行。 但当往TRM中添加各种高效指令的同时, 也意味着无法回避繁琐的细节.

首先需要了解指令确切的行为, 为此需要阅读生存手册中指令集相关的章节. 当前选择ISA为riscv32, 阅读手册并找到以下内容：
1. 每一条指令具体行为的描述
2. 指令opcode的编码表格

说明:  
1. RV32I每条指令具体行为描述位于非特权手册（Volume I）的“Chapter 2. RV32I Base Integer Instruction Set, Version 2.1”  
2. RV32M每条指令具体行为描述位于非特权手册（Volume I）的“Chapter 13. "M" Extension for Integer Multiplication and Division, Version 2.0”  
3. RV32E每条指令具体行为描述位于非特权手册（Volume I）的“Chapter 3. RV32E and RV64E Base Integer Instruction Sets, Version 2.0”  
4. 指令opcode的编码表格位于非特权手册（Volume I）的“Chapter 34. RV32/64G Instruction Set Listings”  
(在PA中，riscv32的客户程序只会由RV32I和RV32M两类指令组成。这得益于RISC-V指令集的设计理念: 模块化)

## RTFSC(2)
本节介绍NEMU的框架代码如何实现指令的执行.
在阅读NEMU源代码过程中，会遇到用于抽象ISA差异的大部分API。[本页面](https://ysyx.oscc.cc/docs/ics-pa/nemu-isa-api.html)对API功能进行了总结，在代码中遇到可查阅。

PA1中提到:
```
cpu_exec()又会调用execute(), 后者模拟了CPU的工作方式: 不断执行指令.
具体地, 代码将在一个for循环中不断调用exec_once()函数.
这个函数的功能就是我们在上一小节中介绍的内容: 让CPU执行当前PC指向的一条指令, 然后更新PC.
```
注：cpu_exec()位于"nemu/src/cpu/cpu-exec.c"中

cpu_exec()函数代码：
```C
/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n); // 调用execute()函数

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
```
cpu_exec()函数会调用execute()函数.  
execute()函数代码如下:  
```C
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc); // 调用exec_once()函数
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}
```
execute()函数会调用exec_once()函数.  
exec_once()函数代码如下:  
```C
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i ++) {
#else
  for (i = ilen - 1; i >= 0; i --) {
#endif
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
#endif
}
```

exec_once()接受一个Decode类型的结构体指针s.  
这个结构体用于存放在执行一条指令过程中所需的信息, 包括:  
1. 指令的PC, 下一条指令的PC等.  
2. 一些ISA相关的信息, NEMU用一个结构类型ISADecodeInfo来对这些信息进行抽象, 具体的定义在"nemu/src/isa/$ISA/include/isa-def.h"中.  

exec_once()会先把当前的PC保存到s的成员pc和snpc中, 其中s->pc就是当前指令的PC, 而s->snpc则是下一条指令的PC, 这里的snpc是"static next PC"的意思.

然后代码会调用isa_exec_once()函数(位于nemu/src/isa/$ISA/inst.c).  
这是因为执行指令的具体过程是和ISA相关的, 此处先不深究isa_exec_once()的细节.   
isa_exec_once()会随着取指的过程修改s->snpc的值, 使得从isa_exec_once()返回后s->snpc正好为下一条指令的PC.   
接下来代码将会通过s->dnpc来更新PC, 这里的dnpc是"dynamic next PC"的意思. 关于snpc和dnpc的区别, 我们会在下文进行说明.

忽略exec_once()中剩下与trace相关的代码, 返回到execute()中.   
代码会对一个用于记录客户指令的计数器加1, 然后进行一些trace和difftest相关的操作(此时先忽略).  
然后检查NEMU的状态是否为NEMU_RUNNING, 若是, 则继续执行下一条指令, 否则则退出执行指令的循环.

事实上, exec_once()函数覆盖了指令周期的所有阶段: 取指, 译码, 执行, 更新PC, 接下来看NEMU是如何实现指令周期的每一个阶段的.  



### 取指(instruction fetch, IF)


### 译码(instruction decode, ID)


### 执行(execute, EX)


### 更新PC


### 结构化程序设计


### 运行第一个C程序


### 运行更多的程序
未测试代码永远是错的, 因此需要更多的测试用例来测试你的NEMU。  
在``am-kernels/tests/cpu-tests/``目录下准备了一些简单的测试用例。在该目录下执行：  
```sh
make ARCH=$ISA-nemu ALL=xxx run
```
其中xxx为测试用例的名称(不包含.c后缀)。  
上述make run的命令最终会启动NEMU, 并运行相应的客户程序. 如果你需要使用GDB来调试NEMU运行客户程序的情况, 可以执行以下命令:  
```sh
make ARCH=$ISA-nemu ALL=xxx gdb
```
***
|***实验必做题：实现更多的指令***|
|-----------------|
|*实现更多的指令, 以通过上述测试用例.*|
|*可以自由选择按照什么顺序来实现指令.要养成尽早做测试的好习惯, 不要实现所有指令之后才进行测试.*| 
|*一般原则: 实现尽可能少的指令来进行下一次的测试.*| 
|*不需要实现所有指令的所有形式, 只需要通过这些测试即可。如果将来仍然遇到了未实现的指令, 就到时候再实现它们.*|
|*框架代码已经实现了部分指令, 但可能未编写相应的模式匹配规则.*|
|*此外, 部分函数的功能也并没有完全实现好(框架代码中已经插入了TODO()作为提示), 你还需要编写相应的功能.*|
|*由于string和hello-str还需要实现额外的内容才能运行(具体在后续小节介绍), 目前可以先使用其它测试用例进行测试.*|

注意: 根据对代码的理解和当下的需求判断需要添加哪些代码，而不是仅在出现TODO的地方写代码。
***

***
|***选做思考题：指令名对照***|
|-----------------------|
|*AT&T格式反汇编结果中的少量指令, 与手册中列出的指令名称不符, 如x86的cltd, mips32和riscv32则有不少伪指令(pseudo instruction). 除了STFW之外, 你有办法在手册中找到对应的指令吗? 如果有的话, 为什么这个办法是有效的呢?*|
***

``PA2阶段1到此结束.``
