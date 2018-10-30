EESchema Schematic File Version 4
LIBS:uvm-cache
EELAYER 26 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 2 2
Title "uVM"
Date "2018-10-29"
Rev "A"
Comp ""
Comment1 "http://www.apache.org/licenses/LICENSE-2.0"
Comment2 "Licensed under the Apache License, Version 2.0"
Comment3 "Copyright 2018 Sam Stevens"
Comment4 ""
$EndDescr
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634CD
P 3650 2150
AR Path="/5FB634CD" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634CD" Ref="SW1"  Part="1" 
F 0 "SW1" H 3650 2435 50  0000 C CNN
F 1 "A15" H 3650 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 3650 2150 50  0001 C CNN
F 3 "" H 3650 2150 50  0001 C CNN
	1    3650 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634D4
P 4250 2150
AR Path="/5FB634D4" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634D4" Ref="SW3"  Part="1" 
F 0 "SW3" H 4250 2435 50  0000 C CNN
F 1 "A14" H 4250 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 4250 2150 50  0001 C CNN
F 3 "" H 4250 2150 50  0001 C CNN
	1    4250 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634DB
P 4850 2150
AR Path="/5FB634DB" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634DB" Ref="SW5"  Part="1" 
F 0 "SW5" H 4850 2435 50  0000 C CNN
F 1 "A13" H 4850 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 4850 2150 50  0001 C CNN
F 3 "" H 4850 2150 50  0001 C CNN
	1    4850 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634E2
P 5450 2150
AR Path="/5FB634E2" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634E2" Ref="SW7"  Part="1" 
F 0 "SW7" H 5450 2435 50  0000 C CNN
F 1 "A12" H 5450 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 5450 2150 50  0001 C CNN
F 3 "" H 5450 2150 50  0001 C CNN
	1    5450 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634E9
P 6050 2150
AR Path="/5FB634E9" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634E9" Ref="SW9"  Part="1" 
F 0 "SW9" H 6050 2435 50  0000 C CNN
F 1 "A11" H 6050 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 6050 2150 50  0001 C CNN
F 3 "" H 6050 2150 50  0001 C CNN
	1    6050 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634F0
P 6650 2150
AR Path="/5FB634F0" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634F0" Ref="SW10"  Part="1" 
F 0 "SW10" H 6650 2435 50  0000 C CNN
F 1 "A10" H 6650 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 6650 2150 50  0001 C CNN
F 3 "" H 6650 2150 50  0001 C CNN
	1    6650 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634F7
P 7250 2150
AR Path="/5FB634F7" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634F7" Ref="SW11"  Part="1" 
F 0 "SW11" H 7250 2435 50  0000 C CNN
F 1 "A11" H 7250 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 7250 2150 50  0001 C CNN
F 3 "" H 7250 2150 50  0001 C CNN
	1    7250 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB634FE
P 7850 2150
AR Path="/5FB634FE" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB634FE" Ref="SW12"  Part="1" 
F 0 "SW12" H 7850 2435 50  0000 C CNN
F 1 "A9" H 7850 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 7850 2150 50  0001 C CNN
F 3 "" H 7850 2150 50  0001 C CNN
	1    7850 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network08 RN?
U 1 1 5FB63505
P 4650 3900
AR Path="/5FB63505" Ref="RN?"  Part="1" 
AR Path="/5FA8FA07/5FB63505" Ref="RN17"  Part="1" 
F 0 "RN17" H 4050 3750 50  0000 C CNN
F 1 "10K" H 4050 3850 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 5125 3900 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 4650 3900 50  0001 C CNN
	1    4650 3900
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5FB63527
P 3400 1450
AR Path="/5FB63527" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB63527" Ref="#PWR064"  Part="1" 
F 0 "#PWR064" H 3400 1300 50  0001 C CNN
F 1 "+5V" H 3415 1623 50  0000 C CNN
F 2 "" H 3400 1450 50  0001 C CNN
F 3 "" H 3400 1450 50  0001 C CNN
	1    3400 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 2250 3850 2250
Wire Wire Line
	4450 2250 4500 2250
Wire Wire Line
	5100 2250 5050 2250
Wire Wire Line
	5700 2250 5650 2250
Wire Wire Line
	6300 2250 6250 2250
Wire Wire Line
	6850 2250 6900 2250
Wire Wire Line
	7500 2250 7450 2250
Wire Wire Line
	8050 2250 8100 2250
