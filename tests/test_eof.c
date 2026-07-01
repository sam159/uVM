#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();
    AsmToken t1 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 1};
    asm_token_list_append(tokens, t1);

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (prog) {
        printf("FAIL: expected NULL when EOF missing\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    asm_token_list_free(tokens);
    printf("PASS: missing EOF detected\n");

    tokens = asm_token_list_create();
    AsmToken t2 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 1};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_EOF, strdup(""), 0, 2, 1};
    asm_token_list_append(tokens, t3);

    prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: parse failed with EOF present\n");
        asm_token_list_free(tokens);
        return 1;
    }

    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS: parse succeeded with EOF\n");

    return 0;
}
