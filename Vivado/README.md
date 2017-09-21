# Vivado Projects

To get started with the Vivado project you need Vivado 2017.2.

## Getting Started
- Open Vivado
- In TCL prompt, change directory to desired FPGA project (e.g. nanoevb)
- source project.tcl

Once Vivado is done loading project, you can edit project or simply
run synthesis and implementation

## Nanoevb
- Implements DMA for PCIe IP
- 8kB BRAM connected to DMA port
- Sanity check constant 0xDEADBEEF on AXI-Lite interface
