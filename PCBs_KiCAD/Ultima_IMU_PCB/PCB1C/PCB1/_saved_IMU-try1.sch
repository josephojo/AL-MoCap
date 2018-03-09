EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
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
LIBS:imu
LIBS:imu2
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L IMU U1
U 1 1 5A8B280C
P 3600 4500
F 0 "U1" H 3600 4400 60  0000 C CNN
F 1 "IMU" H 3600 4600 60  0000 C CNN
F 2 "" H 3700 4400 60  0001 C CNN
F 3 "" H 3700 4400 60  0001 C CNN
	1    3600 4500
	1    0    0    -1  
$EndComp
$Comp
L Conn_01x06 J1
U 1 1 5A8B2A41
P 4200 4550
F 0 "J1" H 4200 4850 50  0000 C CNN
F 1 "Conn_01x06" H 4200 4150 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06_Pitch1.00mm" H 4200 4550 50  0001 C CNN
F 3 "" H 4200 4550 50  0001 C CNN
	1    4200 4550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3150 4400 3150 4100
Wire Wire Line
	3150 4100 3900 4100
Wire Wire Line
	3900 4100 3900 4350
Wire Wire Line
	3900 4350 4000 4350
Wire Wire Line
	3250 4400 3250 4200
Wire Wire Line
	3250 4200 3800 4200
Wire Wire Line
	3800 4200 3800 4450
Wire Wire Line
	3800 4450 4000 4450
Wire Wire Line
	3300 4400 3300 4300
Wire Wire Line
	3700 4550 4000 4550
Wire Wire Line
	3350 4400 3600 4400
Wire Wire Line
	3600 4400 3600 4650
Wire Wire Line
	3600 4650 4000 4650
Wire Wire Line
	3300 4300 3700 4300
Wire Wire Line
	3700 4300 3700 4550
Wire Wire Line
	3500 4550 3550 4550
Wire Wire Line
	3550 4550 3550 4750
Wire Wire Line
	3550 4750 4000 4750
Wire Wire Line
	3500 4650 3500 4850
Wire Wire Line
	3500 4850 4000 4850
$EndSCHEMATC
