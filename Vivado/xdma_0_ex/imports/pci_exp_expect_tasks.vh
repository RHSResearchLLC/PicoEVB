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
// File       : pci_exp_expect_tasks.vh
// Version    : $IpVersion 
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------

`define EXPECT_CPLD_PAYLOAD board.RP.tx_usrapp.expect_cpld_payload
`define EXPECT_MEMWR_PAYLOAD board.RP.tx_usrapp.expect_memwr_payload
`define EXPECT_MEMWR64_PAYLOAD board.RP.tx_usrapp.expect_memwr64_payload

reg [31:0] error_file_ptr;

initial
begin
  error_file_ptr = $fopen("error.dat");
  if (!error_file_ptr) begin
    $write("ERROR: Could not open error.dat.\n");
    $finish;
  end
end

/************************************************************
Task : TSK_EXPECT_CPLD
Inputs : traffic_class, td, ep, attr, length, payload
Outputs : status 0-Failed 1-Successful
Description : Expecting a TLP from Rx side with matching
              traffic_class, td, ep, attr, length and payload
*************************************************************/
task TSK_EXPECT_CPLD;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] completer_id;
  input   [2:0]  completion_status;
  input          bcm;
  input   [11:0] byte_count;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [6:0]  address_low;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] completer_id_;
  reg   [2:0]  completion_status_;
  reg          bcm_;
  reg   [11:0] byte_count_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [6:0]  address_low_;

  integer      payload_len;
  integer      i_;
  reg          wait_for_next;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_cpld; //wait for a rcvd_cpld event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      bcm_ = frame_store_rx[6] >> 4;
      completion_status_= frame_store_rx[6] >> 5;
      byte_count_ = (frame_store_rx[6]);
      byte_count_ = (byte_count_ << 8) | frame_store_rx[7];
      completer_id_ = {frame_store_rx[4], frame_store_rx[5]};
      requester_id_= {frame_store_rx[8], frame_store_rx[9]};
      tag_= frame_store_rx[10];
      address_low_ = frame_store_rx[11];

      payload_len = (bcm_) ? byte_count_ : (length << 2);
      if (payload_len==0) payload_len = 4096;

      $display("[%t] : Received CPLD --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (bcm == bcm_) &&
           (completion_status == completion_status_) &&
           (byte_count == byte_count_) &&
           (completer_id == completer_id_) &&
           (requester_id == requester_id_) &&
           (address_low == address_low_))
        begin
          // find matching header then compare payload
          for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
            if(`EXPECT_CPLD_PAYLOAD[i_] != frame_store_rx[12 + i_]) //find mismatch
            begin
              $fdisplay(error_file_ptr, "[%t] : Found payload mismatch in received CPLD - Tag 0x%h: \n", $time, tag_);
              $fdisplay(error_file_ptr, "Expected:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", `EXPECT_CPLD_PAYLOAD[i_]);
              $fdisplay(error_file_ptr, "Received:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", frame_store_rx[12+i_]);

              $fdisplay(error_file_ptr, "");
              expect_status = 1'b0;
              i_ = 5000;
            end
          //find matching frame
          if(i_ == payload_len)
            expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received CPLD - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t BCM: 0x%h", bcm);
          $fdisplay(error_file_ptr, "\t Completion Status: 0x%h", completion_status);
          $fdisplay(error_file_ptr, "\t Byte Count: 0x%h", byte_count);
          $fdisplay(error_file_ptr, "\t Completer ID: 0x%h", completer_id);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Lower Address: 0x%h", address_low);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t BCM: 0x%h", bcm_);
          $fdisplay(error_file_ptr, "\t Completion Status: 0x%h", completion_status_);
          $fdisplay(error_file_ptr, "\t Byte Count: 0x%h", byte_count_);
          $fdisplay(error_file_ptr, "\t Completer ID: 0x%h", completer_id_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Lower Address: 0x%h", address_low_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask


/************************************************************
Task : TSK_EXPECT_CPL
Inputs : traffic_class, td, ep, attr, length, payload
Outputs : status 0-Failed 1-Successful
Description : Expecting a TLP from Rx side with matching
              traffic_class, td, ep, attr and length
*************************************************************/
task TSK_EXPECT_CPL;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [15:0] completer_id;
  input   [2:0]  completion_status;
  input          bcm;
  input   [11:0] byte_count;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [6:0]  address_low;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [15:0] completer_id_;
  reg   [2:0]  completion_status_;
  reg          bcm_;
  reg   [11:0] byte_count_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [6:0]  address_low_;

  integer      i_;
  reg          wait_for_next;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_cpl; //wait for a rcvd_cpl event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      bcm_ = frame_store_rx[6] >> 4;
      completion_status_= frame_store_rx[6] >> 5;
      byte_count_ = (frame_store_rx[6]);
      byte_count_ = (byte_count_ << 8) | frame_store_rx[7];
      completer_id_ = {frame_store_rx[4], frame_store_rx[5]};
      requester_id_= {frame_store_rx[8], frame_store_rx[9]};
      tag_= frame_store_rx[10];
      address_low_ = frame_store_rx[11];

      $display("[%t] : Received CPL --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (bcm == bcm_) && (completion_status == completion_status_) &&
           (byte_count == byte_count_) &&
           (completer_id == completer_id_) &&
           (requester_id == requester_id_) &&
           (address_low == address_low_))
        begin
          // header matches
          expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received CPL - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t BCM: 0x%h", bcm);
          $fdisplay(error_file_ptr, "\t Completion Status: 0x%h", completion_status);
          $fdisplay(error_file_ptr, "\t Byte Count: 0x%h", byte_count);
          $fdisplay(error_file_ptr, "\t Completer ID: 0x%h", completer_id);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Lower Address: 0x%h", address_low);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t BCM: 0x%h", bcm_);
          $fdisplay(error_file_ptr, "\t Completion Status: 0x%h", completion_status_);
          $fdisplay(error_file_ptr, "\t Byte Count: 0x%h", byte_count_);
          $fdisplay(error_file_ptr, "\t Completer ID: 0x%h", completer_id_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Lower Address: 0x%h", address_low_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask


/************************************************************
Task : TSK_EXPECT_MEMRD
Inputs : traffic_class, td, ep, attr, length, last_dw_be,
         first_dw_be, address
Outputs : status 0-Failed 1-Successful
Description : Expecting a memory read (32-bit address) TLP
              from Rx side with matching header fields
*************************************************************/
task TSK_EXPECT_MEMRD;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  last_dw_be;
  input   [3:0]  first_dw_be;
  input   [29:0] address;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  last_dw_be_;
  reg   [3:0]  first_dw_be_;
  reg   [29:0] address_;

  integer      i_;
  reg          wait_for_next;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_memrd; //wait for a rcvd_memrd event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      last_dw_be_= frame_store_rx[7] >> 4;
      first_dw_be_= frame_store_rx[7];
      address_[29:6] = {frame_store_rx[8], frame_store_rx[9], frame_store_rx[10]};
      address_[5:0] = frame_store_rx[11] >> 2;

      $display("[%t] : Received MEMRD --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (requester_id == requester_id_) &&
           (last_dw_be == last_dw_be_) && (first_dw_be == first_dw_be_) &&
           (address == address_))
        begin
          // header matches
          expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received MEMRD - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be_);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be_);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask


/************************************************************
Task : TSK_EXPECT_MEMRD64
Inputs : traffic_class, td, ep, attr, length, last_dw_be,
         first_dw_be, address
Outputs : status 0-Failed 1-Successful
Description : Expecting a memory read (64-bit address) TLP
              from Rx side with matching header fields
*************************************************************/
task TSK_EXPECT_MEMRD64;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  last_dw_be;
  input   [3:0]  first_dw_be;
  input   [61:0] address;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  last_dw_be_;
  reg   [3:0]  first_dw_be_;
  reg   [61:0] address_;

  integer      i_;
  reg          wait_for_next;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_memrd64; //wait for a rcvd_memrd64 event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      last_dw_be_= frame_store_rx[7] >> 4;
      first_dw_be_= frame_store_rx[7];
      address_[61:6] = {frame_store_rx[8], frame_store_rx[9],
                        frame_store_rx[10], frame_store_rx[11],
                        frame_store_rx[12], frame_store_rx[13],
                        frame_store_rx[14]};
      address_[5:0] = frame_store_rx[15] >> 2;

      $display("[%t] : Received MEMRD64 --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (requester_id == requester_id_) &&
           (last_dw_be == last_dw_be_) && (first_dw_be == first_dw_be_) &&
           (address == address_))
        begin
          // header matches
          expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received MEMRD64 - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be_);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be_);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask


/************************************************************
Task : TSK_EXPECT_MEMWR
Inputs : traffic_class, td, ep, attr, length, last_dw_be,
         first_dw_be, address
Outputs : status 0-Failed 1-Successful
Description : Expecting a memory write (32-bit address) TLP
              from Rx side with matching header fields and
              payload
*************************************************************/
task TSK_EXPECT_MEMWR;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  last_dw_be;
  input   [3:0]  first_dw_be;
  input   [29:0] address;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  last_dw_be_;
  reg   [3:0]  first_dw_be_;
  reg   [29:0] address_;

  integer      payload_len;
  integer      i_;
  reg          wait_for_next;
  reg          check_byte;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_memwr; //wait for a rcvd_memwr event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      last_dw_be_= frame_store_rx[7] >> 4;
      first_dw_be_= frame_store_rx[7];
      address_[29:6] = {frame_store_rx[8], frame_store_rx[9], frame_store_rx[10]};
      address_[5:0] = frame_store_rx[11] >> 2;

      payload_len = (length << 2);
      if (payload_len==0) payload_len = 4096;

      $display("[%t] : Received MEMWR --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (requester_id == requester_id_) &&
           (last_dw_be == last_dw_be_) && (first_dw_be == first_dw_be_) &&
           (address == address_))
        begin
          // find matching header then compare payload
          for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
          begin
            check_byte = 1;
            if (i_ < 4) // apply first_dw_be
            begin
              if (first_dw_be[i_])
                check_byte = 1;
              else
                check_byte = 0;
            end else if (i_ > (payload_len - 5)) // apply last_dw_be
            begin
              if (last_dw_be[4 - (payload_len - i_)])
                check_byte = 1;
              else
                check_byte = 0;
            end
            if(check_byte && `EXPECT_MEMWR_PAYLOAD[i_] != frame_store_rx[12 + i_]) //find mismatch
            begin
              $fdisplay(error_file_ptr, "[%t] : Found payload mismatch in received MEMWR - Tag 0x%h: \n", $time, tag_);
              $fdisplay(error_file_ptr, "Expected:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", `EXPECT_MEMWR_PAYLOAD[i_]);
              $fdisplay(error_file_ptr, "Received:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", frame_store_rx[12+i_]);

              $fdisplay(error_file_ptr, "");
              expect_status = 1'b0;
              i_ = 5000;
            end
          end
          //find matching frame
          if(i_ == payload_len)
            expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received MEMWR - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be_);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be_);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask


/************************************************************
Task : TSK_EXPECT_MEMWR64
Inputs : traffic_class, td, ep, attr, length, last_dw_be,
         first_dw_be, address
Outputs : status 0-Failed 1-Successful
Description : Expecting a memory write (64-bit address) TLP
              from Rx side with matching header fields and
              payload
*************************************************************/
task TSK_EXPECT_MEMWR64;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  last_dw_be;
  input   [3:0]  first_dw_be;
  input   [61:0] address;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  last_dw_be_;
  reg   [3:0]  first_dw_be_;
  reg   [61:0] address_;

  integer      payload_len;
  integer      i_;
  reg          wait_for_next;
  reg          check_byte;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_memwr64; //wait for a rcvd_memwr64 event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      last_dw_be_= frame_store_rx[7] >> 4;
      first_dw_be_= frame_store_rx[7];
      address_[61:6] = {frame_store_rx[8], frame_store_rx[9],
                        frame_store_rx[10], frame_store_rx[11],
                        frame_store_rx[12], frame_store_rx[13],
                        frame_store_rx[14]};
      address_[5:0] = frame_store_rx[15] >> 2;

      payload_len = (length << 2);
      if (payload_len==0) payload_len = 4096;

      $display("[%t] : Received MEMWR64 --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (requester_id == requester_id_) &&
           (last_dw_be == last_dw_be_) && (first_dw_be == first_dw_be_) &&
           (address == address_))
        begin
          // find matching header then compare payload
          for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
          begin
            check_byte = 1;
            if (i_ < 4) // apply first_dw_be
            begin
              if (first_dw_be[i_])
                check_byte = 1;
              else
                check_byte = 0;
            end else if (i_ > (payload_len - 5)) // apply last_dw_be
            begin
              if (last_dw_be[4 - (payload_len - i_)])
                check_byte = 1;
              else
                check_byte = 0;
            end
            if(check_byte && `EXPECT_MEMWR64_PAYLOAD[i_] != frame_store_rx[16 + i_]) //find mismatch
            begin
              $fdisplay(error_file_ptr, "[%t] : Found payload mismatch in received MEMWR64 - Tag 0x%h: \n", $time, tag_);
              $fdisplay(error_file_ptr, "Expected:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", `EXPECT_MEMWR64_PAYLOAD[i_]);
              $fdisplay(error_file_ptr, "Received:");
              for (i_ = 0; i_ < payload_len; i_ = i_ + 1)
                $fdisplay(error_file_ptr,"\t %0x", frame_store_rx[16+i_]);

              $fdisplay(error_file_ptr, "");
              expect_status = 1'b0;
              i_ = 5000;
            end
          end
          //find matching frame
          if(i_ == payload_len)
            expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received MEMWR64 - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW byte-enable: 0x%h", last_dw_be_);
          $fdisplay(error_file_ptr, "\t First DW byte-enable: 0x%h", first_dw_be_);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask



// Please note that the following io tasks can be only used if the customer has a mechanism for allowing
// the customer design to generate io write or io read tlps received by the dsport rx port.


/************************************************************
Task : TSK_EXPECT_IOWRITE
Inputs : td, ep, requested_id, tag, firstDwBe, address, data
Outputs : status 0-Failed 1-Successful
Description : Expecting a TLP from Rx side with matching
              td, ep, requested_id, tag, firstDwBe, address, and 32 bit data
*************************************************************/
task TSK_EXPECT_IOWRITE;


  input          td;
  input          ep;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  firstDwBe;
  input   [31:0] address; // note that low bits [1:0] are not used
  input   [31:0] data; 

  output         expect_status;

  reg   [2:0]  traffic_class;  
  reg   [1:0]  attr;
  reg   [9:0]  length; 
  reg   [3:0]  lastDwBe;   

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  lastDwBe_;
  reg   [3:0]  firstDwBe_;
  reg   [31:0] address_; // note that the bottom two bits are not used in comparison

  reg   [7:0]  write_payload[0:3];


  reg   [3:0]  byte_enabled;
  integer      i_;
  reg          wait_for_next;
  integer      j_;
  
  begin
    // following assignments are required for io header
    traffic_class = 3'b000;
    attr = 2'b00;
    length = 10'b00_0000_0001;
    lastDwBe = 4'b0000;
    write_payload[0] = data[31:24];
    write_payload[1] = data[23:16];
    write_payload[2] = data[15:8];
    write_payload[3] = data[7:0];
 
    j_ = 1000;
    wait_for_next = 1'b1; //haven't found any matching tag yet
   fork 
    while(wait_for_next)
    begin
      @ rcvd_iowr; //wait for a rcvd_iowr event
      byte_enabled = 4'h0;
      traffic_class_ = frame_store_rx[1] >> 4; 
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4; 
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]); 
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      lastDwBe_ = frame_store_rx[7] >>4;
      firstDwBe_ = frame_store_rx[7];
      
      address_ = (frame_store_rx[8]);
      address_ = (address_ << 8) | frame_store_rx[9];
      address_ = (address_ << 8) | frame_store_rx[10];
      address_ = (address_ << 8) | frame_store_rx[11];
      
      $display("[%t] : Received IO WRITE TLP --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) &&
           (requester_id == requester_id_) && 
           (lastDwBe == lastDwBe_) &&
           (firstDwBe == firstDwBe_) &&           
           (address[31:2] == address_[31:2]))
        begin
          // find matching header then compare payload
            expect_status = 1'b1; //assume that we will succeed
            byte_enabled = firstDwBe;
            for (i_ = 0; i_ < 4; i_ = i_ + 1)
             begin
              if (byte_enabled[3] && expect_status)
               if (write_payload[i_] != frame_store_rx[12 + i_]) //find mismatch
               begin
                 $fdisplay(error_file_ptr, "[%t] : Found payload mismatch in IO WRITE DATA - Tag 0x%h: \n", $time, tag_);
                 $fdisplay(error_file_ptr, "Expected:");
                 for (i_ = 0; i_ < 4; i_ = i_ + 1)
                     $fdisplay(error_file_ptr,"\t %0x", write_payload[i_]);
                 $fdisplay(error_file_ptr, "Received:");
                 for (i_ = 0; i_ < 4; i_ = i_ + 1)
                    $fdisplay(error_file_ptr,"\t %0x", frame_store_rx[12+i_]);

                 $fdisplay(error_file_ptr, "");
                 expect_status = 1'b0;                 
               end
              byte_enabled = byte_enabled << 1; 
             end            
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received CPLD - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe); 
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe_);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe_); 
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end // while
    begin
      // time out task function and return 0 if tlp never received and/or tag never matches
      while (j_ && wait_for_next) 
        begin
         `TX_TASKS.TSK_TX_CLK_EAT(1); 
         j_ = j_ - 1;
        end
      if (wait_for_next) 
        begin
          $display("Failure: TSK_EXPECT_IOWRITE timeout. IO WRITE TLP never received or tag mismatch");
          $finish;
        end
    end
   join
  end // 1st
endtask



/************************************************************
Task : TSK_EXPECT_IOREAD
Inputs : td, ep, requested_id, tag, firstDwBe, address
Outputs : status 0-Failed 1-Successful
Description : Expecting a TLP from Rx side with matching
              td, ep, requested_id, tag, firstDwBe, and address
*************************************************************/
task TSK_EXPECT_IOREAD;



  input          td;
  input          ep;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  firstDwBe;
  input   [31:0] address; // note that low bits [1:0] are not used

  output         expect_status;

  reg   [2:0]  traffic_class;  
  reg   [1:0]  attr;
  reg   [9:0]  length; 
  reg   [3:0]  lastDwBe;   

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  lastDwBe_;
  reg   [3:0]  firstDwBe_;
  reg   [31:0] address_; // note that the bottom two bits are not used in comparison

  integer      i_;
  reg          wait_for_next;
  integer      j_;

  begin
    // following assignments are required for io header
    traffic_class = 3'b000;
    attr = 2'b00;
    length = 10'b00_0000_0001;
    lastDwBe = 4'b0000;

    j_ = 1000;    
    wait_for_next = 1'b1; //haven't found any matching tag yet
   fork
    while(wait_for_next)
    begin
      @ rcvd_iord; //wait for a rcvd_iord event
      traffic_class_ = frame_store_rx[1] >> 4; 
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4; 
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]); 
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      lastDwBe_ = frame_store_rx[7] >>4;
      firstDwBe_ = frame_store_rx[7];
      
      address_ = (frame_store_rx[8]);
      address_ = (address_ << 8) | frame_store_rx[9];
      address_ = (address_ << 8) | frame_store_rx[10];
      address_ = (address_ << 8) | frame_store_rx[11];
      
      $display("[%t] : Received IO READ TLP --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) &&
           (requester_id == requester_id_) && 
           (lastDwBe == lastDwBe_) &&
           (firstDwBe == firstDwBe_) &&           
           (address[31:2] == address_[31:2]))
        begin          
          expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received CPLD - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe); 
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe_);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe_); 
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end // while
    begin
      // time out task function and return 0 if tlp never received and/or tag never matches
      while (j_ && wait_for_next) 
        begin
         `TX_TASKS.TSK_TX_CLK_EAT(1); 
         j_ = j_ - 1;
        end
      if (wait_for_next) 
        begin
          $display("Failure: TSK_EXPECT_IOREAD timeout. IO READ TLP never received or tag mismatch");
          $finish;
        end
    end       
   join
  end //1st
endtask



// Please note that the following task, TSK_EXPECT_TYPE0_CONFIGURATION_WRITE, should not be exported
// to the customer because all the current express cores currently consume all Type 0 configuration
// accesses. This means that this task will always time out since a type 0 config tlp will never
// be received at the rx trn interface. This function is being included for completeness and in case
// a future release of an express core passes type 0 configuration writes through the express core 
// and on to the customer rx application.

// Also note that this function has not been tested completely due to the restrictions indicated above.


/************************************************************
Task : TSK_EXPECT_TYPE0_CONFIGURATION_WRITE
Inputs : td, ep, requested_id, tag, firstDwBe, , data
Outputs : status 0-Failed 1-Successful
Description : Expecting a TLP from Rx side with matching
              td, ep, requested_id, tag, firstDwBe, , and 32 bit data
*************************************************************/
task TSK_EXPECT_TYPE0_CONFIGURATION_WRITE;


  input          td;
  input          ep;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [3:0]  firstDwBe;
  input   [7:0]  busNumber;
  input   [4:0]  deviceNumber;
  input   [2:0]  functionNumber;
  input   [3:0]  extRegNumber;
  input   [5:0]  registerNumber;
  input   [31:0] data; 

  output         expect_status;

  reg   [2:0]  traffic_class;  
  reg   [1:0]  attr;
  reg   [9:0]  length; 
  reg   [3:0]  lastDwBe;   

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [3:0]  lastDwBe_;
  reg   [3:0]  firstDwBe_;
  reg   [7:0]  busNumber_;
  reg   [4:0]  deviceNumber_;
  reg   [2:0]  functionNumber_;
  reg   [3:0]  extRegNumber_;
  reg   [5:0]  registerNumber_;

  reg   [7:0]  write_payload[0:3];


  reg   [3:0]  byte_enabled;
  integer      i_;
  reg          wait_for_next;
  integer      j_;
  
  begin
    // following assignments are required for io header
    traffic_class = 3'b000;
    attr = 2'b00;
    length = 10'b00_0000_0001;
    lastDwBe = 4'b0000;
    write_payload[0] = data[31:24];
    write_payload[1] = data[23:16];
    write_payload[2] = data[15:8];
    write_payload[3] = data[7:0];
 
    j_ = 1000;
    wait_for_next = 1'b1; //haven't found any matching tag yet
   fork 
    while(wait_for_next)
    begin
      @ rcvd_cfgwr0; //wait for a rcvd_cfgwr0 event *** currently this event will never occur
      byte_enabled = 4'h0;
      traffic_class_ = frame_store_rx[1] >> 4; 
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4; 
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]); 
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      lastDwBe_ = frame_store_rx[7] >>4;
      firstDwBe_ = frame_store_rx[7];
      
      busNumber_ = frame_store_rx[8];
      deviceNumber_ = frame_store_rx[9] >> 3;
      functionNumber_ = frame_store_rx[9];
      extRegNumber_ = frame_store_rx[10];
      registerNumber_ = frame_store_rx[11] >> 2;
            
      $display("[%t] : Received TYPE 0 CFG WRITE TLP --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) &&
           (requester_id == requester_id_) && 
           (lastDwBe == lastDwBe_) &&
           (firstDwBe == firstDwBe_) &&  
           (busNumber == busNumber_) &&         
           (deviceNumber == deviceNumber_) &&
           (functionNumber == functionNumber_) &&
           (extRegNumber == extRegNumber_) &&
           (registerNumber == registerNumber_))          
        begin
          // find matching header then compare payload
            expect_status = 1'b1; //assume that we will succeed
            byte_enabled = firstDwBe;
            for (i_ = 0; i_ < 4; i_ = i_ + 1)
             begin
              if (byte_enabled[3] && expect_status)
               if (write_payload[i_] != frame_store_rx[12 + i_]) //find mismatch
               begin
                 $fdisplay(error_file_ptr, "[%t] : Found payload mismatch in TYPE 0 WRITE DATA - Tag 0x%h: \n", $time, tag_);
                 $fdisplay(error_file_ptr, "Expected:");
                 for (i_ = 0; i_ < 4; i_ = i_ + 1)
                     $fdisplay(error_file_ptr,"\t %0x", write_payload[i_]);
                 $fdisplay(error_file_ptr, "Received:");
                 for (i_ = 0; i_ < 4; i_ = i_ + 1)
                    $fdisplay(error_file_ptr,"\t %0x", frame_store_rx[12+i_]);

                 $fdisplay(error_file_ptr, "");
                 expect_status = 1'b0;                 
               end
              byte_enabled = byte_enabled << 1; 
             end            
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received CPLD - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe); 
          $fdisplay(error_file_ptr, "\t Bus Number: 0x%h", busNumber); 
          $fdisplay(error_file_ptr, "\t Device Number: 0x%h", deviceNumber); 
          $fdisplay(error_file_ptr, "\t Function Number: 0x%h", functionNumber); 
          $fdisplay(error_file_ptr, "\t Ext Reg Number: 0x%h", extRegNumber);                                         
          $fdisplay(error_file_ptr, "\t Register Number: 0x%h", registerNumber);           
         
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Last DW Byte Enable: 0x%h", lastDwBe_);
          $fdisplay(error_file_ptr, "\t 1st DW Byte Enable: 0x%h", firstDwBe_); 
          $fdisplay(error_file_ptr, "\t Bus Number: 0x%h", busNumber_); 
          $fdisplay(error_file_ptr, "\t Device Number: 0x%h", deviceNumber_); 
          $fdisplay(error_file_ptr, "\t Function Number: 0x%h", functionNumber_); 
          $fdisplay(error_file_ptr, "\t Ext Reg Number: 0x%h", extRegNumber_);                                         
          $fdisplay(error_file_ptr, "\t Register Number: 0x%h", registerNumber_);              
          
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end // while
    begin
      // time out task function and return 0 if tlp never received and/or tag never matches
      while (j_ && wait_for_next) 
        begin
         `TX_TASKS.TSK_TX_CLK_EAT(1); 
         j_ = j_ - 1;
        end
      if (wait_for_next) 
        begin
          $display("Failure: TSK_EXPECT_TYPE0_CONFIGURATION_WRITE timeout. CFG TYPE 0 WRITE TLP never received");
          $finish;
        end
    end
   join
  end // 1st
