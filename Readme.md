# uVM

This is an 8-bit virtual machine with 16-bit addressing. It is RISC-style with no direct operations on memory, instead
operations are done on one of 16 8-bit registers with 4 16-bit being a re-use of the upper 8-bit registers.

## VM

### Registers and Flags

- **8-bit registers** 16 registers (`R0`–`RF`), 8-bit each.
- **16-bit registers** 16 registers (`RX0`-`RXF`) mapped over the 8-bit registers.
  - `RX0`-`RX9` with low bytes mapped to
  - `RXA`-`RXF` mapped to pairs of 8-bit registers
- **R0/RX0** always reads as 0. Writes to R0 are a no-op.
- **PC** (program counter), 16-bit. Increments by 2 each instruction fetch. Must be word-aligned.
- **carry**, 1-bit flag used by ADC and SBC.

### 16-bit Register Mapping

Some instructions can use two 8-bit registers as one 16-bit register for addressing. 

The upper 12 registers form six extended 16-bit address registers (big-endian, high byte : low byte):

| Alias | Registers |
|-------|-----------|
| RXA   | R4 : R5   |
| RXB   | R6 : R7   |
| RXC   | R8 : R9   |
| RXD   | RA : RB   |
| RXE   | RC : RD   |
| RXF   | RE : RF   |

`vm_get_rx()` returns the 16-bit value of a register: 0x0-0x9 return the 8-bit variant, with 0xA-0xF returning the above registers.  

### Memory

- 64KB address space (0x0000 - 0xFFFF).
- All operations work on registers only, no direct memory-to-memory operations.
- Memory access is via hook functions: `readAddr` and `writeAddr` on the VM struct.

### Instruction Format

All instructions are two bytes (big-endian). The upper four bits are the opcode. Three operand formats share the lower 12 bits:

| Format | Layout (16 bits)             | Description                                     |
|--------|------------------------------|-------------------------------------------------|
| Q      | `[op:4][rd:4][imm8:8]`       | Destination register + 8-bit immediate          |
| S      | `[op:4][rd:4][rx:4][ry:4]`   | Destination + two source registers              |
| T      | `[op:4][rd:4][rx:4][imm4:4]` | Destination + source register + 4-bit immediate |
| V      | `[op:4][imm12:12]`           | 12-bit immediate                                |


### Operations

Operand terms
- `exitcode` - sets the vm exitcode
- `rd`/`rx`/`ry` - 8-bit register
- `rdx` - `rd` with extended register support
- `imm4` - immediate value 4-bit
- `imm4_s` - immediate value 4-bit
- `imm4_s_x2` - immediate value 4-bit multiplied by two
- `test` - a conditional jump type
- `imm8` - immediate value 8-bit 
- `imm8_s` - signed immediate value 8-bit 
- `imm8_s_x2` - signed immediate value 8-bit multiplied by two
- `imm12` - immediate value 12-bit
- `imm12_s` - signed immediate value 12-bit
- `imm12_s_x2` - signed immediate value 12-bit multiplied by two
- `carry` - carry flag

All jumps are in words/instructions. 

| Op  | Mnemonic | Format | Operation                          | Description                                                                               |
|-----|----------|--------|------------------------------------|-------------------------------------------------------------------------------------------|
| `0` | HLT      | V      | `exitcode ← imm12`                 | Halt the VM. Sets `halted = true`, and the exit code to `imm12`                           |
| `1` | LDA      | T      | `rdx ← [rdx + imm4]`               | Load byte from memory at 16-bit address `rdx + imm4` into `rdx`                           |
| `2` | STA      | T      | `[rdx + imm4] ← rx`                | Store `rx` to memory at 16-bit address `rdx + imm4`                                       |
| `3` | LDI      | Q      | `rd ← imm8`                        | Load `imm8` into `rd`                                                                     |
| `4` | ADD      | S      | `rd ← rx + ry`                     | 8-bit add. Clears carry                                                                   |
| `5` | ADC      | S      | `rd ← rx + ry + carry`             | 8-bit add with carry. Sets carry on overflow                                              |
| `6` | SUB      | S      | `rd ← rx - ry`                     | 8-bit subtract. Clears carry                                                              |
| `7` | SBC      | S      | `rd ← rx - ry - carry`             | 8-bit subtract with borrow. Propagates borrow bit-by-bit into carry                       |
| `8` | NOT      | T      | `rd ← ~rx`                         | Bitwise NOT of `rx`                                                                       |
| `9` | AND      | S      | `rd ← rx & ry`                     | Bitwise AND of `rx` and `ry`                                                              |
| `A` | SHL      | T      | `rd ← rx << imm4`                  | Logical left shift by `imm` bits                                                          |
| `B` | SHR      | T      | `rd ← rx >> imm4`                  | Logical right shift by `imm` bits                                                         |
| `C` | JMP      | V      | `PC ← PC + imm12_s_x2`             | Relative jump. Update PC with `imm12_s_x2`                                                |
| `D` | JPF      | Q      | `PC ← rdx + imm8_s_x2`             | Far jump. Set PC to the value in `rdx` with `imm8_s_x2` value                             |
| `E` | JNZ      | Q      | `PC ← PC + imm8_s_x2` if `rd != 0` | Conditional jump. Add `imm8_s_x2` to PC if `rd` is not zero                               |
| `F` | JPC      | T      | `PC ← PC + 2` if `!test(rd,rx)`    | Conditional jump. Skips the next instruction if `test` passes for values in `rd` and `rx` |

## Conditional Jumps

Test types, used for comparing values in `rd` and `rx`.

Tests are mapped into a 4-bit value and can be in any combination
```text
3   2   1   0
|   |   |   |
|   |   |   +-- Equality
|   |   +------ Less Than
|   +---------- Greater Than
+-------------- Negate Result
```

Setting bits 0–3 will pass for any value, negating it will not pass for any value,
similarly not setting any bits will not pass for any value.

| Mnemonic | Number | Binary | Test                                         |
|----------|--------|--------|----------------------------------------------|
| EQ       | 1      | 0001   | Equals                                       |
| ~EQ      | 9      | 1001   | Not equals                                   |
| LT       | 2      | 0010   | Less than                                    |
| ~LT      | 10     | 1010   | Not less than                                |
| LTE      | 3      | 0011   | Less than or equal                           |
| ~LTE     | 11     | 1011   | Not less than or equal                       |
| GT       | 4      | 0100   | Greater than                                 |
| ~GT      | 12     | 1100   | Not greater than                             |
| GTE      | 5      | 0101   | Greater than or equal                        |
| ~GTE     | 13     | 1101   | Not greater than or equal                    |

### Error Handling

Errors are reported via the `error` hook and halt the VM:

| Code | Constant               | Condition                                    |
|------|------------------------|----------------------------------------------|
| 1    | `VM_ERR_MISALIGN`      | PC is not word-aligned on instruction fetch. |
| 2    | `VM_ERR_UNKNOWN_OP`    | Opcode is not recognized (0–F).              |
| 3    | `VM_ERR_OUT_OF_BOUNDS` | Memory access address >= 0x10000.            |
| 4    | `VM_ERR_JUMP_SELF`     | An jump instruction jumped to itself         |

### Execution Model

1. `vm_step()` fetches one 2-byte instruction from `readAddr(PC)`.
2. Decodes the opcode and operand format.
3. Executes the operation, writing the result to `rd` via `vm_put_r()`.
4. Increments PC by 2 (except for jumps which set PC directly).
5. The main loop calls `vm_step()` repeatedly until `vm->halted` is true.
