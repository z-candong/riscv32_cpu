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
    g_nr_guest_inst ++; // 记录客户指令的计数器+1
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
  s->pc = pc; // 将当前pc值赋给s的成员变量pc
  s->snpc = pc; // 将当前pc值赋给s的成员变量snpc
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
exec_once()接受一个指向Decode结构体的指针s.  
Decode结构体定义位于“nemu/include/cpu/decode.h”中, 相关代码如下:  
```C
typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;
```
Decode结构体用于存放在执行一条指令过程中所需的信息, 包括:  
1. 指令的PC, 下一条指令的PC等.  
2. 一些ISA相关的信息, NEMU用一个结构类型ISADecodeInfo来对这些信息进行抽象, 具体的定义在"nemu/src/isa/$ISA/include/isa-def.h"中.   

ISADecodeInfo相关代码如下:  
类型别名ISADecodeInfo的定义位于“nemu/include/isa.h”中:  
```C
// Located at src/isa/$(GUEST_ISA)/include/isa-def.h
#include <isa-def.h>

// The macro `__GUEST_ISA__` is defined in $(CFLAGS).
// It will be expanded as "x86" or "mips32" ...
typedef concat(__GUEST_ISA__, _CPU_state) CPU_state;
typedef concat(__GUEST_ISA__, _ISADecodeInfo) ISADecodeInfo; 
```
类型别名ISADecodeInfo指向由宏``concat(__GUEST_ISA__, _ISADecodeInfo)``展开后得到的具体类型.  
其中, ``concat``宏定义位于nemu/include/macro.h中(可在/nemu下通过``grep -r ‘define.*concat’ ./``命令快速找到):  
```C
// macro concatenation
// ##是C语言预处理器中的标记粘贴操作符，可将两个标记(tokens)连接为一个单一的标记
#define concat_temp(x, y) x ## y  
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)
```
``__GUEST_ISA__``定义则位于nemu/Makefile中:
```Makefile
# Extract variabls from menuconfig
GUEST_ISA ?= $(call remove_quote,$(CONFIG_ISA))
ENGINE ?= $(call remove_quote,$(CONFIG_ENGINE))
NAME    = $(GUEST_ISA)-nemu-$(ENGINE)

CFLAGS  += $(CFLAGS_BUILD) $(CFLAGS_TRACE) -D__GUEST_ISA__=$(GUEST_ISA)
```
``-D__GUEST_ISA__=$(GUEST_ISA)``表示定义一个宏``__GUEST_ISA__``, 并将其设置为变量``$(GUEST_ISA)``的值.  
变量``$(GUEST_ISA)``的值则从Kconfig配置系统中提取.  

exec_once()会先把当前的PC保存到s的成员pc和snpc中, 其中s->pc就是当前指令的PC, 而s->snpc则是下一条指令的PC, 这里的snpc是"static next PC"的意思.

然后代码会调用isa_exec_once()函数(位于nemu/src/isa/$ISA/inst.c, 即nemu/src/isa/riscv32/inst.c).   
```C
int isa_exec_once(Decode *s) {
  s->isa.inst = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
```
这是因为执行指令的具体过程是和ISA相关的, 此处先不深究isa_exec_once()的细节.   
isa_exec_once()会随着取指的过程修改s->snpc的值, 使得从isa_exec_once()返回后s->snpc正好为下一条指令的PC.   
接下来代码将会通过s->dnpc来更新PC, 这里的dnpc是"dynamic next PC"的意思. 关于snpc和dnpc的区别, 我们会在下文进行说明.

忽略exec_once()中剩下与trace相关的代码, 返回到execute()中.   
代码会对一个用于记录客户指令的计数器加1, 然后进行一些trace和difftest相关的操作(此时先忽略).  
然后检查NEMU的状态是否为NEMU_RUNNING, 若是, 则继续执行下一条指令, 否则则退出执行指令的循环.

事实上, exec_once()函数覆盖了指令周期的所有阶段: 取指, 译码, 执行, 更新PC, 接下来看NEMU是如何实现指令周期的每一个阶段的.  

### 取指(instruction fetch, IF)
isa_exec_once()做的第一件事情就是取指令.  
```C
int isa_exec_once(Decode *s) {
  s->isa.inst = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
```
从上述isa_exec_once()代码中可见, 取指令工作由函数inst_fetch()(在nemu/include/cpu/ifetch.h中定义)实现. 具体代码如下:  
```C
#ifndef __CPU_IFETCH_H__

#include <memory/vaddr.h>

static inline uint32_t inst_fetch(vaddr_t *pc, int len) {
  uint32_t inst = vaddr_ifetch(*pc, len);
  (*pc) += len;
  return inst;
}

#endif
\
```
inst_fetch()最终会根据参数len来调用vaddr_ifetch()(在nemu/src/memory/vaddr.c中定义). vaddr_ifetch()代码如下:  
```C
word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}
```
可见, vaddr_ifetch()又会通过paddr_read()来访问物理内存中的内容.  
paddr_read()函数位于nemu/src/memory/paddr.c中, 代码如下:  
```C
word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}
```
因此, 取指操作的本质只不过就是一次内存的访问而已.

