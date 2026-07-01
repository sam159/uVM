#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

static AsmToken make_token(AsmTokenType type, const char *value, int line, int col) {
    AsmToken t = {0};
    t.type = type;
    t.value_len = strlen(value);
    t.line = line;
    t.col = col;
    t.value = strdup(value);
    return t;
}

int main(void) {
    int failed = 0;

    /* Test 1: label collection */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, ":loop", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 6));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 1, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 11));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [label]: parse returned NULL\n");
            failed = 1;
        } else if (!asm_resolve(prog)) {
            printf("FAIL [label]: resolve failed\n");
            failed = 1;
        } else if (prog->symbol_table.count != 1) {
            printf("FAIL [label]: expected 1 symbol, got %u\n", prog->symbol_table.count);
            failed = 1;
        } else if (strcmp(prog->symbol_table.entries[0].name, "loop") != 0) {
            printf("FAIL [label]: expected name \"loop\", got \"%s\"\n", prog->symbol_table.entries[0].name);
            failed = 1;
        } else if (prog->symbol_table.entries[0].address != 0) {
            printf("FAIL [label]: expected address 0, got %u\n", prog->symbol_table.entries[0].address);
            failed = 1;
        } else {
            printf("PASS [label]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 2: forward reference */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "JMP", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":end", 1, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, ":end", 2, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 3, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [forwardref]: parse returned NULL\n");
            failed = 1;
        } else if (!asm_resolve(prog)) {
            printf("FAIL [forwardref]: resolve failed\n");
            failed = 1;
        } else if (prog->line_count != 2) {
            printf("FAIL [forwardref]: expected 2 lines, got %u\n", prog->line_count);
            failed = 1;
        } else {
            ASMProgramInstruction *inst = &prog->lines[0]->instruction;
            if (inst->op != ASM_INST_JMP) {
                printf("FAIL [forwardref]: expected JMP\n");
                failed = 1;
            } else if (inst->operand_count != 1) {
                printf("FAIL [forwardref]: expected 1 operand, got %u\n", inst->operand_count);
                failed = 1;
            } else if (inst->operands[0].type != ASM_OPERAND_REL) {
                printf("FAIL [forwardref]: expected REL operand, got %d\n", inst->operands[0].type);
                failed = 1;
            } else if (inst->operands[0].rel != 0) {
                printf("FAIL [forwardref]: expected rel=0, got %d\n", inst->operands[0].rel);
                failed = 1;
            } else {
                printf("PASS [forwardref]\n");
            }
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 3: alias only (no unresolved refs) */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_IDENT, "$ACC", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, "=", 1, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R2", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [alias]: parse returned NULL\n");
            failed = 1;
        } else if (!asm_resolve(prog)) {
            printf("FAIL [alias]: resolve failed\n");
            failed = 1;
        } else {
            printf("PASS [alias]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 4: undefined label */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "JMP", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":nope", 1, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [undef_label]: parse returned NULL\n");
            failed = 1;
        } else if (asm_resolve(prog)) {
            printf("FAIL [undef_label]: resolve should have failed\n");
            failed = 1;
        } else {
            printf("PASS [undef_label]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 5: undefined alias */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "LDI", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R1", 1, 5));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_IDENT, "$NOPE", 1, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 14));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [undef_alias]: parse returned NULL\n");
            failed = 1;
        } else if (asm_resolve(prog)) {
            printf("FAIL [undef_alias]: resolve should have failed\n");
            failed = 1;
        } else {
            printf("PASS [undef_alias]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    return failed ? 1 : 0;
}
