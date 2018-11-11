EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:Zilog
LIBS:zetex
LIBS:Xicor
LIBS:Worldsemi
LIBS:wiznet
LIBS:video
LIBS:ttl_ieee
LIBS:transf
LIBS:switches
LIBS:supertex
LIBS:stm32
LIBS:stm8
LIBS:silabs
LIBS:sensors
LIBS:rfcom
LIBS:relays
LIBS:references
LIBS:pspice
LIBS:Power_Management
LIBS:powerint
LIBS:Oscillators
LIBS:onsemi
LIBS:nxp_armmcu
LIBS:nordicsemi
LIBS:msp430
LIBS:motors
LIBS:motor_drivers
LIBS:microchip_pic32mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic10mcu
LIBS:microchip_dspic33dsc
LIBS:mechanical
LIBS:maxim
LIBS:logo
LIBS:Lattice
LIBS:ir
LIBS:hc11
LIBS:graphic
LIBS:gennum
LIBS:ftdi
LIBS:ESD_Protection
LIBS:elec-unifil
LIBS:diode
LIBS:dc-dc
LIBS:cmos_ieee
LIBS:brooktre
LIBS:bosch
LIBS:bbd
LIBS:battery_management
LIBS:analog_devices
LIBS:Altera
LIBS:allegro
LIBS:actel
LIBS:ac-dc
LIBS:74xgxx
LIBS:xc7a50t-bga325
LIBS:MAX1589A
LIBS:MIC47050
LIBS:TS3L110
LIBS:W25Q
LIBS:93LC46B-SOT23
LIBS:m2-board
LIBS:ft2232-fixed
LIBS:tps82084
LIBS:PicoEVB-cache
EELAYER 25 0
EELAYER END
$Descr A 11000 8500
encoding utf-8
Sheet 1 7
Title "PicoEVB"
Date "2017-10-17"
Rev "D"
Comp "RHS Research, LLC"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 1100 7350 500  500 
U 5881899D
F0 "FPGA Power & Config" 60
F1 "fpga_pwr_cfg_b.sch" 60
$EndSheet
$Sheet
S 2650 6550 500  500 
U 594F3239
F0 "FPGA PCIe" 60
F1 "fpga_pcie_b.sch" 60
$EndSheet
$Sheet
S 2650 7350 500  500 
U 594F323A
F0 "FPGA IO" 60
F1 "fpga_io_b.sch" 60
$EndSheet
$Sheet
S 1100 6550 500  500 
U 594F323B
F0 "Power" 60
F1 "power_b.sch" 60
$EndSheet
$Sheet
S 3900 6550 500  500 
U 588189A5
F0 "USB IO" 60
F1 "USB_IO_b.sch" 60
$EndSheet
$Sheet
S 3900 7350 500  500 
U 588189A7
F0 "M2 edge" 60
F1 "M2_edge_b.sch" 60
$EndSheet
Text Notes 6350 7000 0    118  ~ 24
TOP LEVEL
Text Notes 6850 2350 3    118  ~ 0
M.2 2230 A/E
Wire Notes Line
	6650 2000 6650 3800
Wire Notes Line
	6650 3800 6950 3800
Wire Notes Line
	6950 3800 6950 2000
Wire Notes Line
	6950 2000 6650 2000
Wire Notes Line
	5150 2050 5150 2700
Wire Notes Line
	5150 2700 5950 2700
Wire Notes Line
	5950 2700 5950 2000
Wire Notes Line
	5950 2000 5150 2000
Wire Notes Line
	5150 2000 5150 2100
Text Notes 5850 2250 2    118  ~ 0
FT230X
Wire Notes Line
	5950 2200 6650 2200
Text Notes 6150 2150 0    118  ~ 0
USB
Text Notes 4950 3600 0    118  ~ 0
Artix 7\nXC7A50T\nCSG325
Text Notes 5900 3750 0    118  ~ 0
PCIe x1
Wire Notes Line
	4850 3050 4850 3750
Wire Notes Line
	4850 3750 5800 3750
