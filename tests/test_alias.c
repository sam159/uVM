#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    AsmToken t1 = {ASM_TOKEN_IDENT, strdup("$ACC"), 4, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t2 = {ASM_TOKEN_SYMBOL, strdup("="), 1, 1, 6};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_REGISTER, strdup("R2"), 2, 1, 8};
    asm_token_list_append(tokens, t3);
    AsmToken t4 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 10};
    asm_token_list_append(tokens, t4);

    AsmToken t5 = {ASM_TOKEN_EOF, strdup(""), 0, 2, 1};
    asm_token_list_append(tokens, t5);

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

    if (prog->lines[0]->type != ASM_PROGRAM_LINE_ALIAS) {
        printf("FAIL: expected ALIAS line\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (!prog->lines[0]->alias.variable ||
        strcmp(prog->lines[0]->alias.variable, "$ACC") != 0) {
        printf("FAIL: variable name incorrect\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->alias.value != 2) {
        printf("FAIL: expected value 2, got %ld\n", prog->lines[0]->alias.value);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: alias parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
