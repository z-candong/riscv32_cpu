# 用RTL实现最简单的处理器  
通过实现NEMU, 已经从概念上了解处理器的构成, 以及处理器大致如何工作了, 现在尝试通过RTL代码实现一个最简单的处理器。   
独立设计的处理器命名为NPC (New Processor Core)。  
## 处理器基本架构  
通过RTL代码实现PA1中的TRM（TuRing Machine）部件：  
1. PC - 本质上是一个不断加"1"的计数器，这里的"1"是指一条指令的长度  
2. 寄存器 - 这里指通用寄存器(GPR, General Purpose Register)，它通常是由一组寄存器组成的  
           (注意：从0号寄存器读出的值总是为0)  
3. 加法器 - 用于执行二进制数加法运算的器件  
4. 存储器 - 用于保存数据和指令的器件，相当于一个可以读写的“大数组”  

在数字电路实验中，涵盖以上部件的RTL代码实现方式。

按照处理器工作流程划分RTL项目模块：取指、译码、执行、更新PC：  
1. IFU(Instruction Fetch Unit): 负责根据当前PC从存储器中取出一条指令  
2. IDU(Instruction Decode Unit): 负责对当前指令进行译码, 准备执行阶段需要使用的数据和控制信号  
3. EXU(EXecution Unit): 负责根据控制信号对数据进行执行操作, 并将执行结果写回寄存器或存储器  
4. 更新PC: 通过RTL实现时, 这一操作一般与PC寄存器一同实现, 因而无需为此划分一个独立的模块  

上述部件可自行决定放置于哪一个模块中，同时梳理模块之间的接口。  
存储器是例外。为方便测试，不通过RTL代码实现存储器，而是使用C++实现。  
因此，需要将存储器访问接口的信号拉到顶层，通过C++代码访问存储器：  
```C++
while (???) {
  ...
  top->inst = pmem_read(top->pc);
  top->eval();
  ...
}
```
通过C++代码实现一个简单的存储器：
```C++

```
## 若干代码风格和规范  
- 如果使用Verilog HDL，则建议实例化+连线方式方式描述电路。  
  建议使用数据流建模和结构化建模方式，不推荐使用行为建模方式。  
- 如果使用Chisel，则建议不使用when和switch。  
  因为when和switch语义和Verilog HDL行为建模非常相似。  

简单来讲，在RTL代码编写过程中遵守“不编写任何always语句”的原则。  
下面提供触发器和选择器的Verilog模板以供调用：  
```verilog
// 触发器模板
module Reg #(WIDTH = 1, RESET_VAL = 0) (
  input clk,
  input rst,
  input [WIDTH-1:0] din,
  output reg [WIDTH-1:0] dout,
  input wen
);
  always @(posedge clk) begin
    if (rst) dout <= RESET_VAL;
    else if (wen) dout <= din;
  end
endmodule

// 使用触发器模板的示例
module example(
  input clk,
  input rst,
  input [3:0] in,
  output [3:0] out
);
  // 位宽为1比特, 复位值为1'b1, 写使能一直有效
  Reg #(1, 1'b1) i0 (clk, rst, in[0], out[0], 1'b1);
  // 位宽为3比特, 复位值为3'b0, 写使能为out[0]
  Reg #(3, 3'b0) i1 (clk, rst, in[3:1], out[3:1], out[0]);
endmodule
```

```verilog
// 选择器模板内部实现
module MuxKeyInternal #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1, HAS_DEFAULT = 0) (
  output reg [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [DATA_LEN-1:0] default_out,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);

  localparam PAIR_LEN = KEY_LEN + DATA_LEN;
  wire [PAIR_LEN-1:0] pair_list [NR_KEY-1:0];
  wire [KEY_LEN-1:0] key_list [NR_KEY-1:0];
  wire [DATA_LEN-1:0] data_list [NR_KEY-1:0];

  genvar n;
  generate
    for (n = 0; n < NR_KEY; n = n + 1) begin
      assign pair_list[n] = lut[PAIR_LEN*(n+1)-1 : PAIR_LEN*n];
      assign data_list[n] = pair_list[n][DATA_LEN-1:0];
      assign key_list[n]  = pair_list[n][PAIR_LEN-1:DATA_LEN];
    end
  endgenerate

  reg [DATA_LEN-1 : 0] lut_out;
  reg hit;
  integer i;
  always @(*) begin
    lut_out = 0;
    hit = 0;
    for (i = 0; i < NR_KEY; i = i + 1) begin
      lut_out = lut_out | ({DATA_LEN{key == key_list[i]}} & data_list[i]);
      hit = hit | (key == key_list[i]);
    end
    if (!HAS_DEFAULT) out = lut_out;
    else out = (hit ? lut_out : default_out);
  end
endmodule

// 不带默认值的选择器模板
module MuxKey #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
  output [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);
  MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 0) i0 (out, key, {DATA_LEN{1'b0}}, lut);
endmodule

// 带默认值的选择器模板
module MuxKeyWithDefault #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
  output [DATA_LEN-1:0] out,
  input [KEY_LEN-1:0] key,
  input [DATA_LEN-1:0] default_out,
  input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
);
  MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 1) i0 (out, key, default_out, lut);
endmodule
```


