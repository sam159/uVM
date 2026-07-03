# HLT Instruction: V Format to Q Format

## Summary

Change HLT from V format (12-bit immediate) to Q format (register + 8-bit immediate), enabling halt codes sourced from a register value and/or an immediate. The halt code becomes a full 16-bit value: `(reg_value << 8) | imm8`.

## Motivation

The current V format limits halt codes to 12 bits (0x000-0xFFF) and only supports compile-time constants. Q format enables runtime-determined halt codes via registers while also expanding the range to 16 bits.

## Encoding

```
Before (V format):  0000_iiiiiiiiiiii    HLT value12
After  (Q format):  0000_rrrr_iiiiiiii   HLT reg8, value8
```

- `rrrr` — 4-bit register select (R0-RF)
- `iiiiiiii` — 8-bit immediate value
- `haltCode = (vm_get_r(vm, rd) << 8) | imm`

### Examples

| Assembly          | haltCode           | Notes                          |
|-------------------|--------------------|--------------------------------|
| `HLT R0, 0`       | 0x0000             | Equivalent to old `HLT 0`      |
| `HLT R0, 2`       | 0x0002             | Equivalent to old `HLT 2`      |
| `HLT R2, 0`       | (R2 << 8)          | Register-only high byte        |
| `HLT R2, 0x42`    | (R2 << 8) \| 0x42  | Register high + immediate low  |

## Assembly Syntax

```
HLT reg8, value8
```

Both operands are required. `reg8` is R0-RF (or alias). `value8` is an 8-bit value (decimal, hex, binary, or `$variable`).

## Changes

### VM (`vm.c`)

`OP_HLT` case switches from `vm_decode_V` to `vm_decode_Q`:

```c
case OP_HLT:
    vm_decode_Q(&inst, raw);
    vm->halted = true;
    vm->haltCode = ((uint16_t)vm_get_r(vm, inst.rd) << 8) | inst.imm;
    break;
```

### Assembler (`asm/tree.c`)

1. Move HLT parsing from `parse_operands_v` to `parse_operands_q`:
   - Add `ASM_INST_HLT` branch in `parse_operands_q`
   - Parse two operands: `reg8`, comma, `value8`
   - Remove HLT branch from `parse_operands_v`
2. Move HLT from the V-format dispatch group to the Q-format dispatch group in `asm_parse` (line ~949)
3. Update emit logic (`ASM_INST_HLT` case):
   - Encode as Q format: `(op << 12) | (rd << 8) | imm`
   - Validate imm fits in 8 bits (instead of 12)

### Documentation (`ASM.md`)

Update HLT row in the instruction table:

```
| HLT | Q | HLT reg8, value8 |
```

Update examples that use `HLT 0` to `HLT R0, 0`.

### Existing Programs

Update all `.asm` files under `programs/`:
- `HLT 0` becomes `HLT R0, 0`

### Tests

- Update existing unit tests that construct HLT tokens to provide two operands (reg8 + value8) instead of one value12
- Update `test_instructions_q.c` which already tests Q format parsing with HLT
- Add test verifying haltCode includes register value contribution

### Arduino Port (`arduino/VM.c`)

Current Arduino HLT case does not set haltCode. Minimal change — update decode if needed, or leave as-is since it only sets `vm->halted = true`.

## Breaking Changes

This is a breaking change to the binary encoding and assembly syntax. All existing programs using `HLT value12` must be updated to `HLT reg8, value8`.
