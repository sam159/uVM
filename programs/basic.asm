
; Register assignments
$ONE = R1
$R_ACC = R2
$R_COUNT = R3
$R_INC = R4

; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100
; Set initial values
start:  LDI     $ONE, 1           ; constant 1
        LDI     $R_ACC, 0           ; accumulator = 0
        LDI     $R_COUNT, 50        ; iterations = 50
        LDI     $R_INC, 2           ; increment = 2

; addition loop
loop:   ADD     $R_ACC, $R_ACC, $R_INC          ; accumulator += increment
        SUB     $R_COUNT, $R_COUNT, $ONE      ; iterations--
        JNZ     $R_COUNT, :loop                 ; if iterations is not zero, jump back to label loop

; stop vm
        HLT     0                               ; halt with exit code 0