```verilog
// 二选一多路选择器
module mux21(a,b,s,y);
  input   a,b,s;
  output  y;

  // 通过MuxKey实现如下always代码
  // always @(*) begin
  //  case (s)
  //    1'b0: y = a;
  //    1'b1: y = b;
  //  endcase
  // end
  MuxKey #(2, 1, 1) i0 (y, s, {
    1'b0, a,
    1'b1, b
  });
endmodule

//四选一多路选择器
module mux41(a,s,y);
  input  [3:0] a;
  input  [1:0] s;
  output y;

  // 通过MuxKeyWithDefault实现如下always代码
  // always @(*) begin
  //  case (s)
  //    2'b00: y = a[0];
  //    2'b01: y = a[1];
  //    2'b10: y = a[2];
  //    2'b11: y = a[3];
  //    default: y = 1'b0;
  //  endcase
  // end
  MuxKeyWithDefault #(4, 2, 1) i0 (y, s, 1'b0, {
    2'b00, a[0],
    2'b01, a[1],
    2'b10, a[2],
    2'b11, a[3]
  });
endmodule
```

## 在NPC中实现第一条指令  
任务：实现一条最简单的指令：addi。  
在NEMU中，已经剖析这条指令是如何执行的了，现在使用RTL来实现它。  
***Step1.画出架构图***
画出仅支持addi指令的单周期处理器的架构图。



***Step2.在NPC中实现addi指令***
在实现过程中需要注意以下事项：  
1. PC的复位值设置为0x80000000  
2. 存储器中可以放置若干条addi指令的二进制编码(可以利用0号寄存器的特性来编写行为确定的指令)  
3. 由于目前未实现跳转指令, 因此NPC只能顺序执行, 你可以在NPC执行若干指令之后停止仿真  
4. 可以通过查看波形, 或者在RTL代码中打印通用寄存器的状态, 来检查addi指令是否被正确执行  
5. 关于通用寄存器, 你需要思考如何实现0号寄存器的特性; 此外, 为了避免选择Verilog的同学编写出不太合理的行为建模代码, 我们给出如下不完整的代码供大家补充(大家无需改动always代码块中的内容):  
```verilog
module RegisterFile #(ADDR_WIDTH = 1, DATA_WIDTH = 1) (
  input clk,
  input [DATA_WIDTH-1:0] wdata,
  input [ADDR_WIDTH-1:0] waddr,
  input wen
);
  reg [DATA_WIDTH-1:0] rf [2**ADDR_WIDTH-1:0];
  always @(posedge clk) begin
    if (wen) rf[waddr] <= wdata;
  end
endmodule
```
6. 使用NVBoard需要RTL代码比较好地支持设备, 这将在后续进行介绍, 目前不必接入NVBoard  

## 让程序决定仿真何时结束  
上述内容是让仿真环境（C++代码）来决定执行多少条指令后结束仿真，该做法不具备通用性：需要提前知道一个程序将会执行多少条指令。
现在尝试实现：在程序执行结束的时候自动结束仿真。  
在PA中NEMU给出一个很好的解决方案：trap指令。  
NEMU实现了一条特殊的nemutrap指令, 用于指示客户程序的结束。  
具体地, 在RISC-V中, NEMU选择了ebreak指令来作为nemutrap指令。  
而在NPC中也可以实现类似的功能: 如果程序执行了ebreak指令, 就通知仿真环境结束仿真。

要实现这一功能, 首先需要在NPC中添加ebreak指令的支持。  
不过, 为了让NPC在执行ebreak指令的时候可以通知仿真环境, 你还需要实现一种RTL代码和C++代码之间的交互机制。  
我们借用system verilog中的DPI-C机制来实现这一交互。

***
|***Step1.尝试DPI-C机制***|
|-----------------------|
|*阅读verilator手册, 找到DPI-C机制的相关内容, 并尝试运行手册中的例子。* |


***
|***Step2.通过DPI-C实现ebreak***|  
|-----------------------------|
|*在RTL代码中利用DPI-C机制, 使得在NPC执行ebreak指令的时候通知仿真环境结束仿真。*|
|*实现后, 在存储器中放置一条ebreak指令来进行测试。如果你的实现正确, 仿真环境就无需关心程序何时结束仿真了, 它只需要不停地进行仿真, 直到程序执行ebreak指令为止。*|
|*如果你使用Chisel, 你可以借助Chisel中的BlackBox机制调用Verilog代码, 然后让Verilog代码通过DPI-C机制与仿真环境交互。关于BlackBox的使用方式, 请查阅相关资料。*|


***








