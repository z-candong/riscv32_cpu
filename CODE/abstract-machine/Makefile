# Makefile for AbstractMachine 内核和库

### *生成可读版本的此 Makefile* 通过 `make html`（需要 python-markdown）
html:
	# 将 Makefile 转换为 HTML 使用 markdown_py
	cat Makefile | sed 's/^$[^#]$/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. 基本设置和检查

### 默认构建裸机内核镜像
ifeq ($(MAKECMDGOALS),)
  # 如果没有指定目标，默认构建 'image'
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### 在执行 `make clean/clean-all/html` 时跳过检查
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### 打印构建信息消息
$(info # 正在构建 $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### 检查：环境变量 `$AM_HOME` 是否有效
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  # 确保 AM_HOME 指向有效的 AbstractMachine 仓库
  $(error $$AM_HOME 必须是一个 AbstractMachine 仓库)
endif

### 检查：环境变量 `$ARCH` 是否在支持的架构列表中
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  # 确保 ARCH 是受支持的架构之一
  $(error 预期的 $$ARCH 在 {$(ARCHS)} 中，得到 "$(ARCH)"
endif

### 从 `$ARCH` 提取指令集架构 (`ISA`) 和平台。示例：`ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### 检查是否有东西可以构建
ifeq ($(flavor SRCS), undefined)
  # 确保有源文件可供编译
  $(error 没有要构建的内容)
endif

### 检查结束
endif

## 2. 通用编译目标

### 创建目标目录 (`build/$ARCH`)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### 编译目标（二进制镜像或归档）
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### 收集要链接的文件：对象文件 (`.o`) 和库 (`.a`)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # 懒惰求值 ("=") 导致无限递归
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))

## 3. 通用编译标志

### (交叉) 编译器，例如 mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

### 编译标志
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

## 4. 架构特定配置

### 包含架构特定的配置（例如，来自 `scripts/x86_64-qemu.mk`）
-include $(AM_HOME)/scripts/$(ARCH).mk

## 5. 编译规则

### 规则 (编译): 单个 `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	# 创建必要的目录结构
	@mkdir -p $(dir $@) && echo + CC $<
	# 编译 .c 文件成 .o 文件
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### 规则 (编译): 单个 `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	# 创建必要的目录结构
	@mkdir -p $(dir $@) && echo + CXX $<
	# 编译 .cc 文件成 .o 文件
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### 规则 (编译): 单个 `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	# 创建必要的目录结构
	@mkdir -p $(dir $@) && echo + CXX $<
	# 编译 .cpp 文件成 .o 文件
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### 规则 (编译): 单个 `.S` -> `.o` (gcc, 先预处理再调用 as)
$(DST_DIR)/%.o: %.S
	# 创建必要的目录结构
	@mkdir -p $(dir $@) && echo + AS $<
	# 组装 .S 文件成 .o 文件
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

### 规则 (递归 make): 构建依赖库 (am, klib, ...)
$(LIBS): %:
	# 递归构建每个指定的库
	@$(MAKE) -s -C $(AM_HOME)/$* archive

### 规则 (链接): 对象文件 (`*.o`) 和库 (`*.a`) -> `IMAGE.elf`, 最终 ELF 二进制文件（由 ld 链接）
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	# 打印正在创建镜像的消息
	@echo \# 正在创建镜像 [$(ARCH)]
	# 链接对象文件和库成最终的 ELF 二进制文件
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	# 如果架构是 native，则使用 g++ 进行链接
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	# 否则，使用 ld 进行链接
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

### 规则 (归档): 对象文件 (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(OBJS)
	# 打印正在创建归档的消息
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	# 将对象文件归档成静态库
	@$(AR) rcs $@ $^

### 规则 (`#include` 依赖): 引入 gcc 生成的 `.d` 文件（通过 `-MMD`）
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. 其他

### 构建顺序控制
image: image-dep
archive: $(ARCHIVE)
image-dep: $(LIBS) $(IMAGE).elf
.NOTPARALLEL: image-dep
.PHONY: image image-dep archive run $(LIBS)

### 清理单个项目（删除 `build/`）
clean:
	# 删除构建目录和任何生成的 HTML 文件
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### 清理所有深度为2的子项目（并忽略错误）
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	# 递归清理每个子项目
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)
