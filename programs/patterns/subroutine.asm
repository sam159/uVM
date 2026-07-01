; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100
; Subroutine call/return pattern
; Calls a subroutine that doubles a value using register passing convention
; R1 = constant 1
; R2 = argument / result
; RXC (R8:R9) = return address

$ONE = R1
$ARG = R2

start:  LDI     $ONE, 1
        LDI     $ARG, 5             ; argument = 5

        ; save return address in RXC
        LDI     R8, :return:h
        LDI     R9, :return:l
        JMP     :double


        ; R2 should be 10 (5 * 2)
return: HLT     $ARG

; subroutine: doubles the value in R2

double: ADD     $ARG, $ARG, $ARG    ; R2 = R2 + R2
        JPF     [RXC]               ; return to address in RXC
