:addr 0x0

; Sum 0..99 into R1
        LDI     R1, 0       ; accumulator = 0
        LDI     R2, 0       ; counter = 0
        LDI     R3, 99      ; limit = 99

loop:   ADD     R1, R1, R2    ; acc += counter
        ADD     R2, R2, 1     ; counter++
        JEQ     R1, R2, R3    ; if counter == limit, fall through
        JMP     R0, loop      ; else jump to loop (R0=0, address in upper reg)

        HLT
