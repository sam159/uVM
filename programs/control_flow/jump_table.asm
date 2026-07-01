; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100
; Jump table dispatch using JPF
; Loads a handler address from a table of words indexed by R2,
; then jumps to it via JPF. Demonstrates computed dispatch.
; R1 = constant 1
; R2 = dispatch index (0-2)
; R3 = byte offset (index * 2)
; R4 = result set by handler
; RXB (R6:R7) = table pointer
; RXC (R8:R9) = loaded target address

$ONE = R1
$IDX = R2
$OFF = R3
$RES = R4

start:  LDI     $ONE, 1
        LDI     $IDX, 1             ; dispatch index = 1

        ; point RXB at jump table
        LDI     R6, :jtable:h
        LDI     R7, :jtable:l

        ; compute byte offset = index * 2 (each entry is a word)
        ADD     $OFF, $IDX, $IDX

        ; advance table pointer by offset
        ADD     R7, R7, $OFF

        ; load target address from table (big-endian word)
        LDA     R8, [RXB]           ; high byte
        ADD     R7, R7, $ONE        ; pointer++
        LDA     R9, [RXB]           ; low byte

        ; jump to handler
        JPF     [RXC]

; should not reach here
        HLT     0xFF

; jump table: addresses of handlers
:data :jtable DW :handler0
:data DW :handler1
:data DW :handler2

handler0:   LDI     $RES, 10            ; result = 10
            JMP     :done

handler1:   LDI     $RES, 20            ; result = 20
            JMP     :done

handler2:   LDI     $RES, 30            ; result = 30

        ; R4 = 20 (handler1 was selected)
done:       HLT     0