endtask


/************************************************************
Task : TSK_EXPECT_MSG
Inputs : traffic_class, td, ep, attr, length, msg type, msg code
        address
Outputs : status 0-Failed 1-Successful
Description : Expecting a message no data TLP
              from Rx side with matching header fields
*************************************************************/
task TSK_EXPECT_MSG;

  input   [2:0]  traffic_class;
  input          td;
  input          ep;
  input   [1:0]  attr;
  input   [9:0]  length;
  input   [15:0] requester_id;
  input   [7:0]  tag;
  input   [7:0]  msg_code;
  input   [29:0] address;

  output         expect_status;

  reg   [2:0]  traffic_class_;
  reg          td_;
  reg          ep_;
  reg   [1:0]  attr_;
  reg   [9:0]  length_;
  reg   [15:0] requester_id_;
  reg   [7:0]  tag_;
  reg   [7:0]  msg_code_;
  reg   [29:0] address_;

  integer      i_;
  reg          wait_for_next;

  begin
    wait_for_next = 1'b1; //haven't found any matching tag yet
    while(wait_for_next)
    begin
      @ rcvd_msg; //wait for a rcvd_memrd event
      traffic_class_ = frame_store_rx[1] >> 4;
      td_ = frame_store_rx[2] >> 7;
      ep_ = frame_store_rx[2] >> 6;
      attr_ = frame_store_rx[2] >> 4;
      length_ = frame_store_rx[2];
      length_ = (length_ << 8) | (frame_store_rx[3]);
      requester_id_= {frame_store_rx[4], frame_store_rx[5]};
      tag_= frame_store_rx[6];
      msg_code_ = frame_store_rx[7];

      address_[29:6] = {frame_store_rx[8], frame_store_rx[9], frame_store_rx[10]};
      address_[5:0] = frame_store_rx[11] >> 2;

      $display("[%t] : Received MSG w/o Data --- Tag 0x%h", $realtime, tag_);
      if(tag == tag_) //find matching tag
      begin
        wait_for_next = 1'b0;
        if((traffic_class == traffic_class_) &&
           (td === td_) && (ep == ep_) && (attr == attr_) &&
           (length == length_) && (requester_id == requester_id_) &&
           (msg_code == msg_code_) && (address == address_))
        begin
          // header matches
          expect_status = 1'b1;
        end
        else // header mismatches, error out
        begin
          $fdisplay(error_file_ptr, "[%t] : Found header mismatch in received MSG w/o Data - Tag 0x%h: \n", $time, tag_);
          $fdisplay(error_file_ptr, "Expected:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class);
          $fdisplay(error_file_ptr, "\t TD: %h", td);
          $fdisplay(error_file_ptr, "\t EP: %h", ep);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag);
          $fdisplay(error_file_ptr, "\t Msg Code: 0x%h", msg_code);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address);
          $fdisplay(error_file_ptr, "Received:");
          $fdisplay(error_file_ptr, "\t Traffic Class: 0x%h", traffic_class_);
          $fdisplay(error_file_ptr, "\t TD: %h", td_);
          $fdisplay(error_file_ptr, "\t EP: %h", ep_);
          $fdisplay(error_file_ptr, "\t Attributes: 0x%h", attr_);
          $fdisplay(error_file_ptr, "\t Length: 0x%h", length_);
          $fdisplay(error_file_ptr, "\t Requester ID: 0x%h", requester_id_);
          $fdisplay(error_file_ptr, "\t Tag: 0x%h", tag_);
          $fdisplay(error_file_ptr, "\t Msg Code: 0x%h", msg_code_);
          $fdisplay(error_file_ptr, "\t Address: 0x%h", address_);
          $fdisplay(error_file_ptr, "");
          expect_status = 1'b0;
        end
      end
    end
  end
endtask





