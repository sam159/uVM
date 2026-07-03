# HLT Q Format Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Change HLT instruction from V format (12-bit immediate) to Q format (register + 8-bit immediate), where haltCode = register_value + immediate.

**Architecture:** The VM's OP_HLT case switches from vm_decode_V to vm_decode_Q. The assembler's HLT parsing moves from parse_operands_v to parse_operands_q, parsing reg8 and value8 operands. The emit logic encodes as Q format: (op << 12) | (rd << 8) | imm.

**Tech Stack:** C11, CMake, existing test framework (individual C executables in tests/)

## Global Constraints

- haltCode = (uint16_t)vm_get_r(vm, inst.rd) + (uint16_t)inst.imm
- Assembly syntax: `HLT reg8, value8` (both operands required)
- Binary encoding: `0000_rrrr_iiiiiiii` (Q format)
- Breaking change: all existing `HLT value12` must become `HLT reg8, value8`

---

### Task 1: VM - Update HLT to Q Format

**Files:**
- Modify: `vm.c:163-167`
- Test: `tests/test_hlt_q.c` (new)

**Interfaces:**
- Consumes: vm_decode_Q, vm_get_r
- Produces: haltCode computed as register + immediate

- [ ] **Step 1: Write failing test for HLT Q format**

Create `tests/test_hlt_q.c`:

```c
#include <stdio.h>
#include "vm.h"

static uint8_t test_mem[0x10000];

static uint8_t test_read(uint16_t addr, bool instruction) {
    return test_mem[addr];
}

static void test_write(uint16_t addr, uint8_t data) {
    test_mem[addr] = data;
}

int main(void) {
    VM *vm = new_vm();
    vm->readAddr = test_read;
    vm->writeAddr = test_write;

    // Test 1: HLT R0, 0 -> haltCode = 0
    test_mem[0] = 0x00; test_mem[1] = 0x00;  // HLT R0, 0
    vm_reset(vm);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 0) {
        printf("FAIL: HLT R0, 0 expected haltCode=0, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 2: HLT R0, 2 -> haltCode = 2
    test_mem[0] = 0x00; test_mem[1] = 0x02;  // HLT R0, 2
    vm_reset(vm);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 2) {
        printf("FAIL: HLT R0, 2 expected haltCode=2, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 3: HLT R1, 0 with R1=5 -> haltCode = 5
    test_mem[0] = 0x00; test_mem[1] = 0x10;  // HLT R1, 0
    vm_reset(vm);
    vm_put_r(vm, 1, 5);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 5) {
        printf("FAIL: HLT R1, 0 with R1=5 expected haltCode=5, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 4: HLT R1, 0x42 with R1=1 -> haltCode = 0x43
    test_mem[0] = 0x00; test_mem[1] = 0x142 & 0xFF;  // HLT R1, 0x42
    vm_reset(vm);
    vm_put_r(vm, 1, 1);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 0x43) {
        printf("FAIL: HLT R1, 0x42 with R1=1 expected haltCode=0x43, got %u\n", vm->haltCode);
        return 1;
    }

    printf("PASS: HLT Q format tests\n");
    free_vm(vm);
    return 0;
}
```

- [ ] **Step 2: Add test to CMakeLists.txt**

Add to `CMakeLists.txt` in the test section:

```cmake
add_executable(test_hlt_q tests/test_hlt_q.c vm.c)
target_include_directories(test_hlt_q PRIVATE .)
add_test(NAME test_hlt_q COMMAND test_hlt_q)
```

- [ ] **Step 3: Run test to verify it fails**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_hlt_q`
Expected: FAIL (current HLT uses V format, not Q)

- [ ] **Step 4: Update vm.c HLT case**

Modify `vm.c:163-167`:

```c
case OP_HLT:
    vm_decode_Q(&inst, raw);
    vm->halted = true;
    vm->haltCode = (uint16_t)vm_get_r(vm, inst.rd) + (uint16_t)inst.imm;
    break;
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_hlt_q`
Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add vm.c tests/test_hlt_q.c CMakeLists.txt
git commit -m "feat: change HLT from V format to Q format

haltCode = register_value + immediate
Assembly: HLT reg8, value8
Encoding: 0000_rrrr_iiiiiiii"
```

---

### Task 2: Assembler - Update HLT Parsing

