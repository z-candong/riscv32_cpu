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

>**实验必做题: 准备交叉编译环境 [Done]**
>
>如果你选择的ISA不是x86, 你还需要准备相应的gcc和binutils, 才能正确地进行编译.
>- mips32
>``apt-get install g++-mips-linux-gnu binutils-mips-linux-gnu``
>- riscv32(64)
>``apt-get install g++-riscv64-linux-gnu binutils-riscv64-linux-gnu``
***

>**实验必做题: 运行第一个客户程序 [Done]**  
>
>在NEMU中实现上文提到的指令, 具体细节请务必参考手册. 实现成功后, 在NEMU中运行客户程序dummy, 你将会看到HIT GOOD TRAP的信息. 如果你没有看到这一信息, 说明你的指令实现不正确, 你可以使用PA1中实现的简易调试器帮助你调试.  
***


>**实验必做题: 实现更多的指令 [Done]**
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
10.mul-longlong.c: PASS    
11.pascal.c: PASS  
12.quick-sort.c: PASS  
13.select-sort.c: PASS    
14.shuixianhua.c: 增加指令 div  PASS    
15.prime.c: PASS  
16.recursion.c: PASS  
17.shift.c: 增加指令 sra  PASS  
18.string.c: 待定。  
19.sum.c: PASS   
20.to-lower-case.c: PASS    
21.wanshu.c: PASS   
22.add-longlong.c: PASS  
23.bubble-sort.c: PASS  
24.div.c: PASS  
25.fact.c: PASS  
26.goldbach.c: PASS  
27.if-else.c: PASS    
28.load-store.c: 增加指令 lh, lhu  PASS   
29.max.c: PASS         
30.min3.c: PASS       
31.movsx.c: PASS   
32.sub-longlong.c: PASS          
33.switch.c: PASS  
34.unalign.c: PASS  
35.hello-str.c 待定。   

注意：
- “两个64位有符号数相乘，结果的低32位” 和 “把这两个64位数看作无符号数相乘，结果的低32位” 相同
- 正数按位取反加一等于该正数的相反数的补码









## PA2.3 - 程序，运行时环境与AM
***
>**实验必做题: 阅读Makefile**
>
>abstract-machine项目的Makefile设计得非常巧妙, 你需要把它们看成一种代码来RTFSC, 从而理解它们是如何工作的.
>这样一来, 你就知道怎么编写有一定质量的Makefile了; 同时, 如果哪天Makefile出现了非预期的行为, 你就可以尝试对Makefile进行调试了. 当然, 这少不了RTFM    

解答: 以下为abstract-machine目录下的Makefile文件完整内容。   
```Makefile
# 获取可读版本的Makefile通过 `make html` (需要python-markdown)
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks

# 默认创建裸机内核镜像
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

# 覆盖当 `make clean/clean-all/html` 的检查
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

# 打印构建信息消息
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

# 检查：环境变量 `$AM_HOME` 是否看起来正常
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME 必须是一个AbstractMachine仓库)
endif

# 检查：环境变量 `$ARCH` 必须在支持的列表中
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error 预期 $$ARCH 在 {$(ARCHS)} 中, 得到 "$(ARCH)")
endif

# 提取指令集架构 (`ISA`) 和平台从 `$ARCH`. 示例: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

# 检查是否有东西可以构建
ifeq ($(flavor SRCS), undefined)
  $(error 没有什么可以构建)
endif

# 检查结束
endif

## 2. General Compilation Targets

# 创建目标目录 (`build/$ARCH`)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

# 编译目标 (一个二进制镜像或存档)
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

# 收集要链接的文件: 对象文件 (`.o`) 和库 (`.a`)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # 懒惰求值 ("=") 导致无限递归
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))

## 3. General Compilation Flags

# (交叉) 编译器, e.g., mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

# 编译标志
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INCFLAGS += $(addprefix -I, $(INC_PATH))

ARCH_H := arch/$(ARCH).h
CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
ASFLAGS  += -MMD $(INCFLAGS)
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations

# 插入架构特定配置 (例如来自 `scripts/x86_64-qemu.mk`)
-include $(AM_HOME)/scripts/$(ARCH).mk

## 5. Compilation Rules

# 规则 (编译): 单个 `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

# 规则 (编译): 单个 `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

# 规则 (编译): 单个 `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

# 规则 (编译): 单个 `.S` -> `.o` (gcc, 预处理后调用as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

