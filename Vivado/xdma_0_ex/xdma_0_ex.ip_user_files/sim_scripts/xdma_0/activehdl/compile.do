vlib work
vlib activehdl

vlib activehdl/xil_defaultlib
vlib activehdl/xpm
vlib activehdl/blk_mem_gen_v8_3_5
vlib activehdl/fifo_generator_v13_1_3
vlib activehdl/xdma_v3_0_1

vmap xil_defaultlib activehdl/xil_defaultlib
vmap xpm activehdl/xpm
vmap blk_mem_gen_v8_3_5 activehdl/blk_mem_gen_v8_3_5
vmap fifo_generator_v13_1_3 activehdl/fifo_generator_v13_1_3
vmap xdma_v3_0_1 activehdl/xdma_v3_0_1

vlog -work xil_defaultlib  -sv2k12 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_cdc/hdl/xpm_cdc.sv" \
"/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_memory/hdl/xpm_memory.sv" \

vcom -work xpm -93 \
"/home/dr/Programs/Xilinx/Vivado/2016.4/data/ip/xpm/xpm_VCOMP.vhd" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_clock.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_eq.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_drp.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_rate.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_reset.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_sync.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gtp_pipe_rate.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gtp_pipe_drp.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gtp_pipe_reset.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_user.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pipe_wrapper.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_qpll_drp.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_qpll_reset.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_qpll_wrapper.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_rxeq_scan.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_top.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_core_top.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_rx_null_gen.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_rx_pipeline.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_rx.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_top.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_tx_pipeline.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_tx_thrtl_ctl.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_axi_basic_tx.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_7x.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_bram_7x.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_bram_top_7x.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_brams_7x.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_pipe_lane.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_pipe_misc.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie_pipe_pipeline.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gt_top.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gt_common.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gtp_cpllpd_ovrd.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gtx_cpllpd_ovrd.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gt_rx_valid_filter_7x.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_gt_wrapper.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/source/xdma_0_pcie2_ip_pcie2_top.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_0/sim/xdma_0_pcie2_ip.v" \

vlog -work blk_mem_gen_v8_3_5  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../ipstatic/simulation/blk_mem_gen_v8_3.v" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_1/sim/xdma_0_blk_mem_64_reg_be.v" \

vlog -work fifo_generator_v13_1_3  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../ipstatic/simulation/fifo_generator_vlog_beh.v" \

vcom -work fifo_generator_v13_1_3 -93 \
"../../../ipstatic/hdl/fifo_generator_v13_1_rfs.vhd" \

vlog -work fifo_generator_v13_1_3  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../ipstatic/hdl/fifo_generator_v13_1_rfs.v" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_2/sim/xdma_0_fifo_generator_64_parity_v7.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_3/sim/pcie2_fifo_generator_dma_cpl.v" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/ip_4/sim/pcie2_fifo_generator_tgt_brdg.v" \

vlog -work xdma_v3_0_1  -sv2k12 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../ipstatic/ipshared/be6f/hdl/xdma_v3_0_vl_rfs.sv" \

vlog -work xil_defaultlib  -sv2k12 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_dma_cpl.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_dma_req.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v2_0/hdl/verilog/xdma_0_rx_destraddler.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_rx_demux.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_tgt_cpl.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_tgt_req.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_tx_mux.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_axi_stream_intf.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_cfg_sideband.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_pcie2_to_pcie3_wrapper.sv" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/dma_fifo_wrap.v" \

vlog -work xil_defaultlib  -sv2k12 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/dma_bram_wrap.sv" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/xdma_v3_0/hdl/verilog/xdma_0_core_top.sv" \

vlog -work xil_defaultlib  -v2k5 "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" "+incdir+../../../ipstatic/ipshared/be6f/hdl/verilog" \
"../../../../xdma_0_ex.srcs/sources_1/ip/xdma_0/sim/xdma_0.v" \

vlog -work xil_defaultlib "glbl.v"