**Files:**
- Modify: `asm/tree.c:345-382` (parse_operands_q function)
- Modify: `asm/tree.c:551-577` (parse_operands_v function)
- Modify: `asm/tree.c:929-954` (asm_parse dispatch)

**Interfaces:**
- Consumes: parse_reg8_operand, parse_value_operand, expect_comma
- Produces: HLT instruction with two operands (reg8, value8)

- [ ] **Step 1: Update parse_operands_q to handle HLT**

Modify `asm/tree.c:345-382` to add HLT branch:

```c
static int parse_operands_q(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line) {
    if (opcode == ASM_INST_HLT) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        
        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, tokens->tokens[*pos].col, "expected reg8 for HLT");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
        
        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, tokens->tokens[*pos].col, "expected ',' after reg8");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        
        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, tokens->tokens[*pos].col, "expected value8 for HLT");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        if ((*operands)[1].type == ASM_OPERAND_VARIABLE) {
            parse_error(filename, line, tokens->tokens[*pos].col, "variables not permitted for HLT value8");
            free_operands(*operands, 2); *operands = NULL;
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_JPF) {
        // ... existing JPF code ...
    } else if (opcode == ASM_INST_JNZ) {
        // ... existing JNZ code ...
    }
    
    return 1;
}
```

- [ ] **Step 2: Remove HLT from parse_operands_v**

Modify `asm/tree.c:551-577` to remove the HLT branch:

```c
static int parse_operands_v(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line) {
    *operands = malloc(sizeof(ASMProgramInstructionOperand));
    if (!*operands) return 0;

    // HLT removed - now uses Q format
    if (!parse_rel_operand(tokens, pos, &(*operands)[0])) {
        parse_error(filename, line, tokens->tokens[*pos].col, "expected relative address");
        free_operands(*operands, 0); *operands = NULL;
        return 0;
    }
    *operand_count = 1;
    return 1;
}
```

- [ ] **Step 3: Update asm_parse dispatch**

Modify `asm/tree.c:929-954`:

```c
if (opcode == ASM_INST_JPF || opcode == ASM_INST_JNZ || opcode == ASM_INST_HLT) {
    if (!parse_operands_q(tokens, &pos, opcode, &operands, &operand_count,
                          filename, start_line)) {
        goto fail;
    }
} else if (opcode == ASM_INST_ADD || opcode == ASM_INST_ADC || opcode == ASM_INST_SUB ||
           opcode == ASM_INST_SBC || opcode == ASM_INST_AND) {
    // ... existing S format code ...
} else if (opcode == ASM_INST_LDA || opcode == ASM_INST_STA || opcode == ASM_INST_LDI ||
           opcode == ASM_INST_NOT || opcode == ASM_INST_SHL || opcode == ASM_INST_SHR ||
           opcode == ASM_INST_JPC) {
    // ... existing T format code ...
} else if (opcode == ASM_INST_JMP) {
    if (!parse_operands_v(tokens, &pos, opcode, &operands, &operand_count,
                          filename, start_line)) {
        goto fail;
    }
}
```

- [ ] **Step 4: Build and verify compilation**

Run: `cmake --build cmake-build-debug`
Expected: Compiles without errors

- [ ] **Step 5: Commit**

```bash
git add asm/tree.c
git commit -m "feat: move HLT parsing from V format to Q format"
```

---

### Task 3: Assembler - Update HLT Emission

**Files:**
- Modify: `asm/tree.c:1436-1449` (ASM_INST_HLT emit case)

**Interfaces:**
- Consumes: inst->operands[0].reg8, inst->operands[1].value8
- Produces: Q format encoded instruction

- [ ] **Step 1: Update HLT emit logic**

Modify `asm/tree.c:1436-1449`:

```c
case ASM_INST_HLT: {
    const uint8_t rd = inst->operands[0].reg8;
    uint16_t val;
    if (inst->operands[1].type == ASM_OPERAND_VALUE16) {
        val = inst->operands[1].value16;
    } else {
        val = inst->operands[1].value8;
    }
    if (val > 0xFF) {
        fprintf(stderr, "error: HLT operand %u exceeds 8 bits\n", val);
        return false;
    }
    *encoded = (uint16_t)op << 12 | (uint16_t)rd << 8 | (uint8_t)val;
    break;
}
```

- [ ] **Step 2: Build and verify compilation**

Run: `cmake --build cmake-build-debug`
Expected: Compiles without errors

