#***************************************************************************************
# Copyright (c) 2014-2024 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

# 检查NEMU_HOME是否指向一个有效的NEMU仓库
ifeq ($(wildcard $(NEMU_HOME)/src/nemu-main.c),)
  $(error NEMU_HOME=$(NEMU_HOME) is not a NEMU repo)
endif

# 包含menuconfig生成的变量和规则
-include $(NEMU_HOME)/include/config/auto.conf
-include $(NEMU_HOME)/include/config/auto.conf.cmd

# 定义去除引号的函数
remove_quote = $(patsubst "%",%,$(1))

# 从menuconfig中提取变量
GUEST_ISA ?= $(call remove_quote,$(CONFIG_ISA))       # 客户端指令集架构
ENGINE ?= $(call remove_quote,$(CONFIG_ENGINE))         # 引擎类型
NAME    = $(GUEST_ISA)-nemu-$(ENGINE)                 # 目标程序名称

# 合并所有文件列表
FILELIST_MK = $(shell find -L ./src -name "filelist.mk")   # 查找所有的filelist.mk文件
include $(FILELIST_MK)                                    # 包含这些文件

# 过滤掉黑名单中的目录和文件以获得最终的源文件集合
DIRS-BLACKLIST-y += $(DIRS-BLACKLIST)                     # 添加到黑名单目录列表
SRCS-BLACKLIST-y += $(SRCS-BLACKLIST) $(shell find -L $(DIRS-BLACKLIST-y) -name "*.c")  # 添加到黑名单文件列表
SRCS-y += $(shell find -L $(DIRS-y) -name "*.c")           # 查找所有源文件
SRCS = $(filter-out $(SRCS-BLACKLIST-y),$(SRCS-y))        # 过滤出不在黑名单中的源文件

# 从menuconfig中提取编译器和选项
ifneq ($(CONFIG_CC),)
CC = $(call remove_quote,$(CONFIG_CC))                   # 设置编译器
endif
CFLAGS_BUILD += $(call remove_quote,$(CONFIG_CC_OPT))      # 添加构建选项
CFLAGS_BUILD += $(if $(CONFIG_CC_LTO),-flto,)             # 如果启用了链接时优化，则添加-flto选项
CFLAGS_BUILD += $(if $(CONFIG_CC_DEBUG),-Og -ggdb3,)      # 如果启用了调试模式，则添加-Og -ggdb3选项
CFLAGS_BUILD += $(if $(CONFIG_CC_ASAN),-fsanitize=address,) # 如果启用了地址 sanitizer，则添加-fsanitize=address选项
CFLAGS_TRACE += -DITRACE_COND=$(if $(CONFIG_ITRACE_COND),$(call remove_quote,$(CONFIG_ITRACE_COND)),true) # 添加跟踪条件宏定义
CFLAGS  += $(CFLAGS_BUILD) $(CFLAGS_TRACE) -D__GUEST_ISA__=$(GUEST_ISA) # 组合所有C编译选项
LDFLAGS += $(CFLAGS_BUILD)                                # 将构建选项添加到链接选项

# 包含menuconfig的规则
include $(NEMU_HOME)/scripts/config.mk                    # 包含配置脚本

ifdef CONFIG_TARGET_AM                                      # 如果目标是AM（模拟机）
include $(AM_HOME)/Makefile                                 # 包含AM的Makefile
LINKAGE += $(ARCHIVES)                                      # 添加归档文件到链接项
else                                                        # 否则
# 包含用于构建NEMU的规则
include $(NEMU_HOME)/scripts/native.mk                      # 包含本地构建脚本
endif



