# BIT Instruction Mnemonic Update

## Overview

Replace the raw 4-bit numeric value in the BIT instruction with human-readable mnemonics.

## Current Behavior

BIT uses T-format: `BIT reg8, value4, reg8` where `value4` is a raw nibble:
- 0x0 = AND
- 0x1 = OR
- 0x2 = XOR
- 0x4 = MOD

## New Behavior

BIT syntax: `BIT reg8, mnemonic, reg8` where mnemonic is one of:
- `AND` → 0x0 (`rd & rx`)
- `OR` → 0x1 (`rd | rx`)
- `XOR` → 0x2 (`rd ^ rx`)
- `MOD` → 0x4 (`rd % rx`, zero-divide guard)

Only mnemonics accepted; numeric values are rejected.

## Binary Encoding

Unchanged. The assembler maps mnemonics to the same 4-bit values. VM code is unaffected.

## Files Changed

- `src/asm/tree.c` — parse_operands_t() for BIT; binary emission for BIT
- `ASM.md` — instruction table and operand documentation
- `tests/test_bit_asm.c` — use mnemonics in token stream

## Tests

- `test_bit_asm.c`: update token stream to use mnemonics, verify same binary output
- `test_bit_vm.c`: no changes needed (binary encoding identical)
