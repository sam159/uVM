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

    asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, "loop", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 12));
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

    if (prog->lines[0]->type != ASM_PROGRAM_LINE_INSTRUCTION) {
        printf("FAIL: expected INSTRUCTION line\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (!prog->lines[0]->instruction.label ||
        strcmp(prog->lines[0]->instruction.label, "loop") != 0) {
        printf("FAIL: label not set correctly\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: instruction with label parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);

    printf("PASS: free completed without crash\n");
    return 0;
}