isa_exec_once()在调用inst_fetch()的时候传入了s->snpc的地址.  
因此inst_fetch()最后还会根据len来更新s->snpc, 从而让s->snpc指向下一条指令.  

### 译码(instruction decode, ID)  
在int isa_exec_once()函数执行完内部取指函数后.   
接下来代码会进入decode_exec()函数(位于nemu/src/isa/riscv32/inst.c), 它进行的是译码相关的操作. 代码如下:   
```C
static int decode_exec(Decode *s) {
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  int rd = 0; \
  word_t src1 = 0, src2 = 0, imm = 0; \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}
```
译码的目的是得到指令的操作(即操作码opcode)和操作对象(即操作数operands).  
不同ISA的opcode会出现在指令的不同位置, 只需根据指令的编码格式, 从取出的指令中识别出相应的opcode即可.  

和YEMU相比, NEMU使用一种抽象层次更高的译码方式: 模式匹配, NEMU可以通过一个模式字符串来指定指令中opcode.  
例如在riscv32中有如下模式:  
```C
INSTPAT_START();
INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U, R(rd) = s->pc + imm);
// ...
INSTPAT_END();
```
其中INSTPAT(意思是instruction pattern)是一个宏(在nemu/include/cpu/decode.h中定义), 它用于定义一条模式匹配规则. 
宏INSTPAT代码如下:  
```C
// --- pattern matching wrappers for decode ---
#define INSTPAT(pattern, ...) do { \
  uint64_t key, mask, shift; \
  pattern_decode(pattern, STRLEN(pattern), &key, &mask, &shift); \
  if ((((uint64_t)INSTPAT_INST(s) >> shift) & mask) == key) { \
    INSTPAT_MATCH(s, ##__VA_ARGS__); \
    goto *(__instpat_end); \
  } \
} while (0)
```
宏INSTPAT格式如下:  
```
INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
```
模式字符串中只允许出现4种字符:  
- 0表示相应的位只能匹配0
- 1表示相应的位只能匹配1
- ?表示相应的位可以匹配0或1
- 空格是分隔符, 只用于提升模式字符串的可读性, 不参与匹配  

指令名称在代码中仅当注释使用, 不参与宏展开; 指令类型用于后续译码过程; 而指令执行操作则是通过C代码来模拟指令执行的真正行为.  

此外, nemu/include/cpu/decode.h中还定义了宏INSTPAT_START和INSTPAT_END. 对应代码如下:  
```C
#define INSTPAT_START(name) { const void * __instpat_end = &&concat(__instpat_end_, name);
#define INSTPAT_END(name)   concat(__instpat_end_, name): ; }
```
INSTPAT又使用了另外两个宏INSTPAT_INST和INSTPAT_MATCH, 它们在nemu/src/isa/riscv32/inst.c中定义. 对应代码如下:  
```C
#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  int rd = 0; \
  word_t src1 = 0, src2 = 0, imm = 0; \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}
```
对上述代码进行宏展开并简单整理代码之后, 最后将会得到:  
```C
{ const void * __instpat_end = &&__instpat_end_;
do {
  uint64_t key, mask, shift;
  pattern_decode("??????? ????? ????? ??? ????? 00101 11", 38, &key, &mask, &shift);
  if ((((uint64_t)s->isa.inst >> shift) & mask) == key) {
    {
      int rd = 0;
      word_t src1 = 0, src2 = 0, imm = 0;
      decode_operand(s, &rd, &src1, &src2, &imm, TYPE_U);
      R(rd) = s->pc + imm;
    }
    goto *(__instpat_end);
  }
} while (0);
// ...
__instpat_end_: ; }
```
上述代码中的&&__instpat_end_使用了GCC提供的标签地址扩展功能, goto语句将会跳转到最后的__instpat_end_标签.   

