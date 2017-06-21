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
// File       : sample_tests.vh
// Version    : $IpVersion 
//-----------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

else if(testname =="dma_stream0")
begin

   //----------------------------------------------------------------------------------------
   // XDMA H2C Test Starts
   //----------------------------------------------------------------------------------------

    $display(" **** XDMA AXI-ST *** \n");
    $display(" **** read Address at BAR0  = %h\n", board.RP.tx_usrapp.BAR_INIT_P_BAR[0][31:0]);
    $display(" **** read Address at BAR1  = %h\n", board.RP.tx_usrapp.BAR_INIT_P_BAR[1][31:0]);

    //-------------- Load DATA in Buffer ----------------------------------------------------
    board.RP.tx_usrapp.TSK_INIT_DATA_H2C;
    board.RP.tx_usrapp.TSK_INIT_DATA_C2H;

    board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h00);
      
    //-------------- Descriptor start address for both H2C and C2H --------------------------
    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h4080, 32'h00000100, 4'hF);
    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h5080, 32'h00000300, 4'hF);
      
    //-------------- Start DMA tranfer ------------------------------------------------------
    $display(" **** Start DMA Stream for both H2C and C2H transfer ***\n");

    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h1004, 32'h7, 4'hF);
    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h0004, 32'h7, 4'hF);

    // Wait for data transfer complete.

    // For this example design there is 1 descriptor for H2c and 1 for C2H
    // Read C2H Descriptor count and wiat until it returns 1.
    // Becase it is a loopback, by reading C2H descriptor count to 1
    // it ensures H2C descriptor is also set to 1.   
    while (desc_count == 0) begin

          board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h1048);
          $display ("**** C2H Decsriptor Count = %h\n", P_READ_DATA);
          if (P_READ_DATA == 32'h1)
            desc_count = 1;
    end

    // Read status of both H2C and C2H engines.
    board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h1040);
    board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h0040);
    // Disable run bit for H2C and C2H engine
    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h1004, 32'h6, 4'hF);
    board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h0004, 32'h6, 4'hF);

    #100;  
   $finish;
end

