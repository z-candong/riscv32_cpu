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

SRCS-y += src/nemu-main.c  # 指定参与编译的源文件：src/nemu-main.c 
DIRS-y += src/cpu src/monitor src/utils  # 指定参与编译的目录：src/cpu; src/monitor; src/utils
DIRS-$(CONFIG_MODE_SYSTEM) += src/memory # 条件指定参与编译的目录：src/memory（仅当CONFIG_MODE_SYSTEM为真时参与编译）
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb # 条件指定不参与编译的目录：src/monitor/sdb（仅当CONFIG_TARGET_AM为真时不参与编译）

SHARE = $(if $(CONFIG_TARGET_SHARE),1,0)  # 定义SHARE变量：当CONFIG_TARGET_SHARE被定义且为真，则SHARE为1，否则为0
LIBS += $(if $(CONFIG_TARGET_NATIVE_ELF),-lreadline -ldl -pie,)。# 向链接库列表中添加库: 当CONFIG_TARGET_NATIVE_ELF被定义且为真，则将-lreadline -ldl -pie添加到LIBS中

ifdef mainargs  # 如果定义变量mainargs
ASFLAGS += -DBIN_PATH=\"$(mainargs)\"  # 则将-DBIN_PATH="$(mainargs)"添加到汇编器标志ASFLAGS中。添加后在编译过程中，宏BIN_PATH会被设置为变量mainargs的值
endif
SRCS-$(CONFIG_TARGET_AM) += src/am-bin.S  # 当变量CONFIG_TARGET_AM被定义并且其值为真时，才会将src/am-bin.S加入到SRCS-y中，从而指定编译文件am-bin.S
.PHONY: src/am-bin.S  # 声明src/am-bin.S是一个伪目标（phony target），伪目标不会检查同名文件是否存在或是否需要重新构建，而是每次都会执行相应的规则。
                      # 此处指即使存在src/am-bin.S文件，也会强制重新处理该文件。