$Comp
L uvm-rescue:74HC165-uvm U?
U 1 1 5FB6353D
P 3550 3150
AR Path="/5FB6353D" Ref="U?"  Part="1" 
AR Path="/5FA8FA07/5FB6353D" Ref="U20"  Part="1" 
F 0 "U20" H 3200 3750 50  0000 C CNN
F 1 "74HC165" H 3800 3750 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 3600 3550 50  0001 C CNN
F 3 "" H 3600 3550 50  0001 C CNN
	1    3550 3150
	1    0    0    1   
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5FB63544
P 3550 3900
AR Path="/5FB63544" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB63544" Ref="#PWR068"  Part="1" 
F 0 "#PWR068" H 3550 3750 50  0001 C CNN
F 1 "+5V" H 3565 4073 50  0000 C CNN
F 2 "" H 3550 3900 50  0001 C CNN
F 3 "" H 3550 3900 50  0001 C CNN
	1    3550 3900
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FB6354A
P 3500 2350
AR Path="/5FB6354A" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB6354A" Ref="#PWR067"  Part="1" 
F 0 "#PWR067" H 3500 2100 50  0001 C CNN
F 1 "GND" V 3505 2222 50  0000 R CNN
F 2 "" H 3500 2350 50  0001 C CNN
F 3 "" H 3500 2350 50  0001 C CNN
	1    3500 2350
	0    1    1    0   
$EndComp
Wire Wire Line
	3500 2350 3550 2350
Wire Wire Line
	3550 2350 3550 2400
Wire Wire Line
	4150 2700 4550 2700
Text Label 4550 2700 2    50   ~ 0
ADDR_H_IN
NoConn ~ 2950 2700
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB63577
P 9500 2150
AR Path="/5FB63577" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB63577" Ref="SW13"  Part="1" 
F 0 "SW13" H 9500 2435 50  0000 C CNN
F 1 "A8" H 9500 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 9500 2150 50  0001 C CNN
F 3 "" H 9500 2150 50  0001 C CNN
	1    9500 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB6357E
P 10100 2150
AR Path="/5FB6357E" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB6357E" Ref="SW14"  Part="1" 
F 0 "SW14" H 10100 2435 50  0000 C CNN
F 1 "A7" H 10100 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 10100 2150 50  0001 C CNN
F 3 "" H 10100 2150 50  0001 C CNN
	1    10100 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB63585
P 10700 2150
AR Path="/5FB63585" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB63585" Ref="SW15"  Part="1" 
F 0 "SW15" H 10700 2435 50  0000 C CNN
F 1 "A6" H 10700 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 10700 2150 50  0001 C CNN
F 3 "" H 10700 2150 50  0001 C CNN
	1    10700 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB6358C
P 11300 2150
AR Path="/5FB6358C" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB6358C" Ref="SW16"  Part="1" 
F 0 "SW16" H 11300 2435 50  0000 C CNN
F 1 "A5" H 11300 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 11300 2150 50  0001 C CNN
F 3 "" H 11300 2150 50  0001 C CNN
	1    11300 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB63593
P 11900 2150
AR Path="/5FB63593" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB63593" Ref="SW17"  Part="1" 
F 0 "SW17" H 11900 2435 50  0000 C CNN
F 1 "A4" H 11900 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 11900 2150 50  0001 C CNN
F 3 "" H 11900 2150 50  0001 C CNN
	1    11900 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB6359A
P 12500 2150
AR Path="/5FB6359A" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB6359A" Ref="SW18"  Part="1" 
F 0 "SW18" H 12500 2435 50  0000 C CNN
F 1 "A3" H 12500 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 12500 2150 50  0001 C CNN
F 3 "" H 12500 2150 50  0001 C CNN
	1    12500 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB635A1
P 13100 2150
AR Path="/5FB635A1" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB635A1" Ref="SW19"  Part="1" 
F 0 "SW19" H 13100 2435 50  0000 C CNN
F 1 "A2" H 13100 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 13100 2150 50  0001 C CNN
F 3 "" H 13100 2150 50  0001 C CNN
	1    13100 2150
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT SW?
U 1 1 5FB635A8
P 13700 2150
AR Path="/5FB635A8" Ref="SW?"  Part="1" 
AR Path="/5FA8FA07/5FB635A8" Ref="SW20"  Part="1" 
F 0 "SW20" H 13700 2435 50  0000 C CNN
F 1 "A1" H 13700 2344 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 13700 2150 50  0001 C CNN
F 3 "" H 13700 2150 50  0001 C CNN
	1    13700 2150
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Network08 RN?
U 1 1 5FB635AF
P 10550 3900
AR Path="/5FB635AF" Ref="RN?"  Part="1" 
AR Path="/5FA8FA07/5FB635AF" Ref="RN19"  Part="1" 
F 0 "RN19" V 11067 3900 50  0000 C CNN
F 1 "10K" V 10976 3900 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 11025 3900 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 10550 3900 50  0001 C CNN
	1    10550 3900
	-1   0    0    1   
$EndComp
$Comp
L power:+5V #PWR?
U 1 1 5FB635D1
P 9250 1450
AR Path="/5FB635D1" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB635D1" Ref="#PWR073"  Part="1" 
F 0 "#PWR073" H 9250 1300 50  0001 C CNN
F 1 "+5V" H 9265 1623 50  0000 C CNN
F 2 "" H 9250 1450 50  0001 C CNN
F 3 "" H 9250 1450 50  0001 C CNN
	1    9250 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	9750 2250 9700 2250
