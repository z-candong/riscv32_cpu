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

#include <memory/host.h>  // 包含主机内存相关的定义和函数
#include <memory/paddr.h>  // 包含物理地址相关的定义和函数
#include <device/mmio.h>    // 包含内存映射I/O设备相关的定义和函数
#include <isa.h>            // 包含ISA（Instruction Set Architecture）相关的定义和函数

// 根据配置选择不同的物理内存分配方式
#if defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;  // 动态分配物理内存的指针
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};  // 静态分配物理内存的数组，并进行页对齐
#endif

// 将物理地址转换为主机地址
uint8_t* guest_to_host(paddr_t paddr) {
  return pmem + paddr - CONFIG_MBASE;  // 计算并返回对应的主机地址
}

// 将主机地址转换为物理地址
paddr_t host_to_guest(uint8_t *haddr) {
  return haddr - pmem + CONFIG_MBASE;  // 计算并返回对应的物理地址
}

// 从物理内存中读取数据
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);  // 调用host_read函数从主机地址读取数据
  return ret;  // 返回读取的数据
}

// 向物理内存中写入数据
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);  // 调用host_write函数向主机地址写入数据
}

// 处理物理地址超出边界的错误
static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);  // 使用panic宏报告错误信息
}

// 初始化物理内存
void init_mem() {
#if defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);  // 动态分配物理内存
  assert(pmem);  // 确保内存分配成功
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));  // 如果定义了CONFIG_MEM_RANDOM，则将内存随机初始化
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);  // 记录物理内存区域的信息
}

// 从物理地址读取数据
word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);  // 如果地址在物理内存范围内，则调用pmem_read读取数据
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));  // 如果定义了CONFIG_DEVICE，则调用mmio_read从内存映射I/O设备读取数据
  out_of_bound(addr);  // 如果地址超出范围，则调用out_of_bound处理错误
  return 0;  // 返回默认值0
}

// 向物理地址写入数据
void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }  // 如果地址在物理内存范围内，则调用pmem_write写入数据
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return; )  // 如果定义了CONFIG_DEVICE，则调用mmio_write向内存映射I/O设备写入数据
  out_of_bound(addr);  // 如果地址超出范围，则调用out_of_bound处理错误
}

