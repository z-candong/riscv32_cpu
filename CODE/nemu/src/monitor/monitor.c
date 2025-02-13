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

#include <isa.h>          // 包含ISA相关的头文件
#include <memory/paddr.h>  // 包含物理地址相关的头文件

// 初始化随机数生成器
void init_rand();
// 初始化日志系统，传入日志文件名
void init_log(const char *log_file);
// 初始化内存
void init_mem();
// 初始化差异测试，传入参考.so文件路径、镜像大小和端口号
void init_difftest(char *ref_so_file, long img_size, int port);
// 初始化设备
void init_device();
// 初始化简单调试器
void init_sdb();
// 初始化反汇编工具
void init_disasm();

// 欢迎信息函数
static void welcome() {
  // 打印跟踪状态
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  // 如果启用了跟踪，则打印提示信息
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  // 打印构建时间和日期
  Log("Build time: %s, %s", __TIME__, __DATE__);
  // 打印欢迎信息
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  // 提示用户输入帮助命令
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>  // 包含选项解析头文件

// 设置SDB为批处理模式
void sdb_set_batch_mode();

// 日志文件名指针
static char *log_file = NULL;
// 差异测试参考.so文件路径指针
static char *diff_so_file = NULL;
// 镜像文件路径指针
static char *img_file = NULL;
// 差异测试端口号
static int difftest_port = 1234;

// 加载镜像到内存中
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

  // 将文件指针移动到文件开头
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
  // 定义选项表
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  // 循环解析每个选项
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break; // 设置批处理模式
      case 'p': sscanf(optarg, "%d", &difftest_port); break; // 设置差异测试端口
      case 'l': log_file = optarg; break; // 设置日志文件
      case 'd': diff_so_file = optarg; break; // 设置差异测试参考.so文件
      case 1: img_file = optarg; return 0; // 设置镜像文件
      default:
        // 打印使用说明
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

// 初始化监控器
void init_monitor(int argc, char *argv[]) {
  /* 进行一些全局初始化操作 */

  /* 解析命令行参数 */
  parse_args(argc, argv);

  /* 设置随机种子 */
  init_rand();

  /* 打开日志文件 */
  init_log(log_file);

  /* 初始化内存 */
  init_mem();

  /* 初始化设备 */
  IFDEF(CONFIG_DEVICE, init_device());

  /* 进行ISA依赖的初始化 */
  init_isa();

  /* 将镜像加载到内存中。这将覆盖内置镜像 */
  long img_size = load_img();

  /* 初始化差异测试 */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* 初始化简单调试器 */
  init_sdb();

  /* 初始化反汇编工具 */
  IFDEF(CONFIG_ITRACE, init_disasm());

  /* 显示欢迎消息 */
  welcome();
}
#else // CONFIG_TARGET_AM
// 在AM目标平台上加载镜像到内存中
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

// 初始化监控器（AM平台专用）
void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