Wire Wire Line
	10300 2250 10350 2250
Wire Wire Line
	10950 2250 10900 2250
Wire Wire Line
	11550 2250 11500 2250
Wire Wire Line
	12150 2250 12100 2250
Wire Wire Line
	12700 2250 12750 2250
Wire Wire Line
	13350 2250 13300 2250
Wire Wire Line
	13900 2250 13950 2250
$Comp
L power:+5V #PWR?
U 1 1 5FB635EE
P 9400 3900
AR Path="/5FB635EE" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB635EE" Ref="#PWR077"  Part="1" 
F 0 "#PWR077" H 9400 3750 50  0001 C CNN
F 1 "+5V" H 9415 4073 50  0000 C CNN
F 2 "" H 9400 3900 50  0001 C CNN
F 3 "" H 9400 3900 50  0001 C CNN
	1    9400 3900
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR?
U 1 1 5FB635F4
P 9350 2350
AR Path="/5FB635F4" Ref="#PWR?"  Part="1" 
AR Path="/5FA8FA07/5FB635F4" Ref="#PWR076"  Part="1" 
F 0 "#PWR076" H 9350 2100 50  0001 C CNN
F 1 "GND" V 9355 2222 50  0000 R CNN
F 2 "" H 9350 2350 50  0001 C CNN
F 3 "" H 9350 2350 50  0001 C CNN
	1    9350 2350
	0    1    1    0   
$EndComp
Wire Wire Line
	9350 2350 9400 2350
Wire Wire Line
	9400 2350 9400 2400
Wire Wire Line
	8300 2800 8800 2800
Text Label 8300 2800 0    50   ~ 0
ADDR_H_IN
NoConn ~ 8800 2700
Text HLabel 2950 2800 0    50   Output ~ 0
INP_SER
Text HLabel 2950 3100 0    50   Input ~ 0
INP_ADDRINH
Text HLabel 2950 3200 0    50   Input ~ 0
INP_CLK
Text HLabel 2950 3300 0    50   Input ~ 0
INP_LD
Text HLabel 8800 3100 0    50   Input ~ 0
INP_ADDRINH
Text HLabel 8800 3200 0    50   Input ~ 0
INP_CLK
Text HLabel 8800 3300 0    50   Input ~ 0
INP_LD
$Comp
L Switch:SW_SPDT_MSM SW2
U 1 1 6047FF9B
P 3900 5250
F 0 "SW2" H 3900 5535 50  0000 C CNN
F 1 "Run / Step" H 3900 5450 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 3900 5250 50  0001 C CNN
F 3 "" H 3900 5250 50  0001 C CNN
	1    3900 5250
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT_MSM SW6
U 1 1 60480056
P 5300 5250
F 0 "SW6" H 5300 5535 50  0000 C CNN
F 1 "Examine / Next" H 5300 5444 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 5300 5250 50  0001 C CNN
F 3 "" H 5300 5250 50  0001 C CNN
	1    5300 5250
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT_MSM SW4
U 1 1 604849ED
P 4600 5250
F 0 "SW4" H 4600 5535 50  0000 C CNN
F 1 "Reset / Clear" H 4600 5444 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 4600 5250 50  0001 C CNN
F 3 "" H 4600 5250 50  0001 C CNN
	1    4600 5250
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_SPDT_MSM SW8
U 1 1 60484AF3
P 6000 5250
F 0 "SW8" H 6000 5535 50  0000 C CNN
F 1 "Deposit / Next" H 6000 5444 50  0000 C CNN
F 2 "uvm:TOGGLE_SPDT_ON_ON_2MS" H 6000 5250 50  0001 C CNN
F 3 "" H 6000 5250 50  0001 C CNN
	1    6000 5250
	1    0    0    -1  
$EndComp
$Comp
L uvm-rescue:74HC165-uvm U?
U 1 1 604899DA
P 3300 6250
AR Path="/604899DA" Ref="U?"  Part="1" 
AR Path="/5FA8FA07/604899DA" Ref="U22"  Part="1" 
F 0 "U22" H 2950 6850 50  0000 C CNN
F 1 "74HC165" H 3550 6850 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 3350 6650 50  0001 C CNN
F 3 "" H 3350 6650 50  0001 C CNN
	1    3300 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3900 5800 4050 5800
Wire Wire Line
	4150 5800 4150 5350
Wire Wire Line
	4150 5350 4100 5350
Wire Wire Line
	4100 5150 4200 5150
Wire Wire Line
	4200 5150 4200 5900
Wire Wire Line
	4200 5900 4150 5900
Wire Wire Line
	3900 6000 4250 6000
Wire Wire Line
	4850 6000 4850 5350
Wire Wire Line
	4850 5350 4800 5350
Wire Wire Line
	3900 6100 4350 6100
Wire Wire Line
	4900 6100 4900 5150
Wire Wire Line
	4900 5150 4800 5150
Wire Wire Line
	3900 6200 4450 6200
