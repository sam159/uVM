#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    AsmToken t1 = {ASM_TOKEN_ORIGIN, strdup(":origin"), 7, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t2 = {ASM_TOKEN_NUMBER, strdup("0x100"), 5, 1, 9};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 14};
    asm_token_list_append(tokens, t3);

    AsmToken t4 = {ASM_TOKEN_EOF, strdup(""), 0, 2, 1};
    asm_token_list_append(tokens, t4);

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

    if (prog->lines[0]->type != ASM_PROGRAM_LINE_ORIGIN) {
        printf("FAIL: expected ORIGIN line\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->origin.address != 0x100) {
        printf("FAIL: expected address 0x100, got 0x%x\n", prog->lines[0]->origin.address);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: origin parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
