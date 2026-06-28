# uVM Assembly Format

## Syntax

```
[label:]  opcode  operands  ; comment
```

- One instruction per line
- Labels end with `:` and mark addresses
- Comments follow `;`
- Case-insensitive

### Labels

Labels can be referenced by `:[label]`. The 16-bit address is resolved by the assembler.

## Registers

- `R0` / `ZERO` zero register (Always reads as zero)
- `R1` - `RF` 8-bit registers
- `RXB` - `RXF` 16-bit registers

## Instruction formats

Three encoding families map to assembly formats:

| Format | Layout                       | Assembly         |
|--------|------------------------------|------------------|
| Q      | opcode(4) rd(4) imm(8)       | `OP rd, imm`     |
| S      | opcode(4) rd(4) rx(4) ry(4)  | `OP rd, rx, ry`  |
| T      | opcode(4) rd(4) rx(4) imm(4) | `OP rd, rx, imm` |
| V      | opcode(4) imm(12)            | `OP rd, imm`     |

Refer to the (Readme.md)[Readme.md] for the available instructions and which format to use for each. Keep in mind that
some instructions can operate on the RX registers.

## Directives

### :origin — Set origin

```
:origin address
```

Sets the assembly location counter to `address`. All subsequent labels and instructions are offset from this address. Default origin is `0x0000`.

### $alias - register aliases

```asm
$Example = R2
$Value = 0x100
```

Assigns a user mnemonic to a register or literal value

## Examples

It should be common practice to treat R1 as a source for the value of one, for incrementing/decrementing indexes into
arrays as there is no way to add/subtract immediate values.

# Increment value by two 100 times

```asm
:origin 0x100

; Variables
$ONE = R1
$ACC = R2
$INC = R3
$COUNTER = R4

; Set initial values
        LDI     R1, $ONE                    ; load 1 into R1
        LDI     $ACC, 0                     ; accumulator = 0
        LDI     $INC, 2                     ; increment = 2
        LDI     $COUNTER, 100               ; iterations = 100

; Addition loop
loop:   ADD     $ACC, $ACC, $INC            ; accumulator += increment
        SUB     $COUNTER, $COUNTER, $ONE    ; iterations--
        JNZ     $COUNTER, :loop             ; if iterations is not zero, jump back to label loop

; Stop VM
        HLT     0                           ; halt with exit code 0
```