Wire Wire Line
	5550 6200 5550 5350
Wire Wire Line
	5550 5350 5500 5350
Wire Wire Line
	5600 6300 5600 5150
Wire Wire Line
	5600 5150 5500 5150
Wire Wire Line
	3900 6300 4550 6300
Wire Wire Line
	3900 6400 4650 6400
Wire Wire Line
	6250 6400 6250 5350
Wire Wire Line
	6250 5350 6200 5350
Wire Wire Line
	3900 6500 4750 6500
Wire Wire Line
	6300 6500 6300 5150
Wire Wire Line
	6300 5150 6200 5150
$Comp
L Device:R_Network08 RN?
U 1 1 60484FB0
P 4450 6950
AR Path="/60484FB0" Ref="RN?"  Part="1" 
AR Path="/5FA8FA07/60484FB0" Ref="RN18"  Part="1" 
F 0 "RN18" H 4750 7150 50  0000 C CNN
F 1 "10K" H 4750 7250 50  0000 C CNN
F 2 "Resistor_THT:R_Array_SIP9" V 4925 6950 50  0001 C CNN
F 3 "http://www.vishay.com/docs/31509/csc.pdf" H 4450 6950 50  0001 C CNN
	1    4450 6950
	1    0    0    1   
$EndComp
Wire Wire Line
	4050 6750 4050 5800
Connection ~ 4050 5800
Wire Wire Line
	4050 5800 4150 5800
Wire Wire Line
	4150 6750 4150 5900
Connection ~ 4150 5900
Wire Wire Line
	4150 5900 3900 5900
Wire Wire Line
	4250 6750 4250 6000
Connection ~ 4250 6000
Wire Wire Line
	4250 6000 4850 6000
Wire Wire Line
	4350 6750 4350 6100
Wire Wire Line
	4550 6750 4550 6300
Connection ~ 4550 6300
Wire Wire Line
	4550 6300 5600 6300
Wire Wire Line
	4350 6100 4900 6100
Connection ~ 4350 6100
Wire Wire Line
	4450 6750 4450 6200
Connection ~ 4450 6200
Wire Wire Line
	4450 6200 5550 6200
Wire Wire Line
	4650 6750 4650 6400
Connection ~ 4650 6400
Wire Wire Line
	4650 6400 6250 6400
Wire Wire Line
	4750 6750 4750 6500
Connection ~ 4750 6500
Wire Wire Line
	4750 6500 6300 6500
Text HLabel 2700 6600 0    50   Output ~ 0
INP_SER
Text HLabel 2700 6300 0    50   Input ~ 0
INP_SWINH
Text HLabel 2700 6200 0    50   Input ~ 0
INP_CLK
Text HLabel 2700 6100 0    50   Input ~ 0
INP_LD
$Comp
L power:GND #PWR070
U 1 1 60601199
P 4050 7150
F 0 "#PWR070" H 4050 6900 50  0001 C CNN
F 1 "GND" H 4055 6977 50  0000 C CNN
F 2 "" H 4050 7150 50  0001 C CNN
F 3 "" H 4050 7150 50  0001 C CNN
	1    4050 7150
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR069
U 1 1 606011F6
P 3600 4750
F 0 "#PWR069" H 3600 4600 50  0001 C CNN
F 1 "+5V" H 3615 4923 50  0000 C CNN
F 2 "" H 3600 4750 50  0001 C CNN
F 3 "" H 3600 4750 50  0001 C CNN
	1    3600 4750
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 5250 3600 4800
Wire Wire Line
	3600 5250 3700 5250
Wire Wire Line
	3600 4800 4300 4800
Wire Wire Line
	4300 4800 4300 5250
Wire Wire Line
	4300 5250 4400 5250
Connection ~ 3600 4800
Wire Wire Line
	3600 4800 3600 4750
Wire Wire Line
	4300 4800 5000 4800
Wire Wire Line
	5000 4800 5000 5250
Wire Wire Line
	5000 5250 5100 5250
Wire Wire Line
	5000 4800 5700 4800
Wire Wire Line
	5700 4800 5700 5250
Wire Wire Line
	5700 5250 5800 5250
Connection ~ 5000 4800
NoConn ~ 2700 6700
NoConn ~ 3900 6700
$Comp
L power:+5V #PWR065
U 1 1 606446D4
P 3300 5500
F 0 "#PWR065" H 3300 5350 50  0001 C CNN
F 1 "+5V" H 3315 5673 50  0000 C CNN
F 2 "" H 3300 5500 50  0001 C CNN
F 3 "" H 3300 5500 50  0001 C CNN
	1    3300 5500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR066
U 1 1 60644713
P 3300 7000
F 0 "#PWR066" H 3300 6750 50  0001 C CNN
F 1 "GND" H 3305 6827 50  0000 C CNN
F 2 "" H 3300 7000 50  0001 C CNN
F 3 "" H 3300 7000 50  0001 C CNN
	1    3300 7000
	1    0    0    -1  
