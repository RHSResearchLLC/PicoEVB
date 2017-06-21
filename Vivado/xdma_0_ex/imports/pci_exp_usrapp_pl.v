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
// File       : pci_exp_usrapp_pl.v
// Version    : $IpVersion 
//-----------------------------------------------------------------------------
module pci_exp_usrapp_pl (

                          pl_initial_link_width,
                          pl_lane_reversal_mode,
                          pl_link_gen2_capable,
                          pl_link_partner_gen2_supported,
                          pl_link_upcfg_capable,
                          pl_ltssm_state,
                          pl_received_hot_rst,
                          pl_sel_link_rate,
                          pl_sel_link_width,
                          pl_directed_link_auton,
                          pl_directed_link_change,
                          pl_directed_link_speed,
                          pl_directed_link_width,
                          pl_upstream_prefer_deemph,
                          speed_change_done_n,

                          trn_lnk_up_n,
                          trn_clk,
                          trn_reset_n
                    
                          );


input [2:0]              pl_initial_link_width;
input [1:0]              pl_lane_reversal_mode;
input                    pl_link_gen2_capable;
input                    pl_link_partner_gen2_supported;
input                    pl_link_upcfg_capable;
input [5:0]              pl_ltssm_state;
input                    pl_received_hot_rst;
input                    pl_sel_link_rate;
input [1:0]              pl_sel_link_width;
output                   pl_directed_link_auton;
output [1:0]             pl_directed_link_change;
output                   pl_directed_link_speed;
output [1:0]             pl_directed_link_width;
output                   pl_upstream_prefer_deemph;
output                   speed_change_done_n;


input                    trn_lnk_up_n;
input                    trn_clk;
input                    trn_reset_n;

parameter                Tcq = 1;
parameter                LINK_CAP_MAX_LINK_SPEED = 4'h1;

reg                      pl_directed_link_auton;
reg [1:0]                pl_directed_link_change;
reg                      pl_directed_link_speed;
reg [1:0]                pl_directed_link_width;
reg                      pl_upstream_prefer_deemph;
reg                      speed_change_done_n;

initial begin

   pl_directed_link_auton <= 1'b0;
   pl_directed_link_change <= 2'b0;
   pl_directed_link_speed <= 1'b0;
   pl_directed_link_width <= 2'b0;
   pl_upstream_prefer_deemph <= 1'b0;
   speed_change_done_n <= 1'b1;

   if (LINK_CAP_MAX_LINK_SPEED == 4'h2) begin

     wait (trn_lnk_up_n == 1'b0);

     if (pl_link_gen2_capable && pl_link_partner_gen2_supported) begin

       wait (pl_sel_link_rate == 1'h1);
       wait (pl_ltssm_state == 6'h16);

       speed_change_done_n <= 1'b0; 

     end
   end

end

endmodule // pci_exp_usrapp_pl

