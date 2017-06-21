onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib xdma_0_opt

do {wave.do}

view wave
view structure
view signals

do {xdma_0.udo}

run -all

quit -force
