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
// File       : pci_exp_usrapp_cfg.v
// Version    : $IpVersion 
//-----------------------------------------------------------------------------
//--
//--------------------------------------------------------------------------------

`include "board_common.vh"
//------------------------------------------------------------------------------------------------------------------//

module pci_exp_usrapp_cfg (

  user_clk,
  user_reset,
  // Management Interface
  cfg_mgmt_addr,
  cfg_mgmt_write,
  cfg_mgmt_write_data,
  cfg_mgmt_byte_enable, 

  cfg_mgmt_read,
  cfg_mgmt_read_data,
  cfg_mgmt_read_write_done,
  cfg_mgmt_type1_cfg_reg_access,
  //-------------------------------------------------------------------------------------------//
  // 4. Configuration (CFG) Interface                                                          //
  //-------------------------------------------------------------------------------------------//
  // EP and RP                                                                                 //
  //-------------------------------------------------------------------------------------------//

  cfg_phy_link_down,
  cfg_phy_link_status,
  cfg_negotiated_width,
  cfg_current_speed,
  cfg_max_payload,
  cfg_max_read_req,
  cfg_function_status,
  cfg_function_power_state,
  cfg_vf_status,
  cfg_vf_power_state,
  cfg_link_power_state,

  // Error Reporting Interface
  cfg_err_cor_out,
  cfg_err_nonfatal_out,
  cfg_err_fatal_out,
  //cfg_local_error,

  cfg_ltr_enable,
  cfg_ltssm_state,
  cfg_rcb_status,
  cfg_dpa_substate_change,
  cfg_obff_enable,
  cfg_pl_status_change,

  cfg_tph_requester_enable,
  cfg_tph_st_mode,
  cfg_vf_tph_requester_enable,
  cfg_vf_tph_st_mode,

  cfg_msg_received,
  cfg_msg_received_data,
  cfg_msg_received_type,

  cfg_msg_transmit,
  cfg_msg_transmit_type,
  cfg_msg_transmit_data,
  cfg_msg_transmit_done,

  cfg_fc_ph,
  cfg_fc_pd,
  cfg_fc_nph,
  cfg_fc_npd,
  cfg_fc_cplh,
  cfg_fc_cpld,
  cfg_fc_sel,
  cfg_per_func_status_control,
  cfg_per_func_status_data,
  cfg_per_function_number,
  cfg_per_function_output_request,
  cfg_per_function_update_done,

  cfg_dsn,
  cfg_power_state_change_ack,
  cfg_power_state_change_interrupt,
  cfg_err_cor_in,
  cfg_err_uncor_in,

  cfg_flr_in_process,
  cfg_flr_done,
  cfg_vf_flr_in_process,
  cfg_vf_flr_done,

  cfg_link_training_enable,
  cfg_ds_port_number,

  cfg_ext_read_received,
  cfg_ext_write_received,
  cfg_ext_register_number,
  cfg_ext_function_number,
  cfg_ext_write_data,
  cfg_ext_write_byte_enable,
  cfg_ext_read_data,
  cfg_ext_read_data_valid,
  cfg_interrupt_msi_enable,
  cfg_interrupt_msi_vf_enable,
  cfg_interrupt_msi_mmenable,
  cfg_interrupt_msi_mask_update,
  cfg_interrupt_msi_data,
  cfg_interrupt_msi_select,
  cfg_interrupt_msi_int,
  cfg_interrupt_msi_pending_status,
  cfg_interrupt_msi_sent,
  cfg_interrupt_msi_fail,
  cfg_interrupt_msi_attr,
  cfg_interrupt_msi_tph_present,
  cfg_interrupt_msi_tph_type,
  cfg_interrupt_msi_tph_st_tag,
  cfg_interrupt_msi_function_number,

  cfg_hot_reset_out,
  cfg_config_space_enable,
  cfg_req_pm_transition_l23_ready,
  //-------------------------------------------------------------------------------------------//
  // RP Only                                                                                   //
  //-------------------------------------------------------------------------------------------//
  cfg_hot_reset_in,

  cfg_ds_bus_number,
  cfg_ds_device_number,
  cfg_ds_function_number,
  // Interrupt Interface Signals
  cfg_interrupt_int,
  cfg_interrupt_pending,
  cfg_interrupt_sent

);

  input                                 user_clk;
  input                                 user_reset;
  // Management Interface
  output reg                     [18:0]     cfg_mgmt_addr;
  output reg                                cfg_mgmt_write;
  output reg                     [31:0]     cfg_mgmt_write_data;
  output reg                      [3:0]     cfg_mgmt_byte_enable; 

  output reg                                cfg_mgmt_read;
  input                      [31:0]     cfg_mgmt_read_data;
  input                                 cfg_mgmt_read_write_done;
  output reg                                cfg_mgmt_type1_cfg_reg_access;
  //-------------------------------------------------------------------------------------------//
  // 4. Configuration (CFG) Interface                                                          //
  //-------------------------------------------------------------------------------------------//
  // EP and RP                                                                                 //
  //-------------------------------------------------------------------------------------------//

  input                                 cfg_phy_link_down;
  input                       [1:0]     cfg_phy_link_status;
  input                       [3:0]     cfg_negotiated_width;
  input                       [2:0]     cfg_current_speed;
  input                       [2:0]     cfg_max_payload;
  input                       [2:0]     cfg_max_read_req;
  input                      [15:0]     cfg_function_status;
  input                      [11:0]     cfg_function_power_state;
  input                      [15:0]     cfg_vf_status;
  input                      [23:0]     cfg_vf_power_state;
  input                       [1:0]     cfg_link_power_state;

  // Error Reporting Interface
  input                                 cfg_err_cor_out;
  input                                 cfg_err_nonfatal_out;
  input                                 cfg_err_fatal_out;
  //input                                 cfg_local_error;

  input                                 cfg_ltr_enable;
  input                       [5:0]     cfg_ltssm_state;
  input                       [3:0]     cfg_rcb_status;
  input                       [3:0]     cfg_dpa_substate_change;
  input                       [1:0]     cfg_obff_enable;
  input                                 cfg_pl_status_change;
  input                       [3:0]     cfg_tph_requester_enable;
  input                      [11:0]     cfg_tph_st_mode;
  input                       [7:0]     cfg_vf_tph_requester_enable;
  input                      [23:0]     cfg_vf_tph_st_mode;

  input                                 cfg_msg_received;
  input                       [7:0]     cfg_msg_received_data;
  input                       [4:0]     cfg_msg_received_type;

  output reg                                cfg_msg_transmit;
  output reg                      [2:0]     cfg_msg_transmit_type;
  output reg                     [31:0]     cfg_msg_transmit_data;
  input                                 cfg_msg_transmit_done;

  input                       [7:0]     cfg_fc_ph;
  input                      [11:0]     cfg_fc_pd;
  input                       [7:0]     cfg_fc_nph;
  input                      [11:0]     cfg_fc_npd;
  input                       [7:0]     cfg_fc_cplh;
  input                      [11:0]     cfg_fc_cpld;
  output reg                      [2:0]     cfg_fc_sel;

  output reg                      [2:0]     cfg_per_func_status_control;
  input                      [15:0]     cfg_per_func_status_data;
  output reg                      [2:0]     cfg_per_function_number;
  output reg                                cfg_per_function_output_request;
  input                                 cfg_per_function_update_done;

  output reg                     [63:0]     cfg_dsn;
  output reg                                cfg_power_state_change_ack;
  input                                 cfg_power_state_change_interrupt;
  output reg                                cfg_err_cor_in;
  output reg                                cfg_err_uncor_in;

  input                       [3:0]     cfg_flr_in_process;
  output reg                      [1:0]     cfg_flr_done;
  input                       [7:0]     cfg_vf_flr_in_process;
  output reg                      [7:0]     cfg_vf_flr_done;

  output reg                                cfg_link_training_enable;
  output reg                      [7:0]     cfg_ds_port_number;
  input                                 cfg_ext_read_received;
  input                                 cfg_ext_write_received;
  input                       [9:0]     cfg_ext_register_number;
  input                       [7:0]     cfg_ext_function_number;
  input                      [31:0]     cfg_ext_write_data;
  input                       [3:0]     cfg_ext_write_byte_enable;
  output reg                     [31:0]     cfg_ext_read_data;
  output reg                                cfg_ext_read_data_valid;

  // Interrupt Interface Signals
  output reg                      [3:0]     cfg_interrupt_int;
  output reg                      [1:0]     cfg_interrupt_pending;
  input                                 cfg_interrupt_sent;

  input                       [3:0]     cfg_interrupt_msi_enable;
  input                       [7:0]     cfg_interrupt_msi_vf_enable;
  input                      [11:0]     cfg_interrupt_msi_mmenable;
  input                                 cfg_interrupt_msi_mask_update;
  input                      [31:0]     cfg_interrupt_msi_data;
  output reg                      [3:0]     cfg_interrupt_msi_select;
  output reg                     [31:0]     cfg_interrupt_msi_int;
  output reg                     [63:0]     cfg_interrupt_msi_pending_status;
  input                                 cfg_interrupt_msi_sent;
  input                                 cfg_interrupt_msi_fail;
  output reg                      [2:0]     cfg_interrupt_msi_attr;
  output reg                                cfg_interrupt_msi_tph_present;
  output reg                      [1:0]     cfg_interrupt_msi_tph_type;
  output reg                      [8:0]     cfg_interrupt_msi_tph_st_tag;
  output reg                      [2:0]     cfg_interrupt_msi_function_number;

  input                                 cfg_hot_reset_out;
  output reg                                cfg_config_space_enable;
  output reg                                cfg_req_pm_transition_l23_ready;
  //-------------------------------------------------------------------------------------------//
  // RP Only                                                                                   //
  //-------------------------------------------------------------------------------------------//
  output reg                                cfg_hot_reset_in;

  output reg                      [7:0]     cfg_ds_bus_number;
  output reg                      [4:0]     cfg_ds_device_number;
  output reg                      [2:0]     cfg_ds_function_number;
//----------------------------------------------------------------------------------------------------------------//

parameter                  Tcq = 1;

//----------------------------------------------------------------------------------------------------------------//

initial begin

  cfg_mgmt_addr        <= 0;
  cfg_mgmt_write_data  <= 0;
  cfg_mgmt_byte_enable <= 0;
  cfg_mgmt_write       <= 0;
  cfg_mgmt_read        <= 0;


  cfg_mgmt_type1_cfg_reg_access    <= 0 ;//       
  cfg_msg_transmit                 <= 0 ;// 
  cfg_msg_transmit_type            <= 0 ;//[2:0]
  cfg_msg_transmit_data            <= 0 ;//[31:0]      
  cfg_fc_sel                       <= 0 ;//[2:0]     

  cfg_per_func_status_control      <= 0 ;//[2:0]             
  cfg_per_function_output_request  <= 0 ;//      
  cfg_per_function_number          <= 0; //[2:0]

  cfg_dsn                          <= 64'h78EE32BAD28F906B ;//[63:0]       
  cfg_power_state_change_ack       <= 0 ;//    
  cfg_err_cor_in                   <= 0 ;
  cfg_err_uncor_in                 <= 0 ;//     

  cfg_flr_done                     <= 0 ;//[1:0]
  cfg_vf_flr_done                  <= 0 ;//[5:0]    

  cfg_link_training_enable         <= 1 ;//         

  cfg_ds_port_number               <= 8'h9F ;//[7:0] 
  cfg_ext_read_data                <= 32'hA234567B ;//[31:0]     
  cfg_ext_read_data_valid          <= 0 ;//       


  // Interrupt Interface Signals
  cfg_interrupt_int                <= 0 ;//[3:0]  
  cfg_interrupt_pending            <= 0 ;//[1:0]      

  cfg_interrupt_msi_select         <= 0 ;//[3:0]        
  cfg_interrupt_msi_int            <= 0 ;//[31:0]  
  cfg_interrupt_msi_pending_status <= 0 ;//[63:0]             
  cfg_interrupt_msi_select         <= 0 ;//[3:0]        
  cfg_interrupt_msi_attr           <= 0 ;//[2:0] 
  cfg_interrupt_msi_tph_present    <= 0 ;//        
  cfg_interrupt_msi_tph_type       <= 0 ;//[1:0]       
  cfg_interrupt_msi_tph_st_tag     <= 0 ;//[8:0]        
  cfg_interrupt_msi_function_number<= 0 ;//[2:0]            

  cfg_config_space_enable          <= 1'b1 ;//    
  cfg_req_pm_transition_l23_ready  <= 0 ;//         
  // RP Only                                                   //                             
  cfg_hot_reset_in                 <= 0 ;//        

  cfg_ds_bus_number                <= board.RP.tx_usrapp.RP_BUS_DEV_FNS[15:8]; //[7:0]
  cfg_ds_device_number             <= board.RP.tx_usrapp.RP_BUS_DEV_FNS[7:3];  //[4:0]
  cfg_ds_function_number           <= board.RP.tx_usrapp.RP_BUS_DEV_FNS[2:0];  //[2:0]
end
/********************************************************************************************************************
Task : TSK_READ_CFG_DW                
Description : Read Configuration Space DW
*********************************************************************************************************************/


task TSK_READ_CFG_DW;

input   [31:0]   addr_;

begin           

  if (user_reset) begin
  
    $display("[%t] : user_reset is asserted", $realtime);
    $finish(1); 
  
  end

 // wait ( cfg_mgmt_read_write_done == 1'b1)

  @(posedge user_clk);
  cfg_mgmt_addr  <= #(Tcq) addr_;
  cfg_mgmt_write <= #(Tcq) 1'b0;
  cfg_mgmt_read  <= #(Tcq) 1'b1;

  $display("[%t] : Reading Cfg Addr [0x%h]", $realtime, addr_);
  $fdisplay(board.RP.com_usrapp.tx_file_ptr,
            "\n[%t] : Local Configuration Read Access :", 
            $realtime);
                 
  @(posedge user_clk); 
  #(Tcq);
  wait ( cfg_mgmt_read_write_done == 1'b1)
  #(Tcq);

  $fdisplay(board.RP.com_usrapp.tx_file_ptr,
            "\t\t\tCfg Addr [0x%h] -> Data [0x%h]\n", 
            {addr_,2'b00}, cfg_mgmt_read_data);
  cfg_mgmt_read <= #(Tcq) 1'b0;
         
end

endtask // TSK_READ_CFG_DW;

/********************************************************************************************************************
Task : TSK_WRITE_CFG_DW
Description : Write Configuration Space DW
*********************************************************************************************************************/

task TSK_WRITE_CFG_DW;

input   [31:0]   addr_;
input   [31:0]   data_;
input   [3:0]    ben_;

begin

  if (user_reset) begin

    $display("[%t] : trn_reset_n is asserted", $realtime);
    $finish(1);

  end

  //wait ( cfg_mgmt_read_write_done == 1'b1)

  @(posedge user_clk);
  cfg_mgmt_addr        <= #(Tcq) addr_;
  cfg_mgmt_write_data  <= #(Tcq) data_;
  cfg_mgmt_byte_enable <= #(Tcq) ben_;
  cfg_mgmt_write       <= #(Tcq) 1'b1;
  cfg_mgmt_read        <= #(Tcq) 1'b0;

  $display("[%t] : Writing Cfg Addr [0x%h]", $realtime, addr_);
  $fdisplay(board.RP.com_usrapp.tx_file_ptr,
            "\n[%t] : Local Configuration Write Access :",
            $realtime);

  @(posedge user_clk);
  #(Tcq);
  wait ( cfg_mgmt_read_write_done == 1'b1)
  #(Tcq);

  cfg_mgmt_addr        <= #(Tcq) 0;
  cfg_mgmt_write       <= #(Tcq) 1'b0;
  cfg_mgmt_byte_enable <= #(Tcq) 0;
  @(posedge user_clk);
  @(posedge user_clk);

end

endtask // TSK_WRITE_CFG_DW;

//----------------------------------------------------------------------------------------------------------------//

endmodule // pci_exp_usrapp_cfg
//----------------------------------------------------------------------------------------------------------------//


