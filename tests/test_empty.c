#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    // Empty line
    AsmToken t1 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 1};
    asm_token_list_append(tokens, t1);

    // Comment line
    AsmToken t2 = {ASM_TOKEN_COMMENT, strdup("; comment"), 9, 2, 1};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 2, 10};
    asm_token_list_append(tokens, t3);

    // EOF
    AsmToken t4 = {ASM_TOKEN_EOF, strdup(""), 0, 3, 1};
    asm_token_list_append(tokens, t4);

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->line_count != 2) {
        printf("FAIL: expected 2 lines, got %u\n", prog->line_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->type != ASM_PROGRAM_LINE_EMPTY) {
        printf("FAIL: line 0 not empty\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[1]->type != ASM_PROGRAM_LINE_EMPTY) {
        printf("FAIL: line 1 not empty\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: empty lines parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
