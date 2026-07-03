#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

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
    int failed = 0;

    /* Test 1: DW with label reference parses correctly */
    {
        AsmTokenList *tokens = asm_token_list_create();
        /* :data :ptr DW :target */
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":ptr", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DW", 1, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":target", 1, 15));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 23));
        /* target: HLT 0 */
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, "target", 2, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 8));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R0", 2, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 2, 14));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 16));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 17));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 3, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [dw_label_parse]: parse returned NULL\n");
            failed = 1;
        } else if (prog->line_count != 2) {
            printf("FAIL [dw_label_parse]: expected 2 lines, got %u\n", prog->line_count);
            failed = 1;
        } else if (prog->lines[0]->type != ASM_PROGRAM_LINE_DATA) {
            printf("FAIL [dw_label_parse]: expected DATA line\n");
            failed = 1;
        } else if (prog->lines[0]->data.type != ASM_DATA_DW) {
            printf("FAIL [dw_label_parse]: expected DW type\n");
            failed = 1;
        } else if (!prog->lines[0]->data.value_label) {
            printf("FAIL [dw_label_parse]: expected value_label to be set\n");
            failed = 1;
        } else if (strcmp(prog->lines[0]->data.value_label, "target") != 0) {
            printf("FAIL [dw_label_parse]: expected value_label \"target\", got \"%s\"\n",
                   prog->lines[0]->data.value_label);
            failed = 1;
        } else if (prog->lines[0]->data.data_length != 2) {
            printf("FAIL [dw_label_parse]: expected data_length 2, got %u\n",
                   prog->lines[0]->data.data_length);
            failed = 1;
        } else {
            printf("PASS [dw_label_parse]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 2: DW with numeric value still works */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DW", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0x1234", 1, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 16));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [dw_numeric]: parse returned NULL\n");
            failed = 1;
        } else if (prog->lines[0]->data.value_label != NULL) {
            printf("FAIL [dw_numeric]: expected value_label to be NULL\n");
            failed = 1;
        } else if (prog->lines[0]->data.data_length != 2) {
            printf("FAIL [dw_numeric]: expected data_length 2, got %u\n",
                   prog->lines[0]->data.data_length);
            failed = 1;
        } else if (prog->lines[0]->data.data[0] != 0x12 || prog->lines[0]->data.data[1] != 0x34) {
            printf("FAIL [dw_numeric]: expected 0x1234, got 0x%02x%02x\n",
                   prog->lines[0]->data.data[0], prog->lines[0]->data.data[1]);
            failed = 1;
        } else {
            printf("PASS [dw_numeric]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    return failed ? 1 : 0;
}