else if(testname =="dma_test0")
begin

    //------------- This test performs a 32 bit write to a 32 bit Memory space and performs a read back

	//----------------------------------------------------------------------------------------
	// XDMA H2C Test Starts
	//----------------------------------------------------------------------------------------

    $display(" *** XDMA H2C *** \n");

    $display(" **** read Address at BAR0  = %h\n", board.RP.tx_usrapp.BAR_INIT_P_BAR[0][31:0]);
    $display(" **** read Address at BAR1  = %h\n", board.RP.tx_usrapp.BAR_INIT_P_BAR[1][31:0]);

    //-------------- Load DATA in Buffer ----------------------------------------------------
      board.RP.tx_usrapp.TSK_INIT_DATA_H2C;

	//-------------- DMA Engine ID Read -----------------------------------------------------
      board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h00);
      
    //-------------- Descriptor start address x0100 -----------------------------------------
	  board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h4080, 32'h00000100, 4'hF);
      
    //-------------- Start DMA tranfer ------------------------------------------------------
      $display(" **** Start DMA H2C transfer ***\n");

    fork
    //-------------- Writing XDMA CFG Register to start DMA Transfer for H2C ----------------
      board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h04, 32'h7, 4'hF);

    //-------------- compare H2C data -------------------------------------------------------
      $display("------Compare H2C Data--------\n");
      board.RP.tx_usrapp.COMPARE_DATA_H2C(32'h40);    //input payload bytes
    join

    //For this Example Design there is only one Descriptor used, so Descriptor Count would be 1
      while (desc_count == 0) begin

          board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h48);
          $display ("Decsriptor Count = %h\n", P_READ_DATA);
          if (P_READ_DATA == 32'h1)
            desc_count = 1;

      end

      board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h40);
      $display ("DMA_STATUS  = %h\n", P_READ_DATA); // bit2 : Descriptor completed; bit1: Descriptor end; bit0: DMA Stopped
	  $display ("bit2 : Descriptor completed; bit1: Descriptor end; bit0: DMA Stopped\n");

    //-------------- XDMA H2C and C2H Transfer separated by 1000ns --------------------------
      #1000;

    //----------------------------------------------------------------------------------------
    // XDMA C2H Test Starts
    //----------------------------------------------------------------------------------------
	
      $display(" *** XDMA C2H *** \n");

      desc_count = 0;

    //-------------- Load DATA in Buffer ----------------------------------------------------
      board.RP.tx_usrapp.TSK_INIT_DATA_C2H;

    //-------------- Descriptor start address x0300 -----------------------------------------
      board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h5080, 32'h00000300, 4'hF);

    // Start DMA transfer
      $display(" **** Start DMA C2H transfer ***\n");

    fork
    //-------------- Writing XDMA CFG Register to start DMA Transfer for C2H ----------------
      board.RP.tx_usrapp.TSK_XDMA_REG_WRITE(16'h1004, 32'h7, 4'hF);

    //compare C2H data
      $display("------Compare C2H Data--------\n");
      board.RP.tx_usrapp.COMPARE_DATA_C2H(32'h40);
    join

    //For this Example Design there is only one Descriptor used, so Descriptor Count would be 1

      while (desc_count == 0) begin

          board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h1048);
          $display ("Decsriptor Count = %h\n", P_READ_DATA);
          if (P_READ_DATA == 32'h1)
            desc_count = 1;
      end

      board.RP.tx_usrapp.TSK_XDMA_REG_READ(16'h1040);
      $display ("DMA_STATUS  = %h\n", P_READ_DATA); // bit2 : Descriptor completed; bit1: Descriptor end; bit0: DMA Stopped
      $display ("bit2 : Descriptor completed; bit1: Descriptor end; bit0: DMA Stopped\n");



      #100;

    #1000;


    $finish;
end

else if(testname == "sample_smoke_test0")
begin


    TSK_SIMULATION_TIMEOUT(5050);

    //System Initialization
    TSK_SYSTEM_INITIALIZATION;




    
    $display("[%t] : Expected Device/Vendor ID = %x", $realtime, DEV_VEN_ID); 
    
    //--------------------------------------------------------------------------
    // Read core configuration space via PCIe fabric interface
    //--------------------------------------------------------------------------

    $display("[%t] : Reading from PCI/PCI-Express Configuration Register 0x00", $realtime);

    TSK_TX_TYPE0_CONFIGURATION_READ(DEFAULT_TAG, 12'h0, 4'hF);
    TSK_WAIT_FOR_READ_DATA;
    if  (P_READ_DATA != DEV_VEN_ID) begin
        $display("[%t] : TEST FAILED --- Data Error Mismatch, Write Data %x != Read Data %x", $realtime, 
                                    DEV_VEN_ID, P_READ_DATA);
    end
    else begin
        $display("[%t] : TEST PASSED --- Device/Vendor ID %x successfully received", $realtime, P_READ_DATA);
        $display("[%t] : Test Completed Successfully",$realtime);
    end

    //--------------------------------------------------------------------------
    // Direct Root Port to allow upstream traffic by enabling Mem, I/O and
    // BusMstr in the command register
    //--------------------------------------------------------------------------

    board.RP.cfg_usrapp.TSK_READ_CFG_DW(32'h00000001);
    board.RP.cfg_usrapp.TSK_WRITE_CFG_DW(32'h00000001, 32'h00000007, 4'b0001);
    board.RP.cfg_usrapp.TSK_READ_CFG_DW(32'h00000001);

  $finish;
end


else if(testname == "sample_smoke_test1")
begin

    // This test use tlp expectation tasks.

    TSK_SIMULATION_TIMEOUT(5050);

    // System Initialization
    TSK_SYSTEM_INITIALIZATION;
    // Program BARs (Required so Completer ID at the Endpoint is updated)
    TSK_BAR_INIT;

fork
  begin
    //--------------------------------------------------------------------------
    // Read core configuration space via PCIe fabric interface
    //--------------------------------------------------------------------------

    $display("[%t] : Reading from PCI/PCI-Express Configuration Register 0x00", $realtime);

    TSK_TX_TYPE0_CONFIGURATION_READ(DEFAULT_TAG, 12'h0, 4'hF);
    DEFAULT_TAG = DEFAULT_TAG + 1;
    TSK_TX_CLK_EAT(100);
  end
    //---------------------------------------------------------------------------
    // List Rx TLP expections
    //---------------------------------------------------------------------------
  begin
    test_vars[0] = 0;                                                                                                                         
                                          
    $display("[%t] : Expected Device/Vendor ID = %x", $realtime, DEV_VEN_ID);                                              

    expect_cpld_payload[0] = DEV_VEN_ID[31:24];
    expect_cpld_payload[1] = DEV_VEN_ID[23:16];
    expect_cpld_payload[2] = DEV_VEN_ID[15:8];
    expect_cpld_payload[3] = DEV_VEN_ID[7:0];
    @(posedge pcie_rq_tag_vld);
    exp_tag = pcie_rq_tag;

    board.RP.com_usrapp.TSK_EXPECT_CPLD(
      3'h0, //traffic_class;
      1'b0, //td;
      1'b0, //ep;
      2'h0, //attr;
      10'h1, //length;
      board.RP.tx_usrapp.EP_BUS_DEV_FNS, //completer_id;
      3'h0, //completion_status;
      1'b0, //bcm;
      12'h4, //byte_count;
      board.RP.tx_usrapp.RP_BUS_DEV_FNS, //requester_id;
      exp_tag ,
      7'b0, //address_low;
      expect_status //expect_status;
    );

    if (expect_status) 
      test_vars[0] = test_vars[0] + 1;      
  end
join
  
  expect_finish_check = 1;

  if (test_vars[0] == 1) begin
    $display("[%t] : TEST PASSED --- Finished transmission of PCI-Express TLPs", $realtime);
    $display("[%t] : Test Completed Successfully",$realtime);
  end else begin
    $display("[%t] : TEST FAILED --- Haven't Received All Expected TLPs", $realtime);

    //--------------------------------------------------------------------------
    // Direct Root Port to allow upstream traffic by enabling Mem, I/O and
    // BusMstr in the command register
    //--------------------------------------------------------------------------

    board.RP.cfg_usrapp.TSK_READ_CFG_DW(32'h00000001);
    board.RP.cfg_usrapp.TSK_WRITE_CFG_DW(32'h00000001, 32'h00000007, 4'b0001);
    board.RP.cfg_usrapp.TSK_READ_CFG_DW(32'h00000001);

  end

  $finish;
end

else if(testname == "pio_writeReadBack_test0")
begin

    // This test performs a 32 bit write to a 32 bit Memory space and performs a read back

    board.RP.tx_usrapp.TSK_SIMULATION_TIMEOUT(10050);

    board.RP.tx_usrapp.TSK_SYSTEM_INITIALIZATION;

    board.RP.tx_usrapp.TSK_BAR_INIT;

//--------------------------------------------------------------------------
// Event : Testing BARs
//--------------------------------------------------------------------------

        for (board.RP.tx_usrapp.ii = 0; board.RP.tx_usrapp.ii <= 6; board.RP.tx_usrapp.ii =
            board.RP.tx_usrapp.ii + 1) begin
            if (board.RP.tx_usrapp.BAR_INIT_P_BAR_ENABLED[board.RP.tx_usrapp.ii] > 2'b00) // bar is enabled
               case(board.RP.tx_usrapp.BAR_INIT_P_BAR_ENABLED[board.RP.tx_usrapp.ii])
                   2'b01 : // IO SPACE
                        begin

                          $display("[%t] : Transmitting TLPs to IO Space BAR %x", $realtime, board.RP.tx_usrapp.ii);

                          //--------------------------------------------------------------------------
                          // Event : IO Write bit TLP
                          //--------------------------------------------------------------------------



                          board.RP.tx_usrapp.TSK_TX_IO_WRITE(board.RP.tx_usrapp.DEFAULT_TAG,
                             board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0], 4'hF, 32'hdead_beef);
                             @(posedge pcie_rq_tag_vld);
                             exp_tag = pcie_rq_tag;


                          board.RP.com_usrapp.TSK_EXPECT_CPL(3'h0, 1'b0, 1'b0, 2'b0,
                             board.RP.tx_usrapp.EP_BUS_DEV_FNS, 3'h0, 1'b0, 12'h4,
                             board.RP.tx_usrapp.RP_BUS_DEV_FNS, exp_tag,
                             board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0], test_vars[0]);

                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(10);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;

                          //--------------------------------------------------------------------------
                          // Event : IO Read bit TLP
                          //--------------------------------------------------------------------------


                          // make sure P_READ_DATA has known initial value
                          board.RP.tx_usrapp.P_READ_DATA = 32'hffff_ffff;
                          fork
                             board.RP.tx_usrapp.TSK_TX_IO_READ(board.RP.tx_usrapp.DEFAULT_TAG,
                                board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0], 4'hF);
                             board.RP.tx_usrapp.TSK_WAIT_FOR_READ_DATA;
                          join
                          if  (board.RP.tx_usrapp.P_READ_DATA != 32'hdead_beef)
                             begin
			       testError=1'b1;
                               $display("[%t] : Test FAILED --- Data Error Mismatch, Write Data %x != Read Data %x",
                                   $realtime, 32'hdead_beef, board.RP.tx_usrapp.P_READ_DATA);
                             end
                          else
                             begin
                               $display("[%t] : Test PASSED --- Write Data: %x successfully received",
                                   $realtime, board.RP.tx_usrapp.P_READ_DATA);
                             end


                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(10);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;


                        end

                   2'b10 : // MEM 32 SPACE
                        begin


                          $display("[%t] : Transmitting TLPs to Memory 32 Space BAR %x", $realtime,
                              board.RP.tx_usrapp.ii);

                          //--------------------------------------------------------------------------
                          // Event : Memory Write 32 bit TLP
                          //--------------------------------------------------------------------------

                          board.RP.tx_usrapp.DATA_STORE[0] = 8'h04;
                          board.RP.tx_usrapp.DATA_STORE[1] = 8'h03;
                          board.RP.tx_usrapp.DATA_STORE[2] = 8'h02;
                          board.RP.tx_usrapp.DATA_STORE[3] = 8'h01;

                          board.RP.tx_usrapp.TSK_TX_MEMORY_WRITE_32(board.RP.tx_usrapp.DEFAULT_TAG,
                              board.RP.tx_usrapp.DEFAULT_TC, 11'd1,
                              board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0]+8'h10, 4'h0, 4'hF, 1'b0);
                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(100);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;

                          //--------------------------------------------------------------------------
                          // Event : Memory Read 32 bit TLP
                          //--------------------------------------------------------------------------


                         // make sure P_READ_DATA has known initial value
                         board.RP.tx_usrapp.P_READ_DATA = 32'hffff_ffff;
                          fork
                             board.RP.tx_usrapp.TSK_TX_MEMORY_READ_32(board.RP.tx_usrapp.DEFAULT_TAG,
                                 board.RP.tx_usrapp.DEFAULT_TC, 11'd1,
                                 board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0]+8'h10, 4'h0, 4'hF);
                             board.RP.tx_usrapp.TSK_WAIT_FOR_READ_DATA;
                          join
                          if  (board.RP.tx_usrapp.P_READ_DATA != {board.RP.tx_usrapp.DATA_STORE[3],
                             board.RP.tx_usrapp.DATA_STORE[2], board.RP.tx_usrapp.DATA_STORE[1],
                             board.RP.tx_usrapp.DATA_STORE[0] })
                             begin
			       testError=1'b1;
                               $display("[%t] : Test FAILED --- Data Error Mismatch, Write Data %x != Read Data %x",
                                    $realtime, {board.RP.tx_usrapp.DATA_STORE[3],board.RP.tx_usrapp.DATA_STORE[2],
                                     board.RP.tx_usrapp.DATA_STORE[1],board.RP.tx_usrapp.DATA_STORE[0]},
                                     board.RP.tx_usrapp.P_READ_DATA);

                             end
                          else
                             begin
                               $display("[%t] : Test PASSED --- Write Data: %x successfully received",
                                   $realtime, board.RP.tx_usrapp.P_READ_DATA);
                             end


                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(10);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;

                     end
                2'b11 : // MEM 64 SPACE
                     begin


                          $display("[%t] : Transmitting TLPs to Memory 64 Space BAR %x", $realtime,
                              board.RP.tx_usrapp.ii);


                          //--------------------------------------------------------------------------
                          // Event : Memory Write 64 bit TLP
                          //--------------------------------------------------------------------------

                          board.RP.tx_usrapp.DATA_STORE[0] = 8'h64;
                          board.RP.tx_usrapp.DATA_STORE[1] = 8'h63;
                          board.RP.tx_usrapp.DATA_STORE[2] = 8'h62;
                          board.RP.tx_usrapp.DATA_STORE[3] = 8'h61;

                          board.RP.tx_usrapp.TSK_TX_MEMORY_WRITE_64(board.RP.tx_usrapp.DEFAULT_TAG,
                              board.RP.tx_usrapp.DEFAULT_TC, 10'd1,
                              {board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii+1][31:0],
                              board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0]+8'h20}, 4'h0, 4'hF, 1'b0);
                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(10);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;

                          //--------------------------------------------------------------------------
                          // Event : Memory Read 64 bit TLP
                          //--------------------------------------------------------------------------


                          // make sure P_READ_DATA has known initial value
                          board.RP.tx_usrapp.P_READ_DATA = 32'hffff_ffff;
                          fork
                             board.RP.tx_usrapp.TSK_TX_MEMORY_READ_64(board.RP.tx_usrapp.DEFAULT_TAG,
                                 board.RP.tx_usrapp.DEFAULT_TC, 10'd1,
                                 {board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii+1][31:0],
                                 board.RP.tx_usrapp.BAR_INIT_P_BAR[board.RP.tx_usrapp.ii][31:0]+8'h20}, 4'h0, 4'hF);
                             board.RP.tx_usrapp.TSK_WAIT_FOR_READ_DATA;
                          join
                          if  (board.RP.tx_usrapp.P_READ_DATA != {board.RP.tx_usrapp.DATA_STORE[3],
                             board.RP.tx_usrapp.DATA_STORE[2], board.RP.tx_usrapp.DATA_STORE[1],
                             board.RP.tx_usrapp.DATA_STORE[0] })

                             begin
			       testError=1'b1;
                               $display("[%t] : Test FAILED --- Data Error Mismatch, Write Data %x != Read Data %x",
                                   $realtime, {board.RP.tx_usrapp.DATA_STORE[3],
                                   board.RP.tx_usrapp.DATA_STORE[2], board.RP.tx_usrapp.DATA_STORE[1],
                                   board.RP.tx_usrapp.DATA_STORE[0]}, board.RP.tx_usrapp.P_READ_DATA);

                             end
                          else
                             begin
                               $display("[%t] : Test PASSED --- Write Data: %x successfully received",
                                   $realtime, board.RP.tx_usrapp.P_READ_DATA);
                             end


                          board.RP.tx_usrapp.TSK_TX_CLK_EAT(10);
                          board.RP.tx_usrapp.DEFAULT_TAG = board.RP.tx_usrapp.DEFAULT_TAG + 1;


                     end
                default : $display("Error case in usrapp_tx\n");
            endcase

         end
	 if(testError==1'b0)
            $display("[%t] : Test Completed Successfully",$realtime);


    $display("[%t] : Finished transmission of PCI-Express TLPs", $realtime);
    $finish;
end
