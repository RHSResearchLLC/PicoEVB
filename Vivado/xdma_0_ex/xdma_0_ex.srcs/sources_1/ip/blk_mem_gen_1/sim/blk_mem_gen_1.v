// (c) Copyright 1995-2017 Xilinx, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: xilinx.com:ip:blk_mem_gen:8.3
// IP Revision: 5

`timescale 1ns/1ps

(* DowngradeIPIdentifiedWarnings = "yes" *)
module blk_mem_gen_1 (
  s_aclk,
  s_aresetn,
  s_axi_awid,
  s_axi_awaddr,
  s_axi_awlen,
  s_axi_awsize,
  s_axi_awburst,
  s_axi_awvalid,
  s_axi_awready,
  s_axi_wdata,
  s_axi_wstrb,
  s_axi_wlast,
  s_axi_wvalid,
  s_axi_wready,
  s_axi_bid,
  s_axi_bresp,
  s_axi_bvalid,
  s_axi_bready,
  s_axi_arid,
  s_axi_araddr,
  s_axi_arlen,
  s_axi_arsize,
  s_axi_arburst,
  s_axi_arvalid,
  s_axi_arready,
  s_axi_rid,
  s_axi_rdata,
  s_axi_rresp,
  s_axi_rlast,
  s_axi_rvalid,
  s_axi_rready
);

(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 CLK.ACLK CLK" *)
input wire s_aclk;
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 RST.ARESETN RST" *)
input wire s_aresetn;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWID" *)
input wire [3 : 0] s_axi_awid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWADDR" *)
input wire [31 : 0] s_axi_awaddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWLEN" *)
input wire [7 : 0] s_axi_awlen;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWSIZE" *)
input wire [2 : 0] s_axi_awsize;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWBURST" *)
input wire [1 : 0] s_axi_awburst;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWVALID" *)
input wire s_axi_awvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI AWREADY" *)
output wire s_axi_awready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI WDATA" *)
input wire [63 : 0] s_axi_wdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI WSTRB" *)
input wire [7 : 0] s_axi_wstrb;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI WLAST" *)
input wire s_axi_wlast;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI WVALID" *)
input wire s_axi_wvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI WREADY" *)
output wire s_axi_wready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI BID" *)
output wire [3 : 0] s_axi_bid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI BRESP" *)
output wire [1 : 0] s_axi_bresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI BVALID" *)
output wire s_axi_bvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI BREADY" *)
input wire s_axi_bready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARID" *)
input wire [3 : 0] s_axi_arid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARADDR" *)
input wire [31 : 0] s_axi_araddr;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARLEN" *)
input wire [7 : 0] s_axi_arlen;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARSIZE" *)
input wire [2 : 0] s_axi_arsize;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARBURST" *)
input wire [1 : 0] s_axi_arburst;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARVALID" *)
input wire s_axi_arvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI ARREADY" *)
output wire s_axi_arready;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RID" *)
output wire [3 : 0] s_axi_rid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RDATA" *)
output wire [63 : 0] s_axi_rdata;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RRESP" *)
output wire [1 : 0] s_axi_rresp;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RLAST" *)
output wire s_axi_rlast;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RVALID" *)
output wire s_axi_rvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:aximm:1.0 AXI_SLAVE_S_AXI RREADY" *)
input wire s_axi_rready;

  blk_mem_gen_v8_3_5 #(
    .C_FAMILY("artix7"),
    .C_XDEVICEFAMILY("artix7"),
    .C_ELABORATION_DIR("./"),
    .C_INTERFACE_TYPE(1),
    .C_AXI_TYPE(1),
    .C_AXI_SLAVE_TYPE(0),
    .C_USE_BRAM_BLOCK(0),
    .C_ENABLE_32BIT_ADDRESS(0),
    .C_CTRL_ECC_ALGO("NONE"),
    .C_HAS_AXI_ID(1),
    .C_AXI_ID_WIDTH(4),
    .C_MEM_TYPE(1),
    .C_BYTE_SIZE(8),
    .C_ALGORITHM(1),
    .C_PRIM_TYPE(1),
    .C_LOAD_INIT_FILE(0),
    .C_INIT_FILE_NAME("no_coe_file_loaded"),
    .C_INIT_FILE("blk_mem_gen_1.mem"),
    .C_USE_DEFAULT_DATA(0),
    .C_DEFAULT_DATA("0"),
    .C_HAS_RSTA(0),
    .C_RST_PRIORITY_A("CE"),
    .C_RSTRAM_A(0),
    .C_INITA_VAL("0"),
    .C_HAS_ENA(1),
    .C_HAS_REGCEA(0),
    .C_USE_BYTE_WEA(1),
    .C_WEA_WIDTH(8),
    .C_WRITE_MODE_A("READ_FIRST"),
    .C_WRITE_WIDTH_A(64),
    .C_READ_WIDTH_A(64),
    .C_WRITE_DEPTH_A(512),
    .C_READ_DEPTH_A(512),
    .C_ADDRA_WIDTH(9),
    .C_HAS_RSTB(1),
    .C_RST_PRIORITY_B("CE"),
    .C_RSTRAM_B(0),
    .C_INITB_VAL("0"),
    .C_HAS_ENB(1),
    .C_HAS_REGCEB(0),
    .C_USE_BYTE_WEB(1),
    .C_WEB_WIDTH(8),
    .C_WRITE_MODE_B("READ_FIRST"),
    .C_WRITE_WIDTH_B(64),
    .C_READ_WIDTH_B(64),
    .C_WRITE_DEPTH_B(512),
    .C_READ_DEPTH_B(512),
    .C_ADDRB_WIDTH(9),
    .C_HAS_MEM_OUTPUT_REGS_A(0),
    .C_HAS_MEM_OUTPUT_REGS_B(0),
    .C_HAS_MUX_OUTPUT_REGS_A(0),
    .C_HAS_MUX_OUTPUT_REGS_B(0),
    .C_MUX_PIPELINE_STAGES(0),
    .C_HAS_SOFTECC_INPUT_REGS_A(0),
    .C_HAS_SOFTECC_OUTPUT_REGS_B(0),
    .C_USE_SOFTECC(0),
    .C_USE_ECC(0),
    .C_EN_ECC_PIPE(0),
    .C_HAS_INJECTERR(0),
    .C_SIM_COLLISION_CHECK("ALL"),
    .C_COMMON_CLK(1),
    .C_DISABLE_WARN_BHV_COLL(0),
    .C_EN_SLEEP_PIN(0),
    .C_USE_URAM(0),
    .C_EN_RDADDRA_CHG(0),
    .C_EN_RDADDRB_CHG(0),
    .C_EN_DEEPSLEEP_PIN(0),
    .C_EN_SHUTDOWN_PIN(0),
    .C_EN_SAFETY_CKT(0),
    .C_DISABLE_WARN_BHV_RANGE(0),
    .C_COUNT_36K_BRAM("1"),
    .C_COUNT_18K_BRAM("0"),
    .C_EST_POWER_SUMMARY("Estimated Power for IP     :     7.356425 mW")
  ) inst (
    .clka(1'D0),
    .rsta(1'D0),
    .ena(1'D0),
    .regcea(1'D0),
    .wea(8'B0),
    .addra(9'B0),
    .dina(64'B0),
    .douta(),
    .clkb(1'D0),
    .rstb(1'D0),
    .enb(1'D0),
    .regceb(1'D0),
    .web(8'B0),
    .addrb(9'B0),
    .dinb(64'B0),
    .doutb(),
    .injectsbiterr(1'D0),
    .injectdbiterr(1'D0),
    .eccpipece(1'D0),
    .sbiterr(),
    .dbiterr(),
    .rdaddrecc(),
    .sleep(1'D0),
    .deepsleep(1'D0),
    .shutdown(1'D0),
    .rsta_busy(),
    .rstb_busy(),
    .s_aclk(s_aclk),
    .s_aresetn(s_aresetn),
    .s_axi_awid(s_axi_awid),
    .s_axi_awaddr(s_axi_awaddr),
    .s_axi_awlen(s_axi_awlen),
    .s_axi_awsize(s_axi_awsize),
    .s_axi_awburst(s_axi_awburst),
    .s_axi_awvalid(s_axi_awvalid),
    .s_axi_awready(s_axi_awready),
    .s_axi_wdata(s_axi_wdata),
    .s_axi_wstrb(s_axi_wstrb),
    .s_axi_wlast(s_axi_wlast),
    .s_axi_wvalid(s_axi_wvalid),
    .s_axi_wready(s_axi_wready),
    .s_axi_bid(s_axi_bid),
    .s_axi_bresp(s_axi_bresp),
    .s_axi_bvalid(s_axi_bvalid),
    .s_axi_bready(s_axi_bready),
    .s_axi_arid(s_axi_arid),
    .s_axi_araddr(s_axi_araddr),
    .s_axi_arlen(s_axi_arlen),
    .s_axi_arsize(s_axi_arsize),
    .s_axi_arburst(s_axi_arburst),
    .s_axi_arvalid(s_axi_arvalid),
    .s_axi_arready(s_axi_arready),
    .s_axi_rid(s_axi_rid),
    .s_axi_rdata(s_axi_rdata),
    .s_axi_rresp(s_axi_rresp),
    .s_axi_rlast(s_axi_rlast),
    .s_axi_rvalid(s_axi_rvalid),
    .s_axi_rready(s_axi_rready),
    .s_axi_injectsbiterr(1'D0),
    .s_axi_injectdbiterr(1'D0),
    .s_axi_sbiterr(),
    .s_axi_dbiterr(),
    .s_axi_rdaddrecc()
  );
endmodule
