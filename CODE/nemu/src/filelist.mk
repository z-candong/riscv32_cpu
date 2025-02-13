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

ifdef mainargs
ASFLAGS += -DBIN_PATH=\"$(mainargs)\"
endif
SRCS-$(CONFIG_TARGET_AM) += src/am-bin.S
.PHONY: src/am-bin.S