Wire Notes Line
	5800 3750 5800 3000
Wire Notes Line
	5800 3000 4850 3000
Text Notes 4600 2550 0    118  ~ 0
JTAG
Wire Notes Line
	5150 2600 4650 2600
Wire Notes Line
	4650 2600 4650 3150
Wire Notes Line
	4650 3150 4850 3150
Text Notes 5100 4200 0    118  ~ 0
MGT\nLoopback\n
Wire Notes Line
	5050 3750 5050 4300
Wire Notes Line
	5050 4300 5950 4300
Wire Notes Line
	5950 4300 5950 4000
Wire Notes Line
	5950 4000 5500 4000
Wire Notes Line
	5500 4000 5500 3750
Text Notes 3550 4250 1    118  ~ 0
Analog/Digital\nI/O connector
Wire Notes Line
	3200 2950 3200 4300
Wire Notes Line
	3200 4300 3600 4300
Wire Notes Line
	3600 4300 3600 2950
Wire Notes Line
	3600 2950 3200 2950
Wire Notes Line
	3600 3600 4850 3600
Text Notes 3500 1700 0    118  ~ 0
PicoEVB Block Diagram
Text Notes 6100 4200 0    118  ~ 0
3 LEDs
Text Notes 4100 4150 0    118  ~ 0
CONFIG\nEEPROM
Wire Notes Line
	4000 3800 4000 4200
Wire Notes Line
	4000 4200 4850 4200
Wire Notes Line
	4850 4200 4850 3800
Wire Notes Line
	4850 3800 4000 3800
Wire Notes Line
	4850 4000 4950 4000
Wire Notes Line
	4950 4000 4950 3750
Wire Notes Line
	6900 4350 3400 4350
Wire Notes Line
	6900 3800 6900 4350
Text Notes 3550 2750 1    118  ~ 0
MGT conn\nU.FL (6)
Wire Notes Line
	3200 2800 3600 2800
Wire Notes Line
	3600 2800 3600 1850
Wire Notes Line
	3600 1850 3200 1850
Wire Notes Line
	3200 1850 3200 2800
Wire Notes Line
	3600 2700 4000 2700
Wire Notes Line
	4000 2700 4000 3450
Wire Notes Line
	3600 2200 4250 2200
Wire Notes Line
	4250 2200 4250 3300
Wire Notes Line
	4250 3300 4850 3300
Wire Notes Line
	4000 3450 4850 3450
Text Notes 3650 2650 0    118  ~ 0
MGT0
Text Notes 3700 2150 0    118  ~ 0
REFCLK0
Wire Notes Line
	6650 3600 5800 3600
Wire Notes Line
	5800 3100 6650 3100
Text Notes 5850 3250 0    118  ~ 0
3.3V I/O
Wire Notes Line
	4200 2700 4400 2700
Wire Notes Line
	3850 2950 4050 2950
Wire Notes Line
	3750 3650 3750 3500
Wire Notes Line
	3750 3500 3850 3500
Text Notes 4400 2700 0    79   ~ 16
2
Text Notes 3800 2950 0    79   ~ 16
4
Text Notes 3800 3500 0    79   ~ 16
4
Wire Notes Line
	6300 3150 6300 3000
Wire Notes Line
	6300 3000 6250 3000
Text Notes 6200 3000 0    79   ~ 16
4
Wire Notes Line
	3400 1850 3400 1750
Wire Notes Line
	3400 1750 6900 1750
Wire Notes Line
	6900 1750 6900 2000
Wire Notes Line
	3400 2800 3400 2950
Wire Notes Line
	3400 4350 3400 4300
Wire Notes Line
	6050 4000 6750 4000
Wire Notes Line
	6750 4000 6750 4300
Wire Notes Line
	6750 4300 6050 4300
Wire Notes Line
	6050 4300 6050 4000
Wire Notes Line
	5700 3750 5700 3900
Wire Notes Line
	5700 3900 6300 3900
Wire Notes Line
	6300 3900 6300 4000
$EndSCHEMATC
