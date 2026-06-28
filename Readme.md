# uVM

This is an 8-bit virtual machine with 16-bit addressing. It is RISC-style with no direct operations on memory, instead
operations are done on one of 16 8-bit registers with 4 16-bit being a re-use of the upper 8-bit registers.

## VM

### Registers and Flags

- **16 registers** (R0–RF), 8-bit each.
- **R0** always reads as 0. Writes to R0 are a no-op.
- **PC** (program counter), 16-bit. Increments by 2 each instruction fetch. Must be word-aligned.
- **carry**, 1-bit flag used by ADC and SBC.

### 16-bit Address Registers

The upper 10 registers form five extended 16-bit address registers (big-endian, high byte : low byte):

| Alias | Registers |
|-------|-----------|
| RXB   | R6 : R7   |
| RXC   | R8 : R9   |
| RXD   | RA : RB   |
| RXE   | RC : RD   |
| RXF   | RE : RF   |

`vm_get_rx()` returns the 16-bit value of a register: 0x0-0xA return the 8-bit variant, with 0xB-0xF returning the above registers.  

### Memory

- 64KB address space (0x0000 - 0xFFFF).
- All operations work on registers only, no direct memory-to-memory operations.
- Memory access is via hook functions: `readAddr` and `writeAddr` on the VM struct.

### Instruction Format

All instructions are two bytes (big-endian). The upper four bits are the opcode. Three operand formats share the lower 12 bits:

| Format | Layout (16 bits)            | Description                                     |
|--------|-----------------------------|-------------------------------------------------|
| Q      | `[op:4][rd:4][imm:8]`       | Destination register + 8-bit immediate          |
| S      | `[op:4][rd:4][rx:4][ry:4]`  | Destination + two source registers              |
| T      | `[op:4][rd:4][rx:4][imm:4]` | Destination + source register + 4-bit immediate |

### Operations

`rd` is always an 8-bit register, while `rdx` may be a 16-bit address register if above 0xA. Operands `rx` and `ry` are always 8-bit registers.

| Op  | Mnemonic | Format | Operands                 | Description                                                          |
|-----|----------|--------|--------------------------|----------------------------------------------------------------------|
| `0` | HLT      | —      | —                        | Halt the VM. Sets `halted = true`.                                   |
| `1` | LDA      | T      | `rd ← [rdx + imm]`       | Load byte from memory at 16-bit address `rdx + imm` into `rdx`.      |
| `2` | STA      | T      | `[rdx + imm] ← rx`       | Store `rx` to memory at 16-bit address `rdx + imm`.                  |
| `3` | LDI      | Q      | `rd ← imm`               | Load 8-bit immediate value into `rd`.                                |
| `4` | ADD      | S      | `rd ← rx + ry`           | 8-bit add. Clears carry.                                             |
| `5` | ADC      | S      | `rd ← rx + ry + carry`   | 8-bit add with carry. Sets carry on overflow.                        |
| `6` | SUB      | S      | `rd ← rx - ry`           | 8-bit subtract. Clears carry.                                        |
| `7` | SBC      | S      | `rd ← rx - ry - borrow`  | 8-bit subtract with borrow. Propagates borrow bit-by-bit into carry. |
| `8` | NOT      | T      | `rd ← ~rx`               | Bitwise NOT of `rx`.                                                 |
| `9` | AND      | S      | `rd ← rx & ry`           | Bitwise AND of `rx` and `ry`.                                        |
| `A` | SHL      | T      | `rd ← rx << imm`         | Logical left shift by `imm` bits.                                    |
| `B` | SHR      | T      | `rd ← rx >> imm`         | Logical right shift by `imm` bits.                                   |
| `C` | SYS      | T      | `rd ← syscall(rx, imm)`  | Syscall with call number `rx` and parameter `imm`. Result in `rd`.   |
| `D` | JMP      | Q      | `PC ← rdx`               | Unconditional jump to address in `rdx`.                              |
| `E` | JEQ      | S      | `PC ← rdx` if `rx == ry` | Conditional jump to address in `rdx` if `rx == ry`.                  |
| `F` | JLT      | S      | `PC ← rdx` if `rx < ry`  | Conditional jump to address in `rdx` if `rx < ry` (unsigned).        |

### Error Handling

Errors are reported via the `error` hook and halt the VM:

| Code | Constant               | Condition                                    |
|------|------------------------|----------------------------------------------|
| `4`  | `VM_ERR_MISALIGN`      | PC is not word-aligned on instruction fetch. |
| `5`  | `VM_ERR_UNKNOWN_OP`    | Opcode is not recognized (0–F).              |
| `6`  | `VM_ERR_OUT_OF_BOUNDS` | Memory access address >= 0x10000.            |

### Execution Model

1. `vm_step()` fetches one 2-byte instruction from `readAddr(PC)`.
2. Decodes the opcode and operand format.
3. Executes the operation, writing the result to `rd` via `vm_put_r()`.
4. Increments PC by 2 (except for jumps which set PC directly).
5. The main loop calls `vm_step()` repeatedly until `vm->halted` is true.
