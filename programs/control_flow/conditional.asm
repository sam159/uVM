; reset vector
:origin 0
        JMP     :start

; main program
:origin 0x100

; Conditional branching: find max of two values
; Demonstrates JPC (conditional skip) for if/else patterns
; JPC skips the next instruction when the test is true,
; so it is paired with JMP to form branches.
; R1 = constant 1
; R2 = value A (5)
; R3 = value B (8)
; R4 = max(A, B)

$A = R2
$B = R3
$MAX = R4

start:  LDI     R1, 1
        LDI     $A, 5               ; A = 5
        LDI     $B, 8               ; B = 8

        ; JPC skips JMP if A > B; since 5 < 8, JMP executes to :b_max
        JPC     $A, GT, $B
        JMP     :b_max
a_max:  ADD     $MAX, $A, R0       ; MAX = A
        JMP     :done
b_max:  ADD     $MAX, $B, R0       ; MAX = B

        ; R4 = 8
done:   HLT     0