$EndComp
$Comp
L Interface_Expansion:MCP23017_SP U23
U 1 1 61A5AB30
P 8650 6700
F 0 "U23" H 8200 5700 50  0000 C CNN
F 1 "MCP23017_SP" H 8950 5700 50  0000 C CNN
F 2 "Package_DIP:DIP-28_W7.62mm" H 8850 5700 50  0001 L CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/20001952C.pdf" H 8850 5600 50  0001 L CNN
	1    8650 6700
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR074
U 1 1 61A8DC85
P 8650 5600
F 0 "#PWR074" H 8650 5450 50  0001 C CNN
F 1 "+5V" H 8665 5773 50  0000 C CNN
F 2 "" H 8650 5600 50  0001 C CNN
F 3 "" H 8650 5600 50  0001 C CNN
	1    8650 5600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR075
U 1 1 61A8DCE4
P 8650 7800
F 0 "#PWR075" H 8650 7550 50  0001 C CNN
F 1 "GND" H 8655 7627 50  0000 C CNN
F 2 "" H 8650 7800 50  0001 C CNN
F 3 "" H 8650 7800 50  0001 C CNN
	1    8650 7800
	1    0    0    -1  
$EndComp
Text HLabel 7950 6800 0    50   Input ~ 0
RESET
Text HLabel 7150 6600 0    50   Output ~ 0
IO_INT
Text HLabel 7950 5900 0    50   BiDi ~ 0
SDA
Text HLabel 7950 6000 0    50   Input ~ 0
SCK
$Comp
L Connector_Generic:Conn_02x02_Top_Bottom J4
U 1 1 61CB7B6F
P 8300 4950
F 0 "J4" V 8400 5100 50  0000 R CNN
F 1 "Power" V 8300 5250 50  0000 R CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x02_P2.54mm_Vertical" H 8300 4950 50  0001 C CNN
F 3 "~" H 8300 4950 50  0001 C CNN
	1    8300 4950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8300 4650 8300 4550
Wire Wire Line
	8300 5150 8300 5250
Wire Wire Line
	8300 5250 8400 5250
$Comp
L Device:C C7
U 1 1 61D14A96
P 9050 4900
F 0 "C7" H 9165 4946 50  0000 L CNN
F 1 "100nF" H 9165 4855 50  0000 L CNN
F 2 "LED_THT:LED_D5.0mm" H 9088 4750 50  0001 C CNN
F 3 "~" H 9050 4900 50  0001 C CNN
	1    9050 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	9050 4750 9050 4550
Wire Wire Line
	8300 4550 8400 4550
Wire Wire Line
	9050 5050 9050 5250
Wire Wire Line
	8400 5150 8400 5250
Wire Wire Line
	8400 4650 8400 4550
$Comp
L power:GND #PWR072
U 1 1 61D8EE4B
P 7750 7800
F 0 "#PWR072" H 7750 7550 50  0001 C CNN
F 1 "GND" H 7755 7627 50  0000 C CNN
F 2 "" H 7750 7800 50  0001 C CNN
F 3 "" H 7750 7800 50  0001 C CNN
	1    7750 7800
	1    0    0    -1  
$EndComp
Wire Wire Line
	7750 7800 7750 7500
Wire Wire Line
	7750 7500 7950 7500
Wire Wire Line
	7950 7400 7750 7400
Wire Wire Line
	7750 7400 7750 7500
Connection ~ 7750 7500
Wire Wire Line
	7950 7300 7750 7300
Wire Wire Line
	7750 7300 7750 7400
Connection ~ 7750 7400
$Comp
L Device:R R2
U 1 1 61DCD017
P 7450 6250
F 0 "R2" H 7520 6296 50  0000 L CNN
F 1 "10K" H 7520 6205 50  0000 L CNN
F 2 "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 7380 6250 50  0001 C CNN
F 3 "~" H 7450 6250 50  0001 C CNN
	1    7450 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7150 6600 7450 6600
Wire Wire Line
	7450 6400 7450 6600
Connection ~ 7450 6600
Wire Wire Line
	7450 6600 7800 6600
Wire Wire Line
	7950 6500 7800 6500
Wire Wire Line
	7800 6500 7800 6600
Connection ~ 7800 6600
Wire Wire Line
	7800 6600 7950 6600
$Comp
L power:+5V #PWR071
U 1 1 61DFE22B
P 7450 6100
F 0 "#PWR071" H 7450 5950 50  0001 C CNN
F 1 "+5V" H 7465 6273 50  0000 C CNN
F 2 "" H 7450 6100 50  0001 C CNN
F 3 "" H 7450 6100 50  0001 C CNN
	1    7450 6100
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Top_Bottom J5
U 1 1 61DFF639
P 10500 6650
F 0 "J5" V 10500 7050 50  0000 C CNN
F 1 "PortAB" V 10600 7150 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_2x08_P2.54mm_Vertical" H 10500 6650 50  0001 C CNN
F 3 "~" H 10500 6650 50  0001 C CNN
	1    10500 6650
	0    1    1    0   