# 规则 (递归 make): 构建依赖库 (am, klib, ...)
$(LIBS): %:
	@$(MAKE) -s -C $(AM_HOME)/$* archive

# 规则 (链接): 对象 (`*.o`) 和库 (`*.a`) -> `IMAGE.elf`, 最终的 ELF 镜像打包进入 image (ld)
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

# 规则 (归档): 对象 (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^

# 规则 (`#include` 依赖): 插入由 gcc 生成的 `.d` 文件 `-MMD`
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

# 构建顺序控制
image: image-dep
archive: $(ARCHIVE)
image-dep: $(LIBS) $(IMAGE).elf
.NOTPARALLEL: image-dep
.PHONY: image image-dep archive run $(LIBS)

# 清理单个项目 (移除 `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

# 清理所有深度为2的子项目 (忽略错误)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)
```
下面逐个拆解介绍:    


A.  
```Makefile
# 获取可读版本的Makefile通过 `make html` (需要python-markdown)
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html
```
首先定义目标html。在Makefile中，目标（target）通常是文件名或伪目标（phony target）。  
在这个例子中，html是一个伪目标，意味着它不是一个实际的文件，而是执行某个任务的指令集合。  

html目标下面一行条命令行用于生成一个HTML版本的Makefile文档。具体步骤如下：    
1. ``cat Makefile``: 使用cat命令读取当前目录下的Makefile文件内容，并将其输出到标准输出。    
2. ``sed 's/^$[^#]$/    \1/g'``: 使用sed流编辑器对输入进行处理。这个命令的作用是将每一行开头不是#（即不是注释行）的内容前面加上四个空格。这样做的目的是为了格式化非注释行，使其在Markdown中显示为代码块的一部分。  
3. ``markdown_py``: 这个命令假设系统上安装了markdown_py工具，该工具可以将Markdown文本转换为HTML。  
4. ``> Makefile.html``: 将上述命令的输出重定向到一个新的文件Makefile.html中，这样就得到了一个HTML格式的Makefile文档。  

最后一行中，``.PHONY``是一个特殊的Makefile指示符，用于声明某些目标是伪目标。  
伪目标不会检查同名文件是否存在，也不会比较时间戳来决定是否需要重新构建。  
在这里，html被声明为伪目标，这意味着每次运行make html时都会执行上述命令，即使当前目录下已经存在一个名为html的文件。  


B.  
```Makefile
## 1. Basic Setup and Checks

# 默认创建裸机内核镜像
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

# 覆盖当 `make clean/clean-all/html` 的检查
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

# 打印构建信息消息
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

# 检查：环境变量 `$AM_HOME` 是否看起来正常
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME 必须是一个AbstractMachine仓库)
endif

# 检查：环境变量 `$ARCH` 必须在支持的列表中
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error 预期 $$ARCH 在 {$(ARCHS)} 中, 得到 "$(ARCH)")
endif

# 提取指令集架构 (`ISA`) 和平台从 `$ARCH`. 示例: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

# 检查是否有东西可以构建
ifeq ($(flavor SRCS), undefined)
  $(error 没有什么可以构建)
endif

# 检查结束
endif
```
1. 默认创建裸机内核镜像  
```ifeq ($(MAKECMDGOALS),)```: 检查用户是否指定了任何目标。如果没有指定目标，则执行条件内的命令。  
```MAKECMDGOALS  = image```: 将默认目标设置为image。  
```.DEFAULT_GOAL = image```: 明确指定默认目标为image，这样在没有其他目标被指定时，Make会自动执行image目标。  

2. 覆盖当make clean/ clean-all / html的检查  
```$(findstring $(MAKECMDGOALS),clean|clean-all|html)```: 查找MAKECMDGOALS中是否包含clean、clean-all或html中的任何一个字符串。  
```ifeq (...)```: 如果MAKECMDGOALS不包含这些字符串，则执行条件内的命令。这表示只有在目标不是clean、clean-all或html时才进行后续的检查。  

3. 打印构建信息消息  
```$(info ...)```: 打印一条信息消息到终端。  
这里的消息格式是```# Building <项目名称>-<目标> [<架构>]```，例如```# Building myproject-image [x86_64]```。  

4. 检查：环境变量 $AM_HOME 是否看起来正常  
$(wildcard $(AM_HOME)/am/include/am.h): 检查AM_HOME路径下是否存在am/include/am.h文件。
ifeq (...): 如果不存在该文件，则执行错误处理。
$(error $$AM_HOME 必须是一个AbstractMachine仓库): 输出错误信息并终止Makefile的执行。






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
