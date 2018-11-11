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
Sheet 6 7
Title "PicoEVB"
Date "2017-10-17"
Rev "D"
Comp "RHS Research, LLC"
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L C C67
U 1 1 587DA52B
P 5250 1150
F 0 "C67" V 5300 950 50  0000 L CNN
F 1 "0.1uf" V 5200 900 50  0000 L CNN
F 2 "SMT:c_0201_least" H 5288 1000 50  0001 C CNN
F 3 "~" H 5250 1150 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 5250 1150 60  0001 C CNN "MPN"
F 5 "~" H 5250 1150 60  0001 C CNN "MFG"
	1    5250 1150
	0    -1   -1   0   
$EndComp
$Comp
L GND #PWR050
U 1 1 587DA6D2
P 5600 1150
F 0 "#PWR050" H 5600 900 50  0001 C CNN
F 1 "GND" H 5600 1000 50  0000 C CNN
F 2 "" H 5600 1150 50  0000 C CNN
F 3 "" H 5600 1150 50  0000 C CNN
	1    5600 1150
	0    -1   -1   0   
$EndComp
$Comp
L C C68
U 1 1 587DA891
P 5250 1350
F 0 "C68" V 5300 1150 50  0000 L CNN
F 1 "0.1uf" V 5200 1100 50  0000 L CNN
F 2 "SMT:c_0201_least" H 5288 1200 50  0001 C CNN
F 3 "~" H 5250 1350 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 5250 1350 60  0001 C CNN "MPN"
F 5 "~" H 5250 1350 60  0001 C CNN "MFG"
	1    5250 1350
	0    -1   -1   0   
$EndComp
$Comp
L C C69
U 1 1 587DA8D6
P 5250 1550
F 0 "C69" V 5300 1350 50  0000 L CNN
F 1 "0.1uf" V 5200 1300 50  0000 L CNN
F 2 "SMT:c_0201_least" H 5288 1400 50  0001 C CNN
F 3 "~" H 5250 1550 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 5250 1550 60  0001 C CNN "MPN"
F 5 "~" H 5250 1550 60  0001 C CNN "MFG"
	1    5250 1550
	0    -1   -1   0   
$EndComp
Text GLabel 2800 2300 0    60   BiDi ~ 0
USB_2-
Text GLabel 2800 2400 0    60   BiDi ~ 0
USB_2+
Text GLabel 5900 2100 2    60   Output ~ 0
JTAG_TDI_B
Text GLabel 5900 2200 2    60   Input ~ 0
JTAG_TDO_B
Text GLabel 5900 2000 2    60   Output ~ 0
JTAG_TCK_B
Text GLabel 5900 2300 2    60   Output ~ 0
JTAG_TMS_B
Wire Wire Line
	5400 1150 5600 1150
Wire Wire Line
	5400 1350 5550 1350
Wire Wire Line
	5550 1150 5550 1550
Connection ~ 5550 1150
Wire Wire Line
	5550 1550 5400 1550
Connection ~ 5550 1350
Wire Wire Line
	5100 1350 5000 1350
Wire Wire Line
	5000 1150 5000 1550
Wire Wire Line
	3900 1550 5100 1550
Connection ~ 5000 1350
Wire Wire Line
	3900 2600 3700 2600
Wire Wire Line
	3900 2300 2800 2300
Wire Wire Line
	3900 2400 2800 2400
Wire Wire Line
	5300 2000 5900 2000
Wire Wire Line
	5300 2100 5900 2100
Wire Wire Line
	5300 2200 5900 2200
Wire Wire Line
	5300 2300 5900 2300
NoConn ~ 5300 2500
NoConn ~ 5300 2600
NoConn ~ 5300 2700
NoConn ~ 5300 2800
$Comp
L +3V3 #PWR051
U 1 1 5881D588
P 4700 1400
F 0 "#PWR051" H 4700 1250 50  0001 C CNN
F 1 "+3V3" H 4700 1540 50  0000 C CNN
F 2 "" H 4700 1400 50  0000 C CNN
F 3 "" H 4700 1400 50  0000 C CNN
	1    4700 1400
	1    0    0    -1  
$EndComp
$Comp
L +3V3 #PWR052
U 1 1 594F3248
P 3700 2600
F 0 "#PWR052" H 3700 2450 50  0001 C CNN
F 1 "+3V3" H 3700 2740 50  0000 C CNN
F 2 "" H 3700 2600 50  0000 C CNN
F 3 "" H 3700 2600 50  0000 C CNN
	1    3700 2600
	0    -1   -1   0   
$EndComp
Text Notes 6400 7000 0    118  ~ 24
USB-based JTAG cable
Text Notes 6550 2200 0    118  ~ 24
JTAG to FPGA
$Comp
L FT230XQ U7
U 1 1 59323D6A
P 4600 2400
F 0 "U7" H 4050 3000 50  0000 L CNN
F 1 "FT230XQ" H 4900 3000 50  0000 L CNN
F 2 "Housings_DFN_QFN:QFN-16-1EP_4x4mm_Pitch0.65mm" H 4600 2400 50  0001 C CNN
F 3 "" H 4600 2400 50  0001 C CNN
F 4 "FT230XQ" H 4600 2400 60  0001 C CNN "MPN"
	1    4600 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 2000 3900 1550
Wire Wire Line
	4500 1700 4500 1550
Connection ~ 4500 1550
$Comp
L GND #PWR053
U 1 1 593248AC
P 4600 3200
F 0 "#PWR053" H 4600 2950 50  0001 C CNN
F 1 "GND" H 4600 3050 50  0000 C CNN
F 2 "" H 4600 3200 50  0000 C CNN
F 3 "" H 4600 3200 50  0000 C CNN
	1    4600 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3100 4600 3200
Wire Wire Line
	4500 3100 4500 3150
Wire Wire Line
	4500 3150 4700 3150
Connection ~ 4600 3150
Wire Wire Line
	4700 3150 4700 3100
Wire Wire Line
	4700 1400 4700 1700
Connection ~ 4700 1550
Wire Wire Line
	5100 1150 5000 1150
Connection ~ 5000 1550
$EndSCHEMATC
