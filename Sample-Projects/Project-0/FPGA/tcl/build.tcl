# Use these to run synth/implementation
reset_run synth_1
launch_runs synth_1 -jobs 6
wait_on_run synth_1
launch_runs impl_1 -jobs 6 -to_step write_bitstream
wait_on_run impl_1 
puts "Implementation done!"

write_cfgmem -format mcs -size 4 -interface SPIx4 -force -loadbit "up 0x00000000 ./picoevb.runs/impl_1/project_bd_wrapper.bit" -file "./mcs/out.mcs"

