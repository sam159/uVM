#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    AsmToken t1 = {ASM_TOKEN_DATA, strdup(":data"), 5, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t2 = {ASM_TOKEN_LABEL_REF, strdup(":hexdata"), 9, 1, 7};
    asm_token_list_append(tokens, t2);
    AsmToken t3 = {ASM_TOKEN_DATA_TYPE, strdup("DX"), 2, 1, 17};
    asm_token_list_append(tokens, t3);
    AsmToken t4 = {ASM_TOKEN_NUMBER, strdup("0x010203040506"), 14, 1, 20};
    asm_token_list_append(tokens, t4);
    AsmToken t5 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 34};
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

    if (prog->lines[0]->data.type != ASM_DATA_DX) {
        printf("FAIL: expected DX type\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    // 12 hex digits / 2 = 6 bytes
    if (prog->lines[0]->data.data_length != 6) {
        printf("FAIL: expected length 6, got %u\n", prog->lines[0]->data.data_length);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    uint8_t expected[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    for (uint32_t i = 0; i < 6; i++) {
        if (prog->lines[0]->data.data[i] != expected[i]) {
            printf("FAIL: byte %u expected 0x%02X, got 0x%02X\n",
                   i, expected[i], prog->lines[0]->data.data[i]);
            asm_free_program(prog);
            asm_token_list_free(tokens);
            return 1;
        }
    }

    printf("PASS: DX data parsed\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
