; reset vector
:origin 0
            JMP :start

; Program: Hello World
; Prints "Hello World\n"

; Registers
; R1 = 1
; R2 = current character in string
; RXA(R4,R5) = address of string
; R6 = device status
; R7 = device scrach
; RXF(RE,RF) = return address for print loop

$SERIAL_STATUS = 0x10
$SERIAL_DATA = 0x11
$SERIAL_TXREADY = 0x04

$TIMER_DEV = 0x20
$TIMER_SETUP = 0x00 ; microseconds, dont reset after use
$TIMER_START = 0x01 ; start timer

:origin 0x100
start:      LDI R1, 1                       ; need a one to use for incrementing loops
            LDI R6, $TIMER_DEV              ; setup timer
            LDI R7, $TIMER_SETUP
            STA [RX6], R7
            STA [RX6 + 1], R0               ; load timer with 100us delay
            LDI R7, 100
            STA [RX6 + 2], R7
                                            ; print string in :str
            LDI R4, :str:h                  ; load location of hello world into RXA
            LDI R5, :str:l
            LDI RE, :printnl:h              ; set return address of print loop to printnl
            LDI RF, :printnl:l
            JMP :loop
                                            ; print string in :nl
printnl:    LDI R4, :nl:h                   ; load address into RXA
            LDI R5, :nl:l
            LDI RE, :done:h                 ; set return address to the done label
            LDI RF, :done:l

loop:       LDA R2, [RXA]                   ; read next character
            JPC R2, ~EQ, RZ                 ; skip jump if not zero
            JPF [RXF]                       ; jump to return address

print:      LDI R6, $SERIAL_STATUS          ; load and check if the tx buffer is empty
            LDA R6, [RX6]
            LDI R7, $SERIAL_TXREADY         ; load comparison
            BIT R6, AND, R7                 ; mask tx ready flag
            JNZ R6, :ready                  ; flag set, print char

wait_print: LDI R6, $TIMER_DEV              ; load and start timer
            LDI R7, $TIMER_START
            STA [RX6], R7
            JMP :print                      ; try again

ready:      LDI R6, $SERIAL_DATA            ; write char to serial output
            STA [RX6], R2

            ADD R5, R5, R1                  ; increment $R_POS
            ADC R4, R4, RZ                  ; carry to high byte
            JMP :loop

done:       HLT RZ, 0

:data :str STZ "Hello World"
:data :nl DW 0x0A00
