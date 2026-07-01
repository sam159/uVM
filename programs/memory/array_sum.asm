; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100
; Array sum: sums 5 bytes stored as inline data
; Demonstrates LDA with pointer register, :data DB, and loop
; R1 = constant 1
; R2 = accumulator (sum)
; R3 = remaining count
; R4 = current element
; RXB (R6:R7) = pointer to current element

$ONE = R1
$SUM = R2
$CNT = R3
$ELEM = R4

; array data (jumped over at start)
:data :array DB 1
:data DB 2
:data DB 3
:data DB 4
:data DB 5
:data DB 0 ; padding

start:  LDI     $ONE, 1

        ; point RXB at array start
        LDI     R6, :array:h
        LDI     R7, :array:l

        LDI     $SUM, 0             ; sum = 0
        LDI     $CNT, 5             ; 5 elements

loop:   LDA     $ELEM, [RXB]        ; load current element
        ADD     $SUM, $SUM, $ELEM   ; sum += element
        ADD     R7, R7, $ONE        ; advance pointer
        SUB     $CNT, $CNT, $ONE    ; count--
        JNZ     $CNT, :loop

        ; R2 = 15 (1+2+3+4+5)
        HLT     0