- [ ] **Step 3: Test with a simple program**

Create `programs/test_hlt.asm`:

```asm
:origin 0
        LDI R1, 5
        HLT R1, 0x10
```

Assemble: `./cmake-build-debug/uVM -c programs/test_hlt.asm -o /tmp/test_hlt.bin`
Run: `./cmake-build-debug/uVM /tmp/test_hlt.bin`
Expected: haltCode = 5 + 16 = 21 (0x15)

- [ ] **Step 4: Commit**

```bash
git add asm/tree.c programs/test_hlt.asm
git commit -m "feat: update HLT emit to Q format encoding"
```

---

### Task 4: Update Existing Programs

**Files:**
- Modify: `programs/basic.asm:26`
- Modify: `programs/word_alignment.asm` (find HLT lines)

- [ ] **Step 1: Update basic.asm**

Change line 26 in `programs/basic.asm`:

```asm
        HLT     R0, 0                           ; halt with exit code 0
```

- [ ] **Step 2: Update word_alignment.asm**

Change line 41:

```asm
        HLT R0, 0
```

Change line 43. The original `HLT :final` uses label 0x011A which exceeds 8 bits. Replace with a simple error code:

```asm
FAIL:   HLT R0, 1
```

- [ ] **Step 3: Run integration tests**

Run: `bash tests/test_integration.sh`
Expected: All programs assemble and run correctly

- [ ] **Step 4: Commit**

```bash
git add programs/
git commit -m "chore: update programs for HLT Q format syntax"
```

---

### Task 5: Update Existing Tests

**Files:**
- Modify: `tests/test_instruction.c`
- Modify: `tests/test_instructions_q.c`
- Modify: `tests/test_emit.c`
- Modify: `tests/test_resolve.c`
- Modify: `tests/test_operand_value.c`
- Modify: `tests/test_dw_resolve.c`
- Modify: `tests/test_dw_label.c`
- Modify: `tests/test_free.c`

Every test that constructs HLT tokens currently provides a single number operand (V format). Each must be updated to provide `R0` (register), `,` (comma), then the number — matching Q format. Operand count assertions change from 1 to 2.

- [ ] **Step 1: Update test_instruction.c**

Replace lines 11-14 (HLT token construction):

```c
    // HLT R0, 0
    AsmToken t1 = {ASM_TOKEN_OPCODE, strdup("HLT"), 3, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t_reg = {ASM_TOKEN_REGISTER, strdup("R0"), 2, 1, 5};
    asm_token_list_append(tokens, t_reg);
    AsmToken t_comma = {ASM_TOKEN_SYMBOL, strdup(","), 1, 1, 7};
    asm_token_list_append(tokens, t_comma);
    AsmToken t_operand = {ASM_TOKEN_NUMBER, strdup("0"), 1, 1, 9};
    asm_token_list_append(tokens, t_operand);
```

Change line 50 operand count assertion from `1` to `2`:

```c
    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: expected 2 operands, got %u\n",
```

- [ ] **Step 2: Update test_instructions_q.c**

Replace lines 20-22 (HLT token construction):

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 10));
```

Change line 32 operand count assertion from `1` to `2`:

```c
    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: HLT expected 2 operands, got %u\n", prog->lines[0]->instruction.operand_count);
```

- [ ] **Step 3: Update test_emit.c — test_hlt_v_format**

Replace lines 19-21 (HLT token construction) in `test_hlt_v_format`:

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "7", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 10));
```

The expected encoding stays `0x00, 0x07` (opcode 0, rd=0 for R0, imm=7).

Replace lines 140-146 (HLT tokens in `test_jmp_label_v_format`):

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 2, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 2, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 10));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, ":end", 3, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 3, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 3, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 3, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 3, 13));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 3, 14));
```

Replace lines 209-211 (HLT tokens in `test_origin_emit`):

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 2, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 2, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 10));
```

- [ ] **Step 4: Update test_resolve.c**

Replace lines 24-25 (HLT in Test 1 — label collection):

```c
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 6));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 1, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 1, 14));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 15));
```

Replace lines 59-60 (HLT in Test 2 — forward reference):

```c
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 2, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 2, 11));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 13));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 14));
```

- [ ] **Step 5: Update test_operand_value.c**

Replace lines 20-22 (HLT token construction):

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "42", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 11));
```

Change line 39 operand count from `1` to `2`:

```c
    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: expected 2 operands, got %u\n", prog->lines[0]->instruction.operand_count);
