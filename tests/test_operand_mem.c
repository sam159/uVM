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

    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "LDA", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "RXB", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 8));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, "[", 1, 10));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "RXB", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, "]", 1, 14));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 15));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: expected 2 operands, got %u\n", prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[1].type != ASM_OPERAND_REG16) {
        printf("FAIL: expected REG16 operand\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: memory operand parsed\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
