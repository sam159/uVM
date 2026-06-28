# uVM Assembly Format

## Syntax

```
[label:]  opcode  operands  ; comment
```

- One instruction per line
- Labels end with `:` and mark addresses
- Comments follow `;`
- Case-insensitive

## Registers

- `R0` - `RF` 8-bit registers
- `RXB` - `RXF` 16-bit registers

## Instruction formats

Three encoding families map to assembly formats:

| Format | Layout                       | Assembly         |
|--------|------------------------------|------------------|
| Q      | opcode(4) rd(4) imm(8)       | `OP rd, imm`     |
| S      | opcode(4) rd(4) rx(4) ry(4)  | `OP rd, rx, ry`  |
| T      | opcode(4) rd(4) rx(4) imm(4) | `OP rd, rx, imm` |

Refer to the (Readme.md)[Readme.md] for the available instructions and which format to use for each. Keep in mind that
some instructions can operate on the RX registers.

The `HLT` instruction takes no operands.

## Directives

### :origin — Set origin

```
:origin address
```

Sets the assembly location counter to `address`. All subsequent labels and instructions are offset from this address. Default origin is `0x0000`.

## Examples

```asm
; Sum 0..99 into R1
        LDI     R1, 0       ; accumulator = 0
        LDI     R2, 0       ; counter = 0
        LDI     R3, 99      ; limit = 99

loop:   ADD     R1, R1, R2    ; acc += counter
        ADD     R2, R2, 1     ; counter++
        JEQ     R1, R2, R3    ; if counter == limit, fall through
        JMP     R0, loop      ; else jump to loop (R0=0, address in upper reg)

        HLT
```

## Notes

- Immediate values are 8-bit (0–255) or 4-bit (0–15) depending on the instruction.
- Jump targets use address registers (RXB–RXF) for 16-bit PC loads.
- Labels resolve to byte addresses (even numbers only — instructions are 2-byte aligned).
