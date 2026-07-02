:origin 0x100

; Test: DB followed by instruction should be padded
:data DB 0x42         ; 1 byte at 0x100, padded to 0x102
:data DB 0x43         ; 1 byte at 0x102, padded to 0x104
LDI R1, 0x01          ; instruction at 0x104 (aligned)
LDI R2, 0x02          ; instruction at 0x106 (aligned)

; Test: DX with odd byte count
:data DX 0x010203     ; 3 bytes at 0x108, padded to 0x10C
LDI R3, 0x03          ; instruction at 0x10C (aligned)

; Test: STZ with odd length string
:data STZ "Hi"        ; 3 bytes (2 chars + null) at 0x10E, padded to 0x112
LDI R4, 0x04          ; instruction at 0x112 (aligned)

; Test: DW is already aligned (no padding needed)
:data DW 0x1234       ; 2 bytes at 0x114, no padding
LDI R5, 0x05          ; instruction at 0x116 (aligned)

HLT 0
