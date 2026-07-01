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

    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "LDI", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R1", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "42", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: LDI parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: LDI expected 2 operands, got %u\n", prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: T format instructions parsed\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
