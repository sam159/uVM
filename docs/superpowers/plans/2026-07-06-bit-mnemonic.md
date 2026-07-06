# BIT Mnemonic Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace raw numeric values in the BIT instruction with human-readable mnemonics (AND, OR, XOR, MOD).

**Architecture:** Add a new operand type `ASM_OPERAND_MNEMONIC` to hold the mnemonic string. Add a dedicated `parse_bit_mnemonic_operand()` parser function. Update the assembler parser to accept mnemonic identifiers for BIT's middle operand, and the emitter to map mnemonic strings to the existing 4-bit values. VM code is unaffected.

**Tech Stack:** C11, CMake, existing assembler infrastructure.

## Global Constraints

- C11 single-target build, no new dependencies
- Mnemonics are case-sensitive: only AND, OR, XOR, MOD accepted
- Numeric values (0-15) are rejected for BIT
- Binary encoding is unchanged: same T-format, same opcode 0x9

---

### Task 1: Add ASM_OPERAND_MNEMONIC type

**Files:**
- Modify: `src/asm/tree.h:111-125`

**Interfaces:**
- Consumes: existing `ASMProgramInstructionOperand` union
- Produces: new operand type `ASM_OPERAND_MNEMONIC` with `char *value`

- [ ] **Step 1: Add the new operand type to the enum**

In `src/asm/tree.h`, add `ASM_OPERAND_MNEMONIC` to `ASMProgramInstructionOperandType` enum (after `ASM_OPERAND_TEST`):

```c
/* ASMProgramInstructionOperandType */
typedef enum {
    ASM_OPERAND_VALUE16,
    ASM_OPERAND_VALUE8,
    ASM_OPERAND_VALUE4,
    ASM_OPERAND_REL,
    ASM_OPERAND_REG8,
    ASM_OPERAND_REG8_DOUBLE,
    ASM_OPERAND_REG16,
    ASM_OPERAND_REG16_REL,
    ASM_OPERAND_VARIABLE,
    ASM_OPERAND_LABEL_HIGH,
    ASM_OPERAND_LABEL_LOW,
    ASM_OPERAND_LABEL,
    ASM_OPERAND_TEST,
    ASM_OPERAND_MNEMONIC,
} ASMProgramInstructionOperandType;
```

- [ ] **Step 2: Build to verify no compile errors**

Run: `cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build cmake-build-debug`
Expected: clean build, no errors

- [ ] **Step 3: Commit**

```bash
git add src/asm/tree.h
git commit -m "asm: add ASM_OPERAND_MNEMONIC operand type"
```

---

### Task 2: Add parse_bit_mnemonic_operand function

**Files:**
- Modify: `src/asm/tree.c` (add new static function after `parse_test_operand`)

**Interfaces:**
- Consumes: token list, position pointer
- Produces: operand with type `ASM_OPERAND_MNEMONIC` and `variable` set to "AND"/"OR"/"XOR"/"MOD"

- [ ] **Step 1: Add parse_bit_mnemonic_operand function**

Add this new static function after `parse_test_operand` (after line 323 in tree.c):

```c
static int parse_bit_mnemonic_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type != ASM_TOKEN_IDENT) {
        return 0;
    }

    if (strcmp(tok->value, "AND") != 0 &&
        strcmp(tok->value, "OR") != 0 &&
        strcmp(tok->value, "XOR") != 0 &&
        strcmp(tok->value, "MOD") != 0) {
        return 0;
    }

    op->type = ASM_OPERAND_MNEMONIC;
    op->variable = strdup(tok->value);
    if (!op->variable) return 0;

    (*pos)++;
    return 1;
}
```

- [ ] **Step 2: Build to verify**

Run: `cmake --build cmake-build-debug`
Expected: clean build

- [ ] **Step 3: Commit**

```bash
git add src/asm/tree.c
git commit -m "asm: add parse_bit_mnemonic_operand function"
```

---

### Task 3: Use mnemonic parser in BIT operands

**Files:**
- Modify: `src/asm/tree.c:565-598`

**Interfaces:**
- Consumes: new `parse_bit_mnemonic_operand()` from Task 2
- Produces: operand[1] with type `ASM_OPERAND_MNEMONIC`

- [ ] **Step 1: Replace parse_value_operand with parse_bit_mnemonic_operand for BIT**

In `src/asm/tree.c`, in `parse_operands_t()`, replace line 581:

From:
```c
        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, tokens->tokens[*pos].col, "expected value4");
```

To:
```c
        if (!parse_bit_mnemonic_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, tokens->tokens[*pos].col, "expected AND, OR, XOR, or MOD");
```

- [ ] **Step 2: Build to verify**

Run: `cmake --build cmake-build-debug`
Expected: clean build

- [ ] **Step 3: Commit**

```bash
git add src/asm/tree.c
git commit -m "asm: parse mnemonic instead of value for BIT instruction"
```

---

### Task 4: Emit BIT with mnemonic-to-value mapping

**Files:**
- Modify: `src/asm/tree.c:1492-1506`

**Interfaces:**
- Consumes: operand[1] with type `ASM_OPERAND_MNEMONIC` and `variable` field
- Produces: same T-format binary encoding as before

- [ ] **Step 1: Replace BIT emit to map mnemonic strings to values**

In `src/asm/tree.c`, replace the `ASM_INST_BIT` emit case (lines 1492-1506):

