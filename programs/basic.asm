:origin 0x100
; Set initial values
        LDI     R2, 0       ; accumulator = 0
        LDI     R3, 2       ; increment = 2
        LDI     R4, 100     ; iterations = 100

; addition loop
loop:   ADD     R2, R2, R3  ; accumulator += increment
        ADD     R2, R2, R1  ; counter++
        SUB     R4, R4, R1  ; iterations--
        JNZ     R4, :loop   ; if iterations is not zero, jump back to label loop

; stop vm
        HLT     0           ; halt with exit code 0
