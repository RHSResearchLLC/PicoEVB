##-----------------------------------------------------------------------------
##
## (c) Copyright 2012-2012 Xilinx, Inc. All rights reserved.
##
## This file contains confidential and proprietary information
## of Xilinx, Inc. and is protected under U.S. and
## international copyright and other intellectual property
## laws.
##
## DISCLAIMER
## This disclaimer is not a license and does not grant any
## rights to the materials distributed herewith. Except as
## otherwise provided in a valid license issued to you by
## Xilinx, and to the maximum extent permitted by applicable
## law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
## WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
## AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
## BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
## INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
## (2) Xilinx shall not be liable (whether in contract or tort,
## including negligence, or under any other theory of
## liability) for any loss or damage of any kind or nature
## related to, arising under or in connection with these
## materials, including for any direct, or any indirect,
## special, incidental, or consequential loss or damage
## (including loss of data, profits, goodwill, or any type of
## loss or damage suffered as a result of any action brought
## by a third party) even if such damage or loss was
## reasonably foreseeable or Xilinx had been advised of the
## possibility of the same.
##
## CRITICAL APPLICATIONS
## Xilinx products are not designed or intended to be fail-
## safe, or for use in any application requiring fail-safe
## performance, such as life-support or safety devices or
## systems, Class III medical devices, nuclear facilities,
## applications related to the deployment of airbags, or any
## other applications that could lead to death, personal
## injury, or severe property or environmental damage
## (individually and collectively, "Critical
## Applications"). Customer assumes the sole risk and
## liability of any use of Xilinx products in Critical
## Applications, subject only to applicable laws and
## regulations governing limitations on product liability.
##
## THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
## PART OF THIS FILE AT ALL TIMES.
##
##-----------------------------------------------------------------------------
##
## Project    : The Xilinx PCI Express DMA
## File       : xilinx_xdma_pcie_x0y0.xdc
## Version    : $IpVersion
##-----------------------------------------------------------------------------
###############################################################################
# User Configuration
# Link Width   - x1
# Link Speed   - gen1
# Family       - artix7
# Part         - xc7a50t
# Package      - csg325
# Speed grade  - -1
# PCIe Block   - X0Y0
###############################################################################
#
#########################################################################################################################
# User Constraints
#########################################################################################################################

###############################################################################
# User Time Names / User Time Groups / Time Specs
###############################################################################

###############################################################################
# User Physical Constraints
###############################################################################

#########################################################################################################################
# End User Constraints
#########################################################################################################################
#
#
#
#########################################################################################################################
# PCIE Core Constraints
#########################################################################################################################


###############################################################################
# Pinout and Related I/O Constraints
###############################################################################
#
# SYS reset (input) signal.  The sys_reset_n signal should be
# obtained from the PCI Express interface if possible.  For
# slot based form factors, a system reset signal is usually
# present on the connector.  For cable based form factors, a
# system reset signal may not be available.  In this case, the
# system reset signal must be generated locally by some form of
# supervisory circuit.  You may change the IOSTANDARD and LOC
# to suit your requirements and VCCO voltage banking rules.
# Some 7 series devices do not have 3.3 V I/Os available.
# Therefore the appropriate level shift is required to operate
# with these devices that contain only 1.8 V banks.
#

set_property IOSTANDARD LVCMOS33 [get_ports sys_rst_n]
set_property PULLUP true [get_ports sys_rst_n]

set_property CONFIG_VOLTAGE 1.8 [current_design]
set_property CFGBVS GND [current_design]

#
#
# SYS clock 0 MHz (input) signal. The sys_clk_p and sys_clk_n
# signals are the PCI Express reference clock. Virtex-7 GT
# Transceiver architecture requires the use of a dedicated clock
# resources (FPGA input pins) associated with each GT Transceiver.
# To use these pins an IBUFDS primitive (refclk_ibuf) is
# instantiated in user's design.
# Please refer to the Virtex-7 GT Transceiver User Guide
# (UG) for guidelines regarding clock resource selection.
#
set_property PACKAGE_PIN B6 [get_ports sys_clk_p]

###############################################################################
# Timing Constraints
###############################################################################

create_clock -period 10.000 -name sys_clk [get_ports sys_clk_p]

###############################################################################
# Physical Constraints
###############################################################################

set_false_path -from [get_ports sys_rst_n]

#########################################################################################################################
# End PCIe Core Constraints
#########################################################################################################################

set_property IOSTANDARD LVCMOS33 [get_ports {status_leds[3]}]
set_property IOSTANDARD LVCMOS33 [get_ports {status_leds[2]}]
set_property IOSTANDARD LVCMOS33 [get_ports {status_leds[1]}]
set_property IOSTANDARD LVCMOS33 [get_ports {status_leds[0]}]
set_property PACKAGE_PIN V12 [get_ports {status_leds[0]}]
set_property PACKAGE_PIN V13 [get_ports {status_leds[2]}]
set_property PACKAGE_PIN V14 [get_ports {status_leds[3]}]
set_property PULLUP true [get_ports {status_leds[3]}]
set_property PULLUP true [get_ports {status_leds[2]}]
set_property PULLUP true [get_ports {status_leds[1]}]
set_property PULLUP true [get_ports {status_leds[0]}]
set_property DRIVE 8 [get_ports {status_leds[3]}]
set_property DRIVE 8 [get_ports {status_leds[2]}]
set_property DRIVE 8 [get_ports {status_leds[1]}]
set_property DRIVE 8 [get_ports {status_leds[0]}]
set_property PACKAGE_PIN A10 [get_ports sys_rst_n]

set_property PACKAGE_PIN A9 [get_ports clkreq_l]
set_property IOSTANDARD LVCMOS33 [get_ports clkreq_l]
set_property PULLDOWN true [get_ports clkreq_l]

# High-speed configuration so FPGA is up in time to negotiate with PCIe root complex
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property CONFIG_MODE SPIx4 [current_design]
set_property BITSTREAM.CONFIG.SPI_FALL_EDGE YES [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

set_property PACKAGE_PIN V11 [get_ports {status_leds[1]}]

set_property OFFCHIP_TERM NONE [get_ports TxD]
set_property IOSTANDARD LVCMOS33 [get_ports RxD]
set_property PACKAGE_PIN V17 [get_ports RxD]
set_property PACKAGE_PIN V16 [get_ports TxD]
set_property IOSTANDARD LVCMOS33 [get_ports TxD]
set_property PULLUP true [get_ports RxD]
