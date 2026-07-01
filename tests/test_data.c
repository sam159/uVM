#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    AsmToken t1 = {ASM_TOKEN_DATA, strdup(":data"), 5, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t2 = {ASM_TOKEN_LABEL_REF, strdup(":mybyte"), 8, 1, 7};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_DATA_TYPE, strdup("DB"), 2, 1, 16};
    asm_token_list_append(tokens, t3);
    AsmToken t4 = {ASM_TOKEN_NUMBER, strdup("42"), 2, 1, 19};
    asm_token_list_append(tokens, t4);
    AsmToken t5 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 21};
    asm_token_list_append(tokens, t5);

    AsmToken t6 = {ASM_TOKEN_EOF, strdup(""), 0, 2, 1};
    asm_token_list_append(tokens, t6);

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

    if (prog->lines[0]->type != ASM_PROGRAM_LINE_DATA) {
        printf("FAIL: expected DATA line\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->data.type != ASM_DATA_DB) {
        printf("FAIL: expected DB type\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->data.data_length != 1) {
        printf("FAIL: expected length 1, got %u\n", prog->lines[0]->data.data_length);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->data.data[0] != 42) {
        printf("FAIL: expected value 42, got %u\n", prog->lines[0]->data.data[0]);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: DB data parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
