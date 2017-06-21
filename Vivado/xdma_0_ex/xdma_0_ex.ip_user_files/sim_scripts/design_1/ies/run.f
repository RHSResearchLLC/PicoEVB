-makelib ies/xil_defaultlib -sv \
  "/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
  "/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \
-endlib
-makelib ies/xpm \
  "/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_VCOMP.vhd" \
-endlib
-makelib ies/xil_defaultlib \
  "../../../bd/design_1/hdl/design_1.v" \
-endlib
-makelib ies/xil_defaultlib \
  glbl.v
-endlib