$EndComp
Wire Notes Line
	2150 1000 14150 1000
Wire Notes Line
	14150 1000 14150 4250
Wire Notes Line
	14150 4250 2150 4250
Wire Notes Line
	2150 4250 2150 1000
Wire Notes Line
	2150 4350 6550 4350
Wire Notes Line
	6550 4350 6550 7450
Wire Notes Line
	6550 7450 2150 7450
Wire Notes Line
	2150 4350 2150 7450
Wire Notes Line
	6650 4350 11150 4350
Wire Notes Line
	11150 4350 11150 8300
Wire Notes Line
	11150 8300 6650 8300
Wire Notes Line
	6650 4350 6650 8300
Connection ~ 4300 4800
Text Notes 2150 4250 0    50   ~ 0
Address/Data Input
Text Notes 2150 7450 0    50   ~ 0
Action Input
Text Notes 6650 8300 0    50   ~ 0
PortA / PortB
$Comp
L uvm-rescue:74HC165-uvm U?
U 1 1 5FB635E7
P 9400 3150
AR Path="/5FB635E7" Ref="U?"  Part="1" 
AR Path="/5FA8FA07/5FB635E7" Ref="U21"  Part="1" 
F 0 "U21" H 9050 3750 50  0000 C CNN
F 1 "74HC165" H 9650 3750 50  0000 C CNN
F 2 "Package_DIP:DIP-16_W7.62mm" H 9450 3550 50  0001 C CNN
F 3 "" H 9450 3550 50  0001 C CNN
	1    9400 3150
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR080
U 1 1 620A012F
P 10000 2700
F 0 "#PWR080" H 10000 2450 50  0001 C CNN
F 1 "GND" V 10005 2572 50  0000 R CNN
F 2 "" H 10000 2700 50  0001 C CNN
F 3 "" H 10000 2700 50  0001 C CNN
	1    10000 2700
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3450 2150 3400 2150
Wire Wire Line
	3400 2150 3400 1750
Wire Wire Line
	4050 2150 4050 1750
Wire Wire Line
	4050 1750 3400 1750
Connection ~ 3400 1750
Wire Wire Line
	3400 1750 3400 1450
Wire Wire Line
	4650 2150 4650 1750
Wire Wire Line
	4650 1750 4050 1750
Connection ~ 4050 1750
Wire Wire Line
	5250 2150 5250 1750
Wire Wire Line
	5250 1750 4650 1750
Connection ~ 4650 1750
Wire Wire Line
	5850 2150 5850 1750
Wire Wire Line
	5850 1750 5250 1750
Connection ~ 5250 1750
Wire Wire Line
	6450 2150 6450 1750
Wire Wire Line
	6450 1750 5850 1750
Connection ~ 5850 1750
Wire Wire Line
	7050 2150 7050 1750
Wire Wire Line
	7050 1750 6450 1750
Connection ~ 6450 1750
Wire Wire Line
	7650 2150 7650 1750
Wire Wire Line
	7650 1750 7050 1750
Connection ~ 7050 1750
$Comp
L power:GND #PWR084
U 1 1 5CAC8218
P 5150 4100
F 0 "#PWR084" H 5150 3850 50  0001 C CNN
F 1 "GND" V 5155 3972 50  0000 R CNN
F 2 "" H 5150 4100 50  0001 C CNN
F 3 "" H 5150 4100 50  0001 C CNN
	1    5150 4100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4800 2900 4800 2500
Wire Wire Line
	4800 2500 3900 2500
Wire Wire Line
	3900 2500 3900 2250
Wire Wire Line
	4150 2900 4350 2900
Wire Wire Line
	4850 3000 4850 2450
Wire Wire Line
	4850 2450 4500 2450
Wire Wire Line
	4500 2450 4500 2250
Wire Wire Line
	4900 3100 4900 2500
Wire Wire Line
	4900 2500 5100 2500
Wire Wire Line
	5100 2500 5100 2250
Wire Wire Line
	4950 2550 5700 2550
Wire Wire Line
	5700 2550 5700 2250
Wire Wire Line
	4950 3200 4950 2550
Wire Wire Line
	4150 3300 4750 3300
Wire Wire Line
	5000 3300 5000 2600
Wire Wire Line
	5000 2600 6300 2600
Wire Wire Line
	6300 2600 6300 2250
Wire Wire Line
	4150 3400 4850 3400
Wire Wire Line
	5050 3400 5050 2650
Wire Wire Line
	5050 2650 6900 2650
Wire Wire Line
	6900 2650 6900 2250
Wire Wire Line
	4150 3500 4950 3500
Wire Wire Line
	5100 3500 5100 2700
Wire Wire Line
	5100 2700 7500 2700
Wire Wire Line
	7500 2700 7500 2250
Wire Wire Line
	4150 3600 5050 3600
Wire Wire Line
	5150 3600 5150 2750
Wire Wire Line
	5150 2750 8100 2750
