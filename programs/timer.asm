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

        LDI $R_CTRL, 0x03                   ; write control flags (reset, start)
        STA [RXA + 0], $R_CTRL              ; program should pause for 1ms

        LDI $R_DELAY, 0x0                   ; load 1ms delay into timer
        STA [RXA + 1], $R_DELAY
        LDI $R_DELAY, 0x1
        STA [RXA + 2], $R_DELAY

        LDI $R_CTRL, 0x07                   ; write control flags (ms, reset, start)
        STA [RXA + 0], $R_CTRL              ; program should pause for 1ms

        HLT R0, 0