```

Change lines 46-53 to check the second operand (the value) instead of the first:

```c
    if (prog->lines[0]->instruction.operands[0].type != ASM_OPERAND_REG8) {
        printf("FAIL: expected REG8 first operand\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[1].type != ASM_OPERAND_VALUE8) {
        printf("FAIL: expected VALUE8 second operand\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[1].value8 != 42) {
        printf("FAIL: expected value 42, got %u\n", prog->lines[0]->instruction.operands[1].value8);
```

- [ ] **Step 6: Update test_dw_resolve.c**

Replace lines 35-36 (HLT in Test 1):

```c
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 3, 8));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 3, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 3, 14));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 3, 16));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 3, 17));
```

- [ ] **Step 7: Update test_dw_label.c**

Replace lines 31-32 (HLT in Test 1):

```c
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 8));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 2, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 2, 14));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 16));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 17));
```

- [ ] **Step 8: Update test_free.c**

Replace lines 21-22 (HLT token construction):

```c
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 13));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 1, 15));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 16));
```

Change line 55 operand count from `1` to `2`:

```c
    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: expected 2 operands, got %u\n", prog->lines[0]->instruction.operand_count);
```

- [ ] **Step 9: Build and run all tests**

Run: `cmake --build cmake-build-debug && ctest --test-dir cmake-build-debug --output-on-failure`
Expected: All tests pass

- [ ] **Step 10: Commit**

```bash
git add tests/
git commit -m "test: update tests for HLT Q format"
```

---

### Task 6: Update Documentation

**Files:**
- Modify: `ASM.md:67` (instruction table)
- Modify: `ASM.md:102-103` (relative jumps example)
- Modify: `ASM.md:181` (word alignment example)
- Modify: `ASM.md:229` (full example)

- [ ] **Step 1: Update instruction table**

Change line 67 in `ASM.md`:

```markdown
| HLT      | Q      | `HLT reg8, value8`                                 |
```

- [ ] **Step 2: Update relative jumps example (lines 102-103)**

Change:
```asm
        HLT 1           ; jumped over
        HLT 0
```

To:
```asm
        HLT R0, 1       ; jumped over
        HLT R0, 0
```

- [ ] **Step 3: Update word alignment example (line 181)**

Change:
```asm
HLT 0             ; instruction at address 0x0002 (aligned)
```

To:
```asm
HLT R0, 0         ; instruction at address 0x0002 (aligned)
```

- [ ] **Step 4: Update full example (line 229)**

Change:
```asm
        HLT     0                           ; halt with exit code 0
```

To:
```asm
        HLT     R0, 0                       ; halt with exit code 0
```

- [ ] **Step 5: Commit**

```bash
git add ASM.md
git commit -m "docs: update ASM.md for HLT Q format"
```

---

### Task 7: Final Verification

- [ ] **Step 1: Run full test suite**

Run: `cmake --build cmake-build-debug && ctest --test-dir cmake-build-debug --output-on-failure`
Expected: All tests pass

- [ ] **Step 2: Run integration tests**

Run: `bash tests/test_integration.sh`
Expected: All integration tests pass

- [ ] **Step 3: Manual verification**

Test a few scenarios manually:

```bash
# Test immediate-only halt
echo ':origin 0
        HLT R0, 42' > /tmp/test1.asm
./cmake-build-debug/uVM -c /tmp/test1.asm -o /tmp/test1.bin
./cmake-build-debug/uVM /tmp/test1.bin
# Expected: haltCode = 42

# Test register-only halt
echo ':origin 0
        LDI R2, 100
        HLT R2, 0' > /tmp/test2.asm
./cmake-build-debug/uVM -c /tmp/test2.asm -o /tmp/test2.bin
./cmake-build-debug/uVM /tmp/test2.bin
# Expected: haltCode = 100

# Test combined halt
echo ':origin 0
        LDI R3, 10
        HLT R3, 5' > /tmp/test3.asm
./cmake-build-debug/uVM -c /tmp/test3.asm -o /tmp/test3.bin
./cmake-build-debug/uVM /tmp/test3.bin
# Expected: haltCode = 15
```

- [ ] **Step 4: Create final commit if needed**

If any fixes were needed:

```bash
git add .
git commit -m "fix: final adjustments for HLT Q format"
```
