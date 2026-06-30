:origin 0x100
; Memory copy: copies 4 bytes from inline source data to 0x0300
; Demonstrates LDA/STA with pointer registers and loop control
; R1 = constant 1
; R2 = temporary byte
; R3 = remaining count
; RXB (R6:R7) = source pointer
; RXC (R8:R9) = destination pointer

$ONE = R1
$BYTE = R2
$CNT = R3

        JMP     :init

; source data (jumped over at start)
:data :srcdata DB 0xAA
:data DB 0xBB
:data DB 0xCC
:data DB 0xDD

:init
        LDI     $ONE, 1

        ; source pointer -> inline data
        LDI     R6, :srcdata:h
        LDI     R7, :srcdata:l

        ; destination pointer -> 0x0300
        LDI     R8, 0x03
        LDI     R9, 0x00

        LDI     $CNT, 4             ; copy 4 bytes

:loop   LDA     $BYTE, [RXB]        ; load byte from source
        STA     [RXC], $BYTE        ; store byte to destination
        ADD     R7, R7, $ONE        ; source pointer++
        ADD     R9, R9, $ONE        ; dest pointer++
        SUB     $CNT, $CNT, $ONE    ; count--
        JNZ     $CNT, :loop

        ; memory at 0x0300-0x0303 should be AA BB CC DD
        HLT     0
