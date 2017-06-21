onbreak {quit -f}
onerror {quit -f}

vsim -voptargs="+acc" -t 1ps -L xil_defaultlib -L xpm -L blk_mem_gen_v8_3_5 -L unisims_ver -L unimacro_ver -L secureip -lib xil_defaultlib xil_defaultlib.blk_mem_gen_1 xil_defaultlib.glbl

do {wave.do}

view wave
view structure
view signals

do {blk_mem_gen_1.udo}

run -all

quit -force
