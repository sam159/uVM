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

    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "JPC", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R1", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_TEST, "EQ", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R2", 1, 12));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 14));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 3) {
        printf("FAIL: expected 3 operands, got %u\n", prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[1].type != ASM_OPERAND_TEST) {
        printf("FAIL: expected TEST operand\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operands[1].test.type != ASM_TEST_EQ) {
        printf("FAIL: expected EQ test type\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: test operand parsed\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
