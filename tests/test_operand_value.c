#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "token.h"

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
    AsmTokenList *tokens = asm_token_list_create();

    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "42", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->line_count != 1) {
        printf("FAIL: expected 1 line, got %u\n", prog->line_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 1) {
        printf("FAIL: expected 1 operand, got %u\n", prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[0].type != ASM_OPERAND_VALUE16) {
        printf("FAIL: expected VALUE16 operand\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[0].value16 != 42) {
        printf("FAIL: expected value 42, got %u\n", prog->lines[0]->instruction.operands[0].value16);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: value operand parsed\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
