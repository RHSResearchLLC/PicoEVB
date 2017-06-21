//-----------------------------------------------------------------------------
//
// (c) Copyright 2012-2012 Xilinx, Inc. All rights reserved.
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
//-----------------------------------------------------------------------------
//
// Project    : The Xilinx PCI Express DMA 
// File       : xdma_app.v
// Version    : $IpVersion 
//-----------------------------------------------------------------------------

`timescale 1ps / 1ps
module xdma_app #(
  parameter TCQ = 1,
  parameter C_M_AXI_ID_WIDTH = 4,
  parameter C_DATA_WIDTH = 64,
  parameter C_M_AXI_DATA_WIDTH = C_DATA_WIDTH
)
(

  // AXI Memory Mapped interface
  input  wire  [C_M_AXI_ID_WIDTH-1:0] s_axi_awid,
  input  wire  [64-1:0] s_axi_awaddr,
  input  wire   [7:0] s_axi_awlen,
  input  wire   [2:0] s_axi_awsize,
  input  wire   [1:0] s_axi_awburst,
  input  wire         s_axi_awvalid,
  output wire         s_axi_awready,
  input  wire [C_M_AXI_DATA_WIDTH-1:0]        s_axi_wdata,
  input  wire [(C_M_AXI_DATA_WIDTH/8)-1:0]    s_axi_wstrb,
  input  wire         s_axi_wlast,
  input  wire         s_axi_wvalid,
  output wire         s_axi_wready,
  output wire [C_M_AXI_ID_WIDTH-1:0]          s_axi_bid,
  output wire   [1:0] s_axi_bresp,
  output wire         s_axi_bvalid,
  input  wire         s_axi_bready,
  input  wire [C_M_AXI_ID_WIDTH-1:0]          s_axi_arid,
  input  wire  [64-1:0] s_axi_araddr,
  input  wire   [7:0] s_axi_arlen,
  input  wire   [2:0] s_axi_arsize,
  input  wire   [1:0] s_axi_arburst,
  input  wire         s_axi_arvalid,
  output wire         s_axi_arready,
  output wire   [C_M_AXI_ID_WIDTH-1:0]        s_axi_rid,
  output wire   [C_M_AXI_DATA_WIDTH-1:0]      s_axi_rdata,
  output wire   [1:0] s_axi_rresp,
  output wire         s_axi_rlast,
  output wire         s_axi_rvalid,
  input  wire         s_axi_rready,

  // System IO signals
  input  wire         user_clk,
  input  wire         user_resetn,
  input  wire         user_lnk_up,
  input  wire         sys_rst_n,
  output wire   [3:0] leds

);
  // wire/reg declarations
  wire            sys_reset;
  reg  [25:0]     user_clk_heartbeat;

  // The sys_rst_n input is active low based on the core configuration
  assign sys_resetn = sys_rst_n;

  // Create a Clock Heartbeat
  always @(posedge user_clk) begin
    if(!sys_resetn) begin
      user_clk_heartbeat <= #TCQ 26'd0;
    end else begin
      user_clk_heartbeat <= #TCQ user_clk_heartbeat + 1'b1;
    end
  end

  // LEDs for observation
  assign leds[0] = sys_resetn;
  assign leds[1] = user_resetn;
  assign leds[2] = user_lnk_up;
  assign leds[3] = user_clk_heartbeat[25];

  // Block ram for the AXI interface
  blk_mem_gen_1 blk_mem_xdma_inst(
    .s_aclk          (user_clk),
    .s_aresetn       (user_resetn),
    .s_axi_awid      (s_axi_awid),
    .s_axi_awaddr    (s_axi_awaddr[31:0]),
    .s_axi_awlen     (s_axi_awlen),
    .s_axi_awsize    (s_axi_awsize),
    .s_axi_awburst   (s_axi_awburst),
    .s_axi_awvalid   (s_axi_awvalid),
    .s_axi_awready   (s_axi_awready),
    .s_axi_wdata     (s_axi_wdata),
    .s_axi_wstrb     (s_axi_wstrb),
    .s_axi_wlast     (s_axi_wlast),
    .s_axi_wvalid    (s_axi_wvalid),
    .s_axi_wready    (s_axi_wready),
    .s_axi_bid       (s_axi_bid),
    .s_axi_bresp     (s_axi_bresp),
    .s_axi_bvalid    (s_axi_bvalid),
    .s_axi_bready    (s_axi_bready),
    .s_axi_arid      (s_axi_arid),
    .s_axi_araddr    (s_axi_araddr[31:0]),
    .s_axi_arlen     (s_axi_arlen),
    .s_axi_arsize    (s_axi_arsize),
    .s_axi_arburst   (s_axi_arburst),
    .s_axi_arvalid   (s_axi_arvalid),
    .s_axi_arready   (s_axi_arready),
    .s_axi_rid       (s_axi_rid),
    .s_axi_rdata     (s_axi_rdata),
    .s_axi_rresp     (s_axi_rresp),
    .s_axi_rlast     (s_axi_rlast),
    .s_axi_rvalid    (s_axi_rvalid),
    .s_axi_rready    (s_axi_rready)
  );


endmodule
