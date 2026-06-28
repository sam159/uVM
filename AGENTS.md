# uVM

8-bit VM with 16-bit addressing. C11, single-target CMake build.

## Build & run

```
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
./cmake-build-debug/uVM
```

Single executable: `main.c vm.c vm.h` in `CMakeLists.txt`.

## Architecture

- **16 registers** (R0–R15), 8-bit each. R0 always reads 0, writes to R0 are no-op.
- **16-bit addressing** via upper 8-bit registers: RXB = R6:R7, RXC = R8:R9, RXD = R10:R11, RXE = R12:R13, RXF = R14:R15.
- **16KB memory** (0x0000–0xFFFF). All ops work on registers only — no direct memory-to-memory ops.
- **2-byte instructions** (big-endian). Three formats: Q (rd + imm), S (rd + rx + ry), T (rd + rx + imm).
- **16 opcodes**: HLT(0), LDA(1), STA(2), LDI(3), ADD(4), ADC(5), SUB(6), SBC(7), NOT(8), AND(9), SHL(a), SHR(b), SYS(c), JMP(d), JEQ(e), JLT(f).
- **Errors**: misaligned PC, unknown opcode, out-of-bounds memory access. All halt the VM.
- `vm_step()` executes one instruction. Main loop calls it until `vm->halted`.

## Directories

- `arduino/` — Arduino port with its own `VM.c`/`VM.h`, console, shift-register I/O
- `asm/` — empty, reserved for assembler
- `pcb/` — KiCad project (uvm.pro)
- `cmake-build-*/` — build output, gitignored

## Host interface (vm.h)

Hooks into the VM via function pointers on the `VM` struct: `readAddr`, `writeAddr`, `syscall`, `error`. The root `main.c` provides a simple memory-backed host with one syscall (print register pair).
