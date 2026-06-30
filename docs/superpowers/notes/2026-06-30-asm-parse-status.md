# asm_parse Implementation Status

> **Date:** 2026-06-30
> **Branch:** asm-parse

## What's Implemented

The `asm_parse` function converts a token stream into a structured `ASMProgram` AST. The following line types are fully parsed:

### Line Types

| Line Type | Status | Notes |
|-----------|--------|-------|
| Empty lines | ✅ Complete | NEWLINE tokens become ASM_PROGRAM_LINE_EMPTY |
| Comment-only lines | ✅ Complete | Comments stored in `line->comment` |
| `:origin` directive | ✅ Complete | Sets `current_address`, stores in ASM_PROGRAM_LINE_ORIGIN |
| `$alias` directive | ✅ Complete | Stores variable name and value (number or register) |
| `:data` directive | ⚠️ Partial | DB, DW, STZ, STL work; DX not implemented |
| Instructions | ⚠️ Partial | Opcode and label parsed; **operands not wired up** |

### Data Types

| Type | Status | Notes |
|------|--------|-------|
| DB | ✅ | Single byte (0-255) |
| DW | ✅ | Word, big-endian (0-0xFFFF) |
| STZ | ✅ | String + null terminator |
| STL | ✅ | Length-prefixed string (2-byte length + string) |
| DX | ❌ | Hex data - returns "not yet implemented" error |

### Instruction Parsing

- **Opcode mapping**: All 16 opcodes (HLT, LDA, STA, LDI, ADD, ADC, SUB, SBC, NOT, AND, SHL, SHR, JMP, JPF, JNZ, JPC) correctly mapped
- **Labels**: Optional label prefix parsed and stored
- **Operands**: Helpers exist (`parse_reg8_operand`, `parse_reg16_operand`) but are **not wired into the instruction parser**. Instructions currently expect immediate NEWLINE after opcode.

### Memory Management

- `asm_free_program` correctly frees all dynamic fields:
  - `program->filename`
  - `line->comment`
  - `instruction.label`, `instruction.operands` (including variable/label strings)
  - `data.label`, `data.data`
  - `alias.variable`

---

## Deferred / Incomplete Tasks

### 1. Instruction Operand Parsing (Critical)

**What's missing:** The parser has operand helper functions but doesn't call them. Instructions with operands (i.e., all except HLT with value 0) will fail with "expected newline after instruction".

**What's needed:** Wire up operand parsing based on instruction format:

| Format | Instructions | Operands |
|--------|--------------|----------|
| Q | HLT | `value12` |
| Q | JPF | `[reg16]` or `[reg16+rel]` |
| Q | JNZ | `reg8, [rel]` |
| S | ADD, ADC, SUB, SBC, AND | `reg8, reg8, reg8` |
| T | LDA | `reg16, [reg16]` or `reg16, [reg16+rel]` |
| T | STA | `[reg16], reg16` or `[reg16+rel], reg16` |
| T | LDI | `reg8, value8` |
| T | NOT | `reg8` |
| T | SHL, SHR | `reg8, value4` |
| T | JPC | `reg8, test, reg8` |
| V | JMP | `rel` |

**Helper functions needed:**
- `parse_value_operand()` - numeric literal, `$var`, `:label:h/l`
- `parse_rel_operand()` - relative offset (`+/-NUMBER` or `:label`)
- `parse_mem_operand()` - `[reg16]` or `[reg16+rel]`
- `parse_test_operand()` - optional `~` followed by TEST token

**Known issue:** `parse_reg8_operand` accepts 16-bit registers (RX-prefixed tokens) because `parse_register_id` returns 0-15 for both R0-RF and RX0-RXF. Should add guard: `if (tok->value[0] == 'R' && tok->value[1] == 'X') return 0;`

### 2. DX Data Type

**What's missing:** `:data :label DX 0A0B0C0D` should parse hex digit pairs into bytes.

**What's needed:** Parse hex string, validate even length, convert pairs to bytes.

### 3. Symbol Table

**What's missing:** Labels (instruction labels, data labels) are stored on individual lines but not collected into a symbol table for later resolution.

**What's needed:**
- First pass: collect all labels with their addresses
- Second pass: resolve label references in operands
- Handle forward references (labels used before defined)

### 4. Variable Resolution

**What's missing:** `$alias` definitions are stored but not resolved. When an operand references `$ACC`, the parser should look up the alias value.

**What's needed:**
- Build alias map during parsing
- Resolve `$var` references in operands to their values

### 5. Address Tracking Edge Cases

**What's missing:**
- DX data size calculation (N hex digits / 2 bytes)
- Validation that addresses don't overflow 0xFFFF

---

## Next Steps

### Phase 1: Complete Operand Parsing

1. Implement format-specific operand parsing for each instruction type
2. Wire up `parse_reg8_operand`, `parse_reg16_operand` in instruction parser
3. Add `parse_value_operand`, `parse_rel_operand`, `parse_mem_operand`, `parse_test_operand`
4. Update `test_free.c` to test instruction with operands
5. Update integration test to expect fewer parse errors

### Phase 2: Data Completeness

1. Implement DX hex data parsing
2. Add tests for DX

### Phase 3: Symbol Resolution

1. Add symbol table structure to `ASMProgram`
2. First pass: collect labels and aliases
3. Second pass: resolve references
4. Handle undefined symbol errors

### Phase 4: Code Generation

1. Implement `asm_emit()` to convert `ASMProgram` to binary
2. Handle instruction encoding (Q/S/T/V formats)
3. Handle data emission
4. Handle label/variable resolution during emission

---

## Files Modified

| File | Changes |
|------|---------|
| `asm/tree.h` | Added `label` field to `ASMProgramData`; renamed enum values (ASM_INST_SYS→JMP, etc.) |
| `asm/tree.c` | Implemented `asm_parse` with line dispatch, helpers, and `asm_free_program` |
| `asm/asm.c` | Added EOF token append before `asm_parse` call |
| `tests/test_*.c` | Unit tests for each line type |
| `tests/test_integration.sh` | Integration test for example programs |

---

## Known Issues

1. **test_free.c fails** - Tests instruction with operand (`HLT 0`) but operand parsing not implemented. Will pass once operands are wired up.

2. **parse_reg8_operand accepts 16-bit registers** - Inherited from spec. Should reject RX-prefixed tokens.

3. **EOF error location** - Reports `0:0` for line/col when EOF missing. Should use last token's location.

4. **malloc failure for comment silently swallowed** - `line->comment` becomes NULL on alloc failure. Matches brief but inconsistent with fail-fast philosophy.
