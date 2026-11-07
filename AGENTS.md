# uVM

8-bit VM with 16-bit addressing. C11, single-target CMake build.

## Build & test

```
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

Unit tests are individual C executables in `tests/` (see `CMakeLists.txt`). Integration test assembles all `.asm` files under `programs/`:

```
bash tests/test_integration.sh
```

Single executable built from `src/main.c src/vm.c src/vm.h src/utils.c src/utils.h src/asm/* src/mm/* src/devices/*` (see `CMakeLists.txt`; `src/asm/`, `src/mm/`, `src/devices/`, and `src/` are on the include path). Usage:

```
uVM program.bin                            # load at 0x0000, run from PC=0
uVM -a 0x100 f1.bin -a 0x200 f2.bin        # load multiple files at addresses
uVM -p 0x100 program.bin                   # set initial PC
uVM -c file.asm [-o output.bin]            # assemble into binary format
```

`src/main.c` validates load addresses against `VM_MEM_SIZE` and rejects overlapping loads. After the run it prints final VM state via `vm_print_state()`.

## Architecture

- **16 registers** (R0–R15), 8-bit each. R0 always reads 0, writes to R0 are no-op.
- **16-bit addressing** via upper 8-bit registers: RXB = R6:R7, RXC = R8:R9, RXD = RA:RB, RXE = RC:RD, RXF = RE:RF.
- **64KB memory** (0x0000–0xFFFF; `VM_MEM_SIZE = 0xFFFF`). All ops work on registers only — no direct memory-to-memory ops.
- **2-byte instructions** (big-endian). Four formats: Q (rd + imm), S (rd + rx + ry), T (rd + rx + imm), V (imm12).
- **16 opcodes**: HLT(0), LDA(1), STA(2), LDI(3), ADD(4), ADC(5), SUB(6), SBC(7), NOT(8), BIT(9), SHL(a), SHR(b), JMP(c), JPF(d), JNZ(e), JPC(f). See `ASM.md` for the full instruction table.
- **Test flags** (`vm.h`): `TEST_EQ`(1), `TEST_LT`(2), `TEST_GT`(4), `TEST_NOT`(8). Used by JPC. `vm_test()` evaluates them.
- **Carry flag** (1-bit): ADD/SUB clear it, ADC/SBC use it. `vm_subtract()` propagates borrow.
- **`vm_get_rx()`**: index 0x0–0xA returns the 8-bit register; 0xB–0xF returns the 16-bit address register (RXB–RXF).
- **Errors** (constants in `vm.h`): `VM_ERR_MISALIGN`(1), `VM_ERR_UNKNOWN_OP`(2), `VM_ERR_OUT_OF_BOUNDS`(3), `VM_ERR_JUMP_SELF`(4), `VM_ERR_INVALID_BIT_OP`(5). All halt the VM via the `error` hook.
- `vm_step()` executes one instruction. Main loop calls it until `vm->halted`. Exit code in `vm->haltCode` (uint16_t).

## Directories

- `arduino/` — Arduino port with its own `VM.c`/`VM.h`, console, shift-register I/O
- `src/` — main application sources
  - `src/asm/` — assembler sources: `asm.c`/`asm.h` (compile entry point), `token.c`/`token.h` (tokenizer), `tree.c`/`tree.h` (syntax tree types); `asm_compile(input, output)` tokenizes input, parses into syntax tree, resolves symbols, and emits binary output. `tree.md` documents the syntax tree structure.
  - `src/mm/` — memory management
  - `src/devices/` — memory and timer device implementations
- `programs/` — sample `.asm` sources; assembled `.bin` output is gitignored
- `tests/` — unit tests (individual C executables) and `test_integration.sh`
- `pcb/` — KiCad project (uvm.pro)
- `cmake-build-*/` — build output, gitignored

## Assembly format

See `ASM.md` for the assembly syntax: `[label:] opcode operands ; comment`, registers `R0`–`RF` / `RXA`–`RXF`, the `:origin` (a.k.a. `:addr`) directive, the `:data` directive for literal data (DB/DW/DX/STZ/STL), `$alias` definitions, and how Q/S/T/V formats map to assembly operands.

## Host interface (vm.h)

Hooks into the VM via function pointers on the `VM` struct: `readAddr`, `writeAddr`, `error`. `src/main.c` provides a simple memory-backed host. `src/utils.c`/`src/utils.h` provides `get_temp_dir()` and `copy_file()` helpers.