Wire Wire Line
	8100 2750 8100 2250
Wire Wire Line
	5150 4100 5050 4100
Wire Wire Line
	4350 3700 4350 2900
Wire Wire Line
	4450 3700 4450 3000
Connection ~ 4450 3000
Wire Wire Line
	4550 3700 4550 3100
Connection ~ 4550 3100
Wire Wire Line
	4650 3700 4650 3200
Connection ~ 4650 3200
Wire Wire Line
	4650 3200 4950 3200
Wire Wire Line
	4750 3700 4750 3300
Connection ~ 4750 3300
Wire Wire Line
	4750 3300 5000 3300
Connection ~ 4850 3400
Wire Wire Line
	4850 3400 5050 3400
Wire Wire Line
	4850 3400 4850 3700
Wire Wire Line
	4950 3700 4950 3500
Connection ~ 4950 3500
Wire Wire Line
	4950 3500 5100 3500
Wire Wire Line
	5050 3700 5050 3600
Connection ~ 5050 3600
Wire Wire Line
	5050 3600 5150 3600
Connection ~ 4350 2900
Wire Wire Line
	4550 3100 4900 3100
Wire Wire Line
	4150 3200 4650 3200
Wire Wire Line
	4450 3000 4850 3000
Wire Wire Line
	4150 3100 4550 3100
Wire Wire Line
	4350 2900 4800 2900
Wire Wire Line
	4150 3000 4450 3000
Wire Wire Line
	9250 2150 9300 2150
Wire Wire Line
	9250 1450 9250 1750
Wire Wire Line
	9900 2150 9900 1750
Wire Wire Line
	9900 1750 9250 1750
Connection ~ 9250 1750
Wire Wire Line
	9250 1750 9250 2150
Wire Wire Line
	9900 1750 10500 1750
Wire Wire Line
	10500 1750 10500 2150
Connection ~ 9900 1750
Wire Wire Line
	10500 1750 11100 1750
Wire Wire Line
	11100 1750 11100 2150
Connection ~ 10500 1750
Wire Wire Line
	11100 1750 11700 1750
Wire Wire Line
	11700 1750 11700 2150
Connection ~ 11100 1750
Wire Wire Line
	11700 1750 12300 1750
Wire Wire Line
	12300 1750 12300 2150
Connection ~ 11700 1750
Wire Wire Line
	12300 1750 12900 1750
Wire Wire Line
	12900 1750 12900 2150
Connection ~ 12300 1750
Wire Wire Line
	12900 1750 13500 1750
Wire Wire Line
	13500 1750 13500 2150
Connection ~ 12900 1750
Wire Wire Line
	9750 2500 9750 2250
Wire Wire Line
	10000 3600 10950 3600
Wire Wire Line
	11000 3500 10850 3500
Wire Wire Line
	10950 3400 10750 3400
Wire Wire Line
	10900 3300 10650 3300
Wire Wire Line
	10850 3200 10550 3200
Wire Wire Line
	10700 2900 10250 2900
Wire Wire Line
	10700 2900 10700 2500
Wire Wire Line
	10700 2500 9750 2500
Wire Wire Line
	10750 3000 10750 2450
Wire Wire Line
	10750 2450 10350 2450
Wire Wire Line
	10350 2450 10350 2250
Wire Wire Line
	10800 3100 10800 2500
Wire Wire Line
	10800 2500 10950 2500
Wire Wire Line
	10950 2500 10950 2250
Wire Wire Line
	10850 3200 10850 2550
Wire Wire Line
	10850 2550 11550 2550
Wire Wire Line
	11550 2550 11550 2250
Wire Wire Line
	10900 3300 10900 2600
Wire Wire Line
	10900 2600 12150 2600
Wire Wire Line
	12150 2600 12150 2250
Wire Wire Line
	10950 3400 10950 2650
Wire Wire Line
	10950 2650 12750 2650
Wire Wire Line
	12750 2650 12750 2250
Wire Wire Line
	11000 3500 11000 2700
Wire Wire Line
	11000 2700 13350 2700
Wire Wire Line
	13350 2700 13350 2250
Wire Wire Line
	13950 2250 13950 2750
Wire Wire Line
	13950 2750 11050 2750
Wire Wire Line
	11050 2750 11050 3600
Wire Wire Line
	10950 3700 10950 3600
Connection ~ 10950 3600
Wire Wire Line
	10950 3600 11050 3600
Wire Wire Line
	10850 3700 10850 3500
Connection ~ 10850 3500
Wire Wire Line
	10850 3500 10000 3500
Wire Wire Line
	10750 3700 10750 3400
Connection ~ 10750 3400
Wire Wire Line
	10750 3400 10000 3400
Wire Wire Line
	10650 3700 10650 3300
Connection ~ 10650 3300
Wire Wire Line
	10650 3300 10000 3300
Wire Wire Line
	10550 3700 10550 3200
Connection ~ 10550 3200
Wire Wire Line
	10550 3200 10000 3200
