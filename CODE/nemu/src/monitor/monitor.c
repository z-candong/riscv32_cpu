/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

// 初始化随机数生成器
void init_rand();
// 初始化日志系统，并指定日志文件路径
void init_log(const char *log_file);
// 初始化内存系统
void init_mem();
// 初始化差异测试系统，传入参考库文件名、镜像大小和端口号
void init_difftest(char *ref_so_file, long img_size, int port);
// 初始化设备系统
void init_device();
// 初始化简单调试器（Simple Debugger）
void init_sdb();
// 初始化反汇编功能
void init_disasm();

// 欢迎信息函数，打印构建时间和配置信息
static void welcome() {
  // 打印跟踪开关状态
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  // 如果开启了跟踪，则提示可能产生大日志文件
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  // 打印编译时间
  Log("Build time: %s, %s", __TIME__, __DATE__);
  // 打印欢迎信息，包含模拟器名称
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  // 提示用户输入“help”获取帮助
  printf("For help, type \"help\"\n");
}

// 如果不是目标架构AM，则包含命令行参数解析头文件
#ifndef CONFIG_TARGET_AM
#include <getopt.h>

// 设置SDB为批处理模式
void sdb_set_batch_mode();

// 日志文件路径，默认为空
static char *log_file = NULL;
// 差异测试参考库文件路径，默认为空
static char *diff_so_file = NULL;
// 镜像文件路径，默认为空
static char *img_file = NULL;
// ELF文件路径，默认为空
static char *elf_file = NULL; 
// 差异测试端口，默认1234
static int difftest_port = 1234;

// 解析ELF文件
void parse_elf(const char *elf_file); 

// 加载镜像文件到内存中，返回镜像大小
static long load_img() {
  // 如果没有提供镜像文件，则使用默认内置镜像
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // 内置镜像大小
  }

  // 打开镜像文件
  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  // 获取文件大小
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  // 打印加载的镜像文件及其大小
  Log("The image is %s, size = %ld", img_file, size);

  // 将文件指针移回文件开头
  fseek(fp, 0, SEEK_SET);
  // 读取文件内容到内存中
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  // 关闭文件
  fclose(fp);
  return size;
}

// 解析命令行参数
static int parse_args(int argc, char *argv[]) {
  // 定义命令行选项表
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
	  {"elf"			, required_argument, NULL, 'e'},	
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:e:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'e': elf_file = optarg; break;  
			case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
			  printf("\t-e,--elf=FILE           parse the elf file\n");  
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

// 初始化监控器，传入命令行参数
void init_monitor(int argc, char *argv[]) {
  /* 进行一些全局初始化。 */

  /* 解析命令行参数。 */
  parse_args(argc, argv);

  /* 设置随机种子。 */
  init_rand();

  /* 打开日志文件。 */
  init_log(log_file);

	/* 初始化ELF文件 */
	parse_elf(elf_file);	

  /* 初始化内存。 */
  init_mem();

  /* 初始化设备。 */
  IFDEF(CONFIG_DEVICE, init_device());

  /* 进行ISA相关的初始化。 */
  init_isa();

  /* 将镜像加载到内存中。这将覆盖内置镜像。 */
  long img_size = load_img();

  /* 初始化差异测试。 */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* 初始化简单调试器。 */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* 显示欢迎消息。 */
  welcome();
}
#else // CONFIG_TARGET_AM
// 加载镜像文件到内存中，适用于目标架构AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

// AM架构下的初始化监控器函数
void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif




