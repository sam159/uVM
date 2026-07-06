; reset vector
:origin 0
        JMP     :start

; timer device address
$TIMER_DEV = 0x20

$TIMER_MILLI = 0x04
$TIMER_RESET = 0x02
$TIMER_START = 0x01

$R_CTRL = R1
$R_DELAY = R2
$R_TMP = R3

:origin 0x100

; test us delay
start:  LDI R4, 0                           ; load address of timer into RXA
        LDI R5, $TIMER_DEV

        LDI $R_CTRL, $TIMER_RESET           ; set control flags to us, reset on finsh
        STA [RXA + 0], $R_CTRL

        LDI $R_DELAY, 0x3                   ; load 1000us delay into timer
        STA [RXA + 1], $R_DELAY
        LDI $R_DELAY, 0xE8
        STA [RXA + 2], $R_DELAY

        LDA $R_CTRL, [RXA + 0]              ; update control flag to set start
        LDI $R_TMP, $TIMER_START
        BIT $R_CTRL, OR, $R_TMP
        STA [RXA + 0], $R_CTRL              ; program should pause for 1ms

        LDI $R_DELAY, 0x0                   ; load 1ms delay into timer
        STA [RXA + 1], $R_DELAY
        LDI $R_DELAY, 0x1
        STA [RXA + 2], $R_DELAY

        LDA $R_CTRL, [RXA + 0]              ; update control flag to set start and millis
        LDI $R_TMP, $TIMER_MILLI
        BIT $R_CTRL, OR, $R_TMP
        LDI $R_TMP, $TIMER_START
        BIT $R_CTRL, OR, $R_TMP
        STA [RXA + 0], $R_CTRL              ; program should pause for 1ms

        HLT R0, 0
