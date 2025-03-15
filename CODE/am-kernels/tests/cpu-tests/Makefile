# 定义伪目标，避免这些目标被视为文件名
.PHONY: all run gdb clean latest $(ALL)

# 存储测试结果的文件名
RESULT = .result

# 创建或清空结果文件
$(shell > $(RESULT))

# 定义颜色代码用于终端输出
COLOR_RED   = \033[1;31m
COLOR_GREEN = \033[1;32m
COLOR_NONE  = \033[0m

# 查找 tests/ 目录下的所有 .c 文件，并提取其基本名称（去掉路径和扩展名）
ALL = $(basename $(notdir $(shell find tests/. -name "*.c")))

# 主目标，依赖于每个测试用例对应的 Makefile 文件，并列出所有测试项
all: $(addprefix Makefile., $(ALL))
	@echo "test list [$(words $(ALL)) item(s)]:" $(ALL)

# 模式规则，表示如何处理每一个测试用例
$(ALL): %: Makefile.%

# 为每个测试用例生成一个临时的 Makefile 文件，然后调用这个临时文件进行编译和运行测试
Makefile.%: tests/%.c latest
	# 生成临时 Makefile 文件，包含 NAME 和 SRCS 变量，并包含主 Makefile
	@/bin/echo -e "NAME = $*\nSRCS = $<\ninclude $${AM_HOME}/Makefile" > $@
	# 调用生成的临时 Makefile 进行编译和运行测试
	@if make -s -f $@ ARCH=$(ARCH) $(MAKECMDGOALS); then \
		# 如果测试通过，记录 PASS 到结果文件
		printf "[%14s] $(COLOR_GREEN)PASS$(COLOR_NONE)\n" $* >> $(RESULT); \
	else \
		# 如果测试失败，记录 ***FAIL*** 到结果文件
		printf "[%14s] $(COLOR_RED)***FAIL***$(COLOR_NONE)\n" $* >> $(RESULT); \
	fi
	# 删除临时 Makefile 文件
	-@rm -f Makefile.$*

# 显示测试结果并清理结果文件
run: all
	@cat $(RESULT)
	@rm $(RESULT)

# 调试目标，目前未定义具体操作
gdb: all

# 清理临时文件和构建目录
clean:
	rm -rf Makefile.* build/

# 当前规则没有实际操作，只是触发点
latest:
