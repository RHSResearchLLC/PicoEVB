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
Sheet 7 7
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
L GND #PWR054
U 1 1 58818EBF
P 5950 3850
F 0 "#PWR054" H 5950 3600 50  0001 C CNN
F 1 "GND" H 5950 3700 50  0000 C CNN
F 2 "" H 5950 3850 50  0000 C CNN
F 3 "" H 5950 3850 50  0000 C CNN
	1    5950 3850
	0    -1   -1   0   
$EndComp
$Comp
L +3V3 #PWR055
U 1 1 58818EC5
P 4450 3450
F 0 "#PWR055" H 4450 3300 50  0001 C CNN
F 1 "+3V3" H 4450 3590 50  0000 C CNN
F 2 "" H 4450 3450 50  0000 C CNN
F 3 "" H 4450 3450 50  0000 C CNN
	1    4450 3450
	0    -1   -1   0   
$EndComp
Text GLabel 6200 3050 2    60   BiDi ~ 0
USB_2-
Text GLabel 6200 2950 2    60   BiDi ~ 0
USB_2+
$Comp
L M.2-AE PCB1
U 1 1 58818ECE
P 5300 3900
F 0 "PCB1" H 4900 5350 60  0000 C CNN
F 1 "M.2-AE" H 5850 5350 60  0000 C CNN
F 2 "M2-Board:M.2-AE-22x42" H 4850 4000 60  0001 C CNN
F 3 "~" H 4850 4000 60  0001 C CNN
F 4 "xxx" H 4850 4000 60  0001 C CNN "MPN"
F 5 "~" H 4850 4000 60  0001 C CNN "MFG"
	1    5300 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3450 4450 3450
Wire Wire Line
	4600 3750 4600 3450
Connection ~ 4600 3550
Connection ~ 4600 3650
$Comp
L GND #PWR056
U 1 1 58818ED9
P 4600 4350
F 0 "#PWR056" H 4600 4100 50  0001 C CNN
F 1 "GND" H 4600 4200 50  0000 C CNN
F 2 "" H 4600 4350 50  0000 C CNN
F 3 "" H 4600 4350 50  0000 C CNN
	1    4600 4350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4600 3850 4600 4350
Connection ~ 5950 3550
Connection ~ 5950 3650
Connection ~ 5950 3750
Connection ~ 5950 3950
Connection ~ 5950 4050
Connection ~ 5950 4150
Connection ~ 4600 4250
Connection ~ 4600 4150
Connection ~ 4600 4050
Connection ~ 4600 3950
Wire Wire Line
	6200 2950 5950 2950
Wire Wire Line
	6200 3050 5950 3050
NoConn ~ 5950 2600
Wire Wire Line
	4450 3100 4600 3100
Wire Wire Line
	4600 3200 4450 3200
$Comp
L C C72
U 1 1 594F3242
P 3950 2950
F 0 "C72" V 4000 3000 50  0000 L CNN
F 1 "0.1uf" V 3900 3000 50  0000 L CNN
F 2 "SMT:c_0201_least" H 3988 2800 50  0001 C CNN
F 3 "~" H 3950 2950 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 3950 2950 60  0001 C CNN "MPN"
F 5 "~" H 3950 2950 60  0001 C CNN "MFG"
	1    3950 2950
	0    -1   1    0   
$EndComp
$Comp
L C C70
U 1 1 58818F05
P 3650 2850
F 0 "C70" V 3700 2900 50  0000 L CNN
F 1 "0.1uf" V 3600 2900 50  0000 L CNN
F 2 "SMT:c_0201_least" H 3688 2700 50  0001 C CNN
F 3 "~" H 3650 2850 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 3650 2850 60  0001 C CNN "MPN"
F 5 "~" H 3650 2850 60  0001 C CNN "MFG"
	1    3650 2850
	0    -1   1    0   
$EndComp
Wire Wire Line
	4600 2850 3800 2850
Wire Wire Line
	3350 2850 3500 2850
Text Label 4550 2950 2    60   ~ 0
PETX0_2-
Text Label 4550 2850 2    60   ~ 0
PETX0_2+
$Comp
L C C71
U 1 1 594F3244
P 3800 2600
F 0 "C71" V 3850 2650 50  0000 L CNN
F 1 "0.1uf" V 3750 2650 50  0000 L CNN
F 2 "SMT:c_0201_least" H 3838 2450 50  0001 C CNN
F 3 "~" H 3800 2600 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 3800 2600 60  0001 C CNN "MPN"
F 5 "~" H 3800 2600 60  0001 C CNN "MFG"
	1    3800 2600
	0    1    -1   0   
$EndComp
$Comp
L C C73
U 1 1 58818F26
P 4050 2700
F 0 "C73" V 4100 2750 50  0000 L CNN
F 1 "0.1uf" V 4000 2750 50  0000 L CNN
F 2 "SMT:c_0201_least" H 4088 2550 50  0001 C CNN
F 3 "~" H 4050 2700 50  0000 C CNN
F 4 "cc0201krx5r5bb104" H 4050 2700 60  0001 C CNN "MPN"
F 5 "~" H 4050 2700 60  0001 C CNN "MFG"
	1    4050 2700
	0    1    -1   0   
$EndComp
Wire Wire Line
	4600 2700 4200 2700
Wire Wire Line
	3900 2700 3550 2700
Wire Wire Line
	3650 2600 3550 2600
Wire Wire Line
	4600 2600 3950 2600
Text GLabel 4450 3100 0    60   Input ~ 0
MGT0_RX_B+
Text GLabel 4450 3200 0    60   Input ~ 0
MGT0_RX_B-
Text GLabel 3550 2600 0    60   Input ~ 0
MGT_REFCLK0_B+
Text GLabel 3550 2700 0    60   Input ~ 0
MGT_REFCLK0_B-
Text GLabel 3350 2850 0    60   Input ~ 0
MGT0_TX_B+
Text GLabel 3350 2950 0    60   Input ~ 0
MGT0_TX_B-
Wire Wire Line
	4100 2950 4600 2950
Wire Wire Line
	3800 2950 3350 2950
Text Label 4250 2600 0    60   ~ 0
PECLK_2+
Text Label 4250 2700 0    60   ~ 0
PECLK_2-
Text GLabel 6200 2700 2    60   BiDi ~ 0
PE_CLKREQ_2_L
Text GLabel 6200 2800 2    60   BiDi ~ 0
PERST_2_L
Wire Wire Line
	6200 2700 5950 2700
Wire Wire Line
	6200 2800 5950 2800
Connection ~ 4600 3450
Text Notes 6450 7000 0    118  ~ 24
M.2 edge connections
Wire Wire Line
	5950 3450 5950 4250
Connection ~ 5950 3850
Text GLabel 6250 4400 2    60   Input ~ 0
W_DISABLE_1
Text GLabel 6250 4500 2    60   Input ~ 0
W_DISABLE_2
Wire Wire Line
	5950 4400 6250 4400
Wire Wire Line
	6250 4500 5950 4500
Text GLabel 6200 3200 2    60   Output ~ 0
M2_LED_1
Text GLabel 6200 3300 2    60   Output ~ 0
M2_LED_2
Wire Wire Line
	6200 3200 5950 3200
Wire Wire Line
	6200 3300 5950 3300
$EndSCHEMATC
