# uVM Assembly Format

## Syntax

Each line should be one of
- `:origin value` - origin directive
- `:data [#label] type value` - data directive
- `$ALIAS = value` - alias directive
- `[label:] opcode operands ; comment` - instruction
- `; comment` - comment only or whitespace
- empty line

All identifiers and operands are case-sensitive.
Spaces and tabs are ignored.
Lines may be up to 1024 bytes in length and terminated with either LF or CRLF.
A word in two bytes in big-endian format.

## Registers

- `R0` / `RX0` / `RZ` / `RXZ` zero register (Always reads as zero)
- `R1` - `RF` 8-bit registers
- `RX1` - `RX9` 16-bit registers with low bytes mapped to `R1`-`R9`
- `RXA` - `RXF` 16-bit registers

## Instruction formats

Three encoding families map to assembly formats:

| Format | Layout                       | Assembly         |
|--------|------------------------------|------------------|
| Q      | opcode(4) rd(4) imm(8)       | `OP rd, imm`     |
| S      | opcode(4) rd(4) rx(4) ry(4)  | `OP rd, rx, ry`  |
| T      | opcode(4) rd(4) rx(4) imm(4) | `OP rd, rx, imm` |
| V      | opcode(4) imm(12)            | `OP imm`         |

## Instructions

Operands
- `value4` / `value8` / `value12`
  - values beyond what can fit in the destination will result in an error 
  - one of the following
    - absolute value.
      - `0` - decimal
      - `0x0` - hex
      - `0000b` - binary
    - reference value
      - `$var` - variable
      - `:label:h` - high byte of address at `:label`
      - `:label:l` - low byte of address at `:label`
      - `:label` - truncated address at `:label`
- `rel` - relative address
  - values beyond what can fit in the destination will result in an error
  - preceded by `-` or `+`
  - one of
    - absolute value
      - `0` - decimal
      - `0x0` - hex
    - `:label` - computed distance in words to lable
- `reg8` - an 8-bit register - `RZ` or `R0`-`RF` or variable
- `=reg8` - same as `reg` but use value for both `rd` and `rx`
- `reg16` - a 16-bit register - `RxZ` or `RX0`-`RXF` or variable
- `test` - one of, optionally preceded by a `~` to negate it
  - `EQ` / `LT` / `LTE` / `GT` / `GTE`

| Mnemonic | Format | Operands                                           |
|----------|--------|----------------------------------------------------|
| HLT      | V      | `HLT value12`                                      |
| LDA      | T      | `LDA reg16, [reg16]` or `LDA reg16, [reg16 + rel]` |
| STA      | T      | `STA [reg16], reg16` or `STA [reg16 + rel], reg16` |
| LDI      | Q      | `LDI reg8, value8`                                 |
| ADD      | S      | `ADD reg8, reg8, reg8`                             |
| ADC      | S      | `ADC reg8, reg8, reg8`                             |
| SUB      | S      | `SUB reg8, reg8, reg8`                             |
| SBC      | S      | `SBC reg8, reg8, reg8`                             |
| NOT      | T      | `NOT reg8`                                         |
| AND      | S      | `AND reg8, reg8, reg8`                             |
| SHL      | T      | `SHL reg8, value4`                                 |
| SHR      | T      | `SHR reg8, value4`                                 |
| JMP      | V      | `JMP rel`                                          |
| JPF      | Q      | `JPF [reg16]` or `JPF [reg16 + rel]`               |
| JNZ      | Q      | `JNZ reg8, [rel]`                                  |
| JPC      | T      | `JPC reg8, test, reg8`                             |

## Memory Addresses

References to memory addresses for STA/LDA should be bracketed. Examples:
- `LDA R2, [RXB]`
- `LDA RXB, [RXC+2]`
- `STA [RXB], R2`

## Relative Jumps

For JMP and JNZ, these are relative jumps from the current instruction that can go a certain distance backwards and
forwards. To signify direction, it should be prefixed with +/- followed by the amount or a label. Example:

```asm
LDI R1, 1
LDI R2, 10
:loop2  SUB R2, R2, R1
        JNZ R2, -:loop  : jump backwards if R2 == 0, -:loop resolves to current instruction - target instruction
        JMP +1          ; jump forwards one instruction
        HLT 1           ; jumped over
        HLT 0
```

## Directives

Limits
- Aliases and Labels must be alphanumeric and can contain underscores with a max length of 32 characters.

### `:origin` — Set origin

```
:origin address
```

Sets the assembly location counter to `address`. All following labels and instructions are offset from this address. Default origin is `0x0000`.

### `label:` — Address label

Marks the address of the next instruction with this label.

### `$alias` - Aliases

May reference an absolute value or register.

```asm
$Example = R2
$Value = 0x100
```

### `:data [:label] type value` - Literal data

Includes the specified data at the current location of the binary output. This is unguarded, and the program should ensure it is not executed as instructions.

Label is optional and must be prefixed with a hash. Referenced by `:label`.

the type may be one of
- `DB` - single byte
    - absolute value
- `DW` - word / two bytes
    -  absolute value
- `DX` - hex data
    - one or more bytes in hex format, prefixed with `0x`
- `STZ` - null terminated string
    - value enclosed in single-quotes. Cannot have a new line. Escape single-quotes with a backslash
    - a zero is included at the end of the string
- `STL` - length prefixed string
    - value as above
    - the length of the string as a word is included before the string for loading into an extended register.

```asm
; examples
:orgin 0x1000
:data :byte1 DB 99      ; 99 in 0x1000
:data :byte2 DB 0x99    ; 153 in 0x1001
:data :word1 DW 0x1000  ; 4096 across 0x1002 and 0x1003

:origin 0x2000
:data :hex1 DX 0x010203040506   ; 6-bytes stored at 0x2000
:data :helloZ STZ 'Hello'       ; stores 0x65486C6C7200
:data :helloL STL 'Hello'       ; stores 0x000A65486C6C72
```

## Patterns / Practices

### `R1` as source for one

As there is no add/subtract with immediate value instruction, incrementing and decrementing indexes requires that a register has the amount in it. Using R1 for this solves that.

```asm
$ONE = R1
LDI $ONE, 1
```

### No-op

Example instructions for a no-op
- `JMP +0`
- `ADD =R0, R0`
- `NOT R0`

## Examples

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
