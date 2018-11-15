# Vivado Project

### PROM file for onboard flash
Once the bitstream is generated, in the Vivado TCL command prompt, from the Vivado/nanoevb project directory run the following command:

**Note: Make sure Vivado working directory is set to same dir as .xpr file: `cd <directory of .xpr file>`**

	write_cfgmem -force -format mcs -size 4 -interface SPIx4 \
	-loadbit {up 0x00000000 "./picoevb.runs/impl_1/project_bd_wrapper.bit" } \
	-file "../mcs/proj0.mcs"


### Programming onboard flash

First, make sure xvcd cable driver is running. Build the xvcd project, then run
`sudo ./xvcd -p 2542 -s i:0x0403:0x6015:0`

Then from vivado, run the following commands to connect to the xvcd driver:
`open_hw; connect_hw_server; open_hw_target -xvc_url localhost:2542`

At this point you should see the FPGA, and can use the dashboard to add monitors, if you like.
If you'd like to program the flash with the bitstream, you can run the following in Vivado
`source ./tcl/prog-flash.tcl`

**Note: Make sure Vivado working directory is set to same dir as .xpr file: `cd <directory of .xpr file>`**








