; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100
; Nested loop multiplication
; Computes R4 = R2 * R3 via repeated addition
; Outer loop runs R2 times, inner loop adds R3 to accumulator each iteration
; R1 = constant 1
; R2 = outer counter (multiplier)
; R3 = inner limit (multiplicand)
; R4 = accumulator (result)
; R5 = inner counter (reset each outer iteration)

$ONE = R1
$OUTER = R2
$ILIM = R3
$ACC = R4
$INNER = R5

start:  LDI     $ONE, 1
        LDI     $OUTER, 4           ; multiplier = 4
        LDI     $ILIM, 3            ; multiplicand = 3
        LDI     $ACC, 0             ; result = 0

; outer loop: once per multiplier count
oloop:  ADD     $INNER, $ILIM, R0   ; reset inner counter = ILIM

; inner loop: increment accumulator INNER times
iloop:  ADD     $ACC, $ACC, $ONE    ; acc++
        SUB     $INNER, $INNER, $ONE ; inner--
        JNZ     $INNER, :iloop      ; while inner != 0

        SUB     $OUTER, $OUTER, $ONE ; outer--
        JNZ     $OUTER, :oloop      ; while outer != 0

        ; R4 = 12 (4 * 3)
        HLT     0