此外, pattern_decode()函数在nemu/include/cpu/decode.h中定义, 它用于将模式字符串转换成3个整型变量.   
pattern_decode()函数代码如下:   
```C
// --- pattern matching mechanism ---
__attribute__((always_inline))
static inline void pattern_decode(const char *str, int len,
    uint64_t *key, uint64_t *mask, uint64_t *shift) {
  uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i) \
  if ((i) >= len) goto finish; \
  else { \
    char c = str[i]; \
    if (c != ' ') { \
      Assert(c == '0' || c == '1' || c == '?', \
          "invalid character '%c' in pattern string", c); \
      __key  = (__key  << 1) | (c == '1' ? 1 : 0); \
      __mask = (__mask << 1) | (c == '?' ? 0 : 1); \
      __shift = (c == '?' ? __shift + 1 : 0); \
    } \
  }

#define macro2(i)  macro(i);   macro((i) + 1)
#define macro4(i)  macro2(i);  macro2((i) + 2)
#define macro8(i)  macro4(i);  macro4((i) + 4)
#define macro16(i) macro8(i);  macro8((i) + 8)
#define macro32(i) macro16(i); macro16((i) + 16)
#define macro64(i) macro32(i); macro32((i) + 32)
  macro64(0);
  panic("pattern too long");
#undef macro
finish:
  *key = __key >> __shift;
  *mask = __mask >> __shift;
  *shift = __shift;
}
```
pattern_decode()函数将模式字符串中的0和1抽取到整型变量key中, mask表示key的掩码, 而shift则表示opcode距离最低位的比特数量, 用于帮助编译器进行优化.   

考虑PA1中介绍的内建客户程序中的如下指令:  
```
0x00000297   auipc t0,0
```
具体地, 上述例子中:
```C
key   = 0x17;
mask  = 0x7f;
shift = 0;
```
NEMU取指令的时候会把指令记录到s->isa.inst中, 此时指令满足上述宏展开的if语句, 表示匹配到auipc指令的编码, 因此将会进行进一步的译码操作.

刚才我们只知道了指令的具体操作(比如auipc是将当前PC值与立即数相加并写入寄存器), 但我们还是不知道操作对象(比如立即数是多少, 写入到哪个寄存器).   
为了解决这个问题, decode_exec()代码需要进行进一步的译码工作, 这是通过调用decode_operand()函数来完成的. decode_operand()代码如下:    
```C
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_N: break;
    default: panic("unsupported type = %d", type);
  }
}
```
这个函数将会根据传入的指令类型type来进行操作数的译码, 译码结果将记录到函数参数rd, src1, src2和imm中, 它们分别代表目的操作数的寄存器号码, 两个源操作数和立即数.  

此时会发现, 类似寄存器和立即数这些操作数, 其实是非常常见的操作数类型. 为了进一步实现操作数译码和指令译码的解耦, 我们对这些操作数的译码进行了抽象封装:  

- 框架代码定义了src1R()和src2R()两个辅助宏, 用于寄存器的读取结果记录到相应的操作数变量中  
- 框架代码还定义了immI等辅助宏, 用于从指令中抽取出立即数

这些宏均在nemu/src/isa/riscv32/inst.c中, 相应代码如下:  
```C
#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
```
有了这些辅助宏, 就可以用它们来方便地编写decode_operand()了, 例如RISC-V中I型指令的译码过程可以通过如下代码实现:  
```C
case TYPE_I: src1R(); immI(); break;
```
另外补充几点说明:

- decode_operand中用到了宏BITS和SEXT, 它们均在nemu/include/macro.h中定义, 分别用于位抽取和符号扩展  
- decode_operand会首先统一对目标操作数进行寄存器操作数的译码, 即调用*rd = BITS(i, 11, 7), 不同的指令类型可以视情况使用rd  
- 在模式匹配过程的最后有一条inv的规则, 表示"若前面所有的模式匹配规则都无法成功匹配, 则将该指令视为非法指令  
***
|***选作思考题: 立即数背后的故事***|
|----------------------------|
|*Motorola 68k系列的处理器都是大端架构的, 考虑以下两种情况:*|
|*1. 假设我们需要将NEMU运行在Motorola 68k的机器上(把NEMU的源代码编译成Motorola 68k的机器码)*|
|*2.假设我们需要把Motorola 68k作为一个新的ISA加入到NEMU中*|
|*在这两种情况下, 你需要注意些什么问题? 为什么会产生这些问题? 怎么解决它们?*|
|*需注意，不仅是立即数的访问, 长度大于1字节的内存访问都需要考虑类似的问题*|

***
|***选作思考题: 立即数背后的故事(2)***|
|-------------------------------|
|*mips32和riscv32的指令长度只有32位, 因此它们不能像x86那样, 把C代码中的32位常数直接编码到一条指令中.*|
|*思考一下, mips32和riscv32应该如何解决这个问题?*|
***

**注意**:  
理解宏的语义不需要手动宏展开, 直接使用GCC将编译预处理的结果输出, 即可看到宏展开结果.  
拿到宏展开结果就可以快速理解其展开后的语义, 再反过来理解相应的宏是如何一步步被展开。
最方便的做法是让GCC编译NEMU的时候顺便输出预处理的结果, ``编写Makefile实现这一功能``.    