From:
```c
        case ASM_INST_BIT: {
            const uint8_t rd = inst->operands[0].reg8;
            const uint8_t rx = inst->operands[2].reg8;
            uint16_t val;
            if (inst->operands[1].type == ASM_OPERAND_VALUE16) {
                val = inst->operands[1].value16;
            } else {
                val = inst->operands[1].value8;
            }
            if (val > 15) {
                fprintf(stderr, "error: BIT operation %u exceeds 4 bits\n", val);
                return false;
            }
            *encoded = (uint16_t)op << 12 | (uint16_t)rd << 8 | (uint16_t)rx << 4 | (uint8_t)val;
            break;
        }
```

To:
```c
        case ASM_INST_BIT: {
            const uint8_t rd = inst->operands[0].reg8;
            const uint8_t rx = inst->operands[2].reg8;
            const char *mnemonic = inst->operands[1].variable;
            uint8_t imm = 0;
            if (strcmp(mnemonic, "AND") == 0) imm = 0x0;
            else if (strcmp(mnemonic, "OR") == 0) imm = 0x1;
            else if (strcmp(mnemonic, "XOR") == 0) imm = 0x2;
            else if (strcmp(mnemonic, "MOD") == 0) imm = 0x4;
            else {
                fprintf(stderr, "error: unknown BIT mnemonic '%s'\n", mnemonic);
                return false;
            }
            *encoded = (uint16_t)op << 12 | (uint16_t)rd << 8 | (uint16_t)rx << 4 | imm;
            break;
        }
```

- [ ] **Step 2: Build to verify**

Run: `cmake --build cmake-build-debug`
Expected: clean build

- [ ] **Step 3: Commit**

```bash
git add src/asm/tree.c
git commit -m "asm: emit BIT with mnemonic-to-value mapping"
```

---

### Task 5: Update free_operands to free mnemonic strings

**Files:**
- Modify: `src/asm/tree.c:22-40` (free_operands function)

**Interfaces:**
- Consumes: new `ASM_OPERAND_MNEMONIC` type
- Produces: proper memory cleanup for mnemonic operands

- [ ] **Step 1: Add mnemonic case to free_operands**

In `src/asm/tree.c`, in the `free_operands` function, add a case for `ASM_OPERAND_MNEMONIC` alongside `ASM_OPERAND_VARIABLE`:

After line 27, add:
```c
        } else if (ops[i].type == ASM_OPERAND_MNEMONIC) {
            free(ops[i].variable);
```

The full conditional block should read:
```c
static void free_operands(ASMProgramInstructionOperand *ops, uint32_t count) {
    if (!ops) return;
    for (uint32_t i = 0; i < count; i++) {
        if (ops[i].type == ASM_OPERAND_VARIABLE) {
            free(ops[i].variable);
        } else if (ops[i].type == ASM_OPERAND_LABEL ||
                   ops[i].type == ASM_OPERAND_LABEL_HIGH ||
                   ops[i].type == ASM_OPERAND_LABEL_LOW) {
            free(ops[i].label);
        } else if (ops[i].type == ASM_OPERAND_MNEMONIC) {
            free(ops[i].variable);
        }
    }
}
```

- [ ] **Step 2: Build to verify**

Run: `cmake --build cmake-build-debug`
Expected: clean build

- [ ] **Step 3: Commit**

```bash
git add src/asm/tree.c
git commit -m "asm: free mnemonic strings in free_operands"
```

---

### Task 6: Update assembler test to use mnemonics

**Files:**
- Modify: `tests/test_bit_asm.c`

**Interfaces:**
- Consumes: new mnemonic parsing from Tasks 2-4
- Produces: same binary output `0x9121` for `BIT R1, AND, R2`

- [ ] **Step 1: Replace numeric token with mnemonic ident token**

In `tests/test_bit_asm.c`, change line 23 from:

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "1", 1, 9));
```

to:

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_IDENT, "AND", 1, 9));
```

- [ ] **Step 2: Run the test**

Run: `cmake --build cmake-build-debug && cmake-build-debug/test_bit_asm`
Expected: PASS with same binary output 0x9121

- [ ] **Step 3: Commit**

```bash
git add tests/test_bit_asm.c
git commit -m "tests: use mnemonics instead of values in BIT assembler test"
```

---

### Task 7: Update ASM.md documentation

**Files:**
- Modify: `ASM.md`

**Interfaces:**
- Consumes: none (documentation only)
- Produces: updated docs showing mnemonic syntax

- [ ] **Step 1: Update the BIT row in the instructions table**

In `ASM.md` line 79, replace:

From:
```
| BIT      | T      | `BIT reg8, value4, reg8`                           |
```

To:
```
| BIT      | T      | `BIT reg8, mnemonic, reg8` (AND, OR, XOR, MOD)    |
```

- [ ] **Step 2: Add BIT usage example**

In the Examples section of ASM.md, add:

```asm
; Bitwise AND: R1 = R1 & R2
        BIT     R1, AND, R2
; Bitwise OR: R1 = R1 | R2
        BIT     R1, OR, R2
; Bitwise XOR: R1 = R1 ^ R2
        BIT     R1, XOR, R2
; Modulo: R1 = R1 % R2
        BIT     R1, MOD, R2
```

- [ ] **Step 3: Commit**

```bash
git add ASM.md
git commit -m "docs: update BIT instruction to use mnemonics"
```

---

### Task 8: Run full test suite

**Files:**
- No file changes

**Interfaces:**
- Consumes: all previous tasks

- [ ] **Step 1: Build and run all tests**

Run: `cmake --build cmake-build-debug && ctest --test-dir cmake-build-debug --output-on-failure`
Expected: all tests pass, including `test_bit_vm` and `test_bit_asm`

- [ ] **Step 2: Run integration test**

Run: `bash tests/test_integration.sh`
Expected: all programs assemble and run successfully

- [ ] **Step 3: Commit final changes if any fixes needed**

```bash
git add -A
git commit -m "test: fix any issues found during full test run"
```
