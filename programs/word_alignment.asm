; reset vector
:origin 0
        JMP     :start

:origin 0x100
; Test: DB followed by instruction should be padded
:data DB 0x42           ; 1 byte at 0x100, padded to 0x102
:data DB 0x43           ; 1 byte at 0x102, padded to 0x104

start:  LDI R1, 0x01    ; instruction at 0x102 (aligned)
        LDI R2, 0x02    ; instruction at 0x104 (aligned)
        JMP :a          ; instruction at 0x108

; Test: DX with odd byte count
:data DX 0x010203       ; 3 bytes at 0x10A

a:      LDI R3, 0x03    ; instruction at 0x10C (aligned)
        JMP :b          ; 0x10E

; Test: STZ with odd length string
:data STZ "Hi"          ; 3 bytes (2 chars + null) at 0x110 padded to 0x113

b:      LDI R4, 0x04    ; instruction at 0x114 (aligned)
        JMP :final      ; 0x116

; Test: DW is already aligned (no padding needed)
:data DW 0x1234         ; 2 bytes at 0x118, no padding
final:  LDI R5, 0x05    ; instruction at 0x11A (aligned)

        LDI R6, :final:h
        LDI R7, :final:l

        LDI RA, 0x01        ; expected address is 0x0114
        JPC R6, EQ, RA      ; don't jump is R6 = RA
        JMP :FAIL

        LDI RA, 0x1A
        JPC R7, EQ, RA
        JMP :FAIL

        HLT 0

FAIL:   HLT :final