### 执行(execute, EX)
译码阶段结束之后, 代码将会执行模式匹配规则中指定的指令执行操作, 这部分操作会用到译码的结果, 并通过C代码来模拟指令执行的真正行为.   
例如对于auipc指令, 由于译码阶段已经把U型立即数记录到操作数imm中了, 我们只需要通过R(rd) = s->pc + imm将立即数与当前PC值相加并写入目标寄存器中, 这样就完成了指令的执行.  
指令执行的阶段结束之后, decode_exec()函数将会返回0, 并一路返回到exec_once()函数中. 不过目前代码并没有使用这个返回值, 因此可以忽略它.  

### 更新PC
最后是更新PC. 更新PC的操作非常简单, 只需要把s->dnpc赋值给cpu.pc即可. 之前有提到snpc和dnpc, 现在来说明一下它们的区别.
> **静态指令和动态指令**  
> 在程序分析领域中, 静态指令是指程序代码中的指令, 动态指令是指程序运行过程中的指令. 例如对于以下指令序列
> ```
> 100: jmp 102
> 101: add
> 102: xor
> ```
> jmp指令的下一条静态指令是add指令, 而下一条动态指令则是xor指令.  

有了静态指令和动态指令这两个概念之后, 我们就可以说明snpc和dnpc的区别了: snpc是下一条静态指令, 而dnpc是下一条动态指令.   
对于顺序执行的指令, 它们的snpc和dnpc是一样的; 但对于跳转指令, snpc和dnpc就会有所不同, dnpc应该指向跳转目标的指令.   
显然, 我们应该使用s->dnpc来更新PC, 并且在指令执行的过程中正确地维护s->dnpc.  

上文已经把一条指令在NEMU中执行的流程进行了大概的介绍, 但还有少量的细节没有完全覆盖, 这些细节需要进一步理解. 

### 结构化程序设计
刚才介绍了译码过程中的一些辅助用的函数和宏, 它们的引入都是为了实现代码的解偶, 提升可维护性.   
如果指令集越复杂, 指令之间的共性特征就越多, 这意味着, 如果独立实现每条指令不同形式不同操作数宽度的译码和执行过程, 将会引入大量重复的代码.   
需要修改的时候, 所有相关代码都要分别修改, 遗漏了某一处就会造成bug, 工程维护的难度急速上升.  

一种好的做法是把译码, 执行和操作数宽度的相关代码分离开来, 实现解耦, 也就是在程序设计课上提到的结构化程序设计.   
在框架代码中, 实现译码和执行之间的解耦的是通过INSTPAT定义的模式匹配规则, 这样就可以分别编写译码和执行的内容, 然后来进行组合了:   
这样的设计可以很容易实现执行行为相同但译码方式不同的多条指令.   
***
|***实验必做题: RTFSC理解指令执行的过程***|
|-----------------------------------|
|*整理一条指令在NEMU中的执行过程.*|
|*除了nemu/src/device和nemu/src/isa/$ISA/system之外, NEMU的其它代码你都已经有能力理解了.*|
|*讲义中没有提到的文件也需要阅读, 尝试尽可能地理解每一处细节. 在遇到bug时, 这些细节会成为帮助调试的线索.*|
***
### 运行第一个C程序
首先克隆一个新的子项目am-kernels, 里面包含了一些测试程序:
```Bash
cd ics2024
bash init.sh am-kernels
```
PA2的第一个任务, 就是实现若干条指令, 使得第一个简单的C程序可以在NEMU中运行起来.  
这个简单的C程序是am-kernels/tests/cpu-tests/tests/dummy.c, 它什么都不做就直接返回了.  
***
|***实验必做题: 准备交叉编译环境***|
|----------------------------|
|*如果你选择的ISA不是x86, 你还需要准备相应的gcc和binutils, 才能正确地进行编译.*|
|*针对riscv32(64):*|
|*``apt-get install g++-riscv64-linux-gnu binutils-riscv64-linux-gnu``*|

在``am-kernels/tests/cpu-tests/``目录下键入
```Bash
make ARCH=$ISA-nemu ALL=dummy run
```
编译dummy程序, 并启动NEMU运行它.  



***


```C

```

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
|***实验必做题: 实现更多的指令***|
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
|***选做思考题: 指令名对照***|
|-----------------------|
|*AT&T格式反汇编结果中的少量指令, 与手册中列出的指令名称不符, 如x86的cltd, mips32和riscv32则有不少伪指令(pseudo instruction). 除了STFW之外, 你有办法在手册中找到对应的指令吗? 如果有的话, 为什么这个办法是有效的呢?*|
***

``PA2阶段1到此结束.``