Wire Wire Line
	10450 3700 10450 3100
Wire Wire Line
	10000 3100 10450 3100
Connection ~ 10450 3100
Wire Wire Line
	10450 3100 10800 3100
Wire Wire Line
	10350 3700 10350 3000
Wire Wire Line
	10000 3000 10350 3000
Connection ~ 10350 3000
Wire Wire Line
	10350 3000 10750 3000
Wire Wire Line
	10250 3700 10250 2900
Connection ~ 10250 2900
Wire Wire Line
	10250 2900 10000 2900
$Comp
L power:GND #PWR085
U 1 1 5D05FE53
P 11050 4100
F 0 "#PWR085" H 11050 3850 50  0001 C CNN
F 1 "GND" V 11055 3972 50  0000 R CNN
F 2 "" H 11050 4100 50  0001 C CNN
F 3 "" H 11050 4100 50  0001 C CNN
	1    11050 4100
	0    -1   -1   0   
$EndComp
Wire Wire Line
	11050 4100 10950 4100
Wire Wire Line
	9350 6600 9950 6600
Wire Wire Line
	9950 6600 9950 6450
Wire Wire Line
	9950 6450 10100 6450
Wire Wire Line
	9350 6500 9900 6500
Wire Wire Line
	9900 6500 9900 6400
Wire Wire Line
	9900 6400 10200 6400
Wire Wire Line
	10200 6400 10200 6450
Wire Wire Line
	9350 6400 9850 6400
Wire Wire Line
	9850 6400 9850 6350
Wire Wire Line
	9850 6350 10300 6350
Wire Wire Line
	10300 6350 10300 6450
Wire Wire Line
	9350 6300 10400 6300
Wire Wire Line
	10400 6300 10400 6450
Wire Wire Line
	9350 6200 10500 6200
Wire Wire Line
	10500 6200 10500 6450
Wire Wire Line
	9350 6100 10600 6100
Wire Wire Line
	10600 6100 10600 6450
Wire Wire Line
	9350 6000 10700 6000
Wire Wire Line
	10700 6000 10700 6450
Wire Wire Line
	9350 5900 10800 5900
Wire Wire Line
	10800 5900 10800 6450
Wire Wire Line
	9350 6800 9950 6800
Wire Wire Line
	9950 6800 9950 6950
Wire Wire Line
	9950 6950 10100 6950
Wire Wire Line
	9350 6900 9900 6900
Wire Wire Line
	9900 6900 9900 7000
Wire Wire Line
	9900 7000 10200 7000
Wire Wire Line
	10200 7000 10200 6950
Wire Wire Line
	9350 7000 9850 7000
Wire Wire Line
	9850 7000 9850 7050
Wire Wire Line
	9850 7050 10300 7050
Wire Wire Line
	10300 7050 10300 6950
Wire Wire Line
	9350 7100 10400 7100
Wire Wire Line
	10400 7100 10400 6950
Wire Wire Line
	9350 7200 10500 7200
Wire Wire Line
	10500 7200 10500 6950
Wire Wire Line
	9350 7300 10600 7300
Wire Wire Line
	10600 7300 10600 6950
Wire Wire Line
	9350 7400 10700 7400
Wire Wire Line
	10700 7400 10700 6950
Wire Wire Line
	9350 7500 10800 7500
Wire Wire Line
	10800 7500 10800 6950
NoConn ~ 13900 2050
NoConn ~ 13300 2050
NoConn ~ 12700 2050
NoConn ~ 12100 2050
NoConn ~ 11500 2050
NoConn ~ 10900 2050
NoConn ~ 10300 2050
NoConn ~ 9700 2050
NoConn ~ 8050 2050
NoConn ~ 7450 2050
NoConn ~ 6850 2050
NoConn ~ 6250 2050
NoConn ~ 5650 2050
NoConn ~ 5050 2050
NoConn ~ 4450 2050
NoConn ~ 3850 2050
Connection ~ 9050 4550
Wire Wire Line
	9050 4550 9200 4550
Connection ~ 9050 5250
Wire Wire Line
	9050 5250 9200 5250
Wire Wire Line
	8400 4550 8600 4550
Connection ~ 8400 4550
Wire Wire Line
	8400 5250 8600 5250
Connection ~ 8400 5250
$Comp
L Device:CP C8
U 1 1 5DD28219
P 8600 4900
F 0 "C8" H 8718 4946 50  0000 L CNN
F 1 "10uF" H 8718 4855 50  0000 L CNN
F 2 "LED_THT:LED_D5.0mm" H 8638 4750 50  0001 C CNN
F 3 "~" H 8600 4900 50  0001 C CNN
	1    8600 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	8600 5050 8600 5250
Connection ~ 8600 5250
Wire Wire Line
	8600 5250 9050 5250
Wire Wire Line
	8600 4750 8600 4550
Connection ~ 8600 4550
Wire Wire Line
	8600 4550 9050 4550
$EndSCHEMATC
