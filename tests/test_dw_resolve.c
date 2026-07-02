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

    /* Test 1: DW label resolves to correct address */
    {
        AsmTokenList *tokens = asm_token_list_create();
        /* :data :ptr DW :target */
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":ptr", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DW", 1, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":target", 1, 15));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 23));
        /* :origin 0x100 */
        asm_token_list_append(tokens, make_token(ASM_TOKEN_ORIGIN, ":origin", 2, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0x100", 2, 9));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 14));
        /* target: HLT 0 */
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, "target", 3, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 3, 8));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 3, 12));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 3, 13));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 4, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [dw_resolve]: parse returned NULL\n");
            failed = 1;
        } else if (!asm_resolve(prog)) {
            printf("FAIL [dw_resolve]: resolve failed\n");
            failed = 1;
        } else if (prog->lines[0]->data.value_label != NULL) {
            printf("FAIL [dw_resolve]: value_label should be NULL after resolve\n");
            failed = 1;
        } else if (!prog->lines[0]->data.data) {
            printf("FAIL [dw_resolve]: data should be allocated after resolve\n");
            failed = 1;
        } else if (prog->lines[0]->data.data[0] != 0x01 || prog->lines[0]->data.data[1] != 0x00) {
            printf("FAIL [dw_resolve]: expected 0x0100, got 0x%02x%02x\n",
                   prog->lines[0]->data.data[0], prog->lines[0]->data.data[1]);
            failed = 1;
        } else {
            printf("PASS [dw_resolve]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    /* Test 2: Undefined label in DW fails */
    {
        AsmTokenList *tokens = asm_token_list_create();
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 1, 1));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DW", 1, 7));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":nope", 1, 10));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 16));
        asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

        ASMProgram *prog = asm_parse("test.asm", tokens);
        if (!prog) {
            printf("FAIL [dw_undef]: parse returned NULL\n");
            failed = 1;
        } else if (asm_resolve(prog)) {
            printf("FAIL [dw_undef]: resolve should have failed\n");
            failed = 1;
        } else {
            printf("PASS [dw_undef]\n");
        }
        asm_free_program(prog);
        asm_token_list_free(tokens);
    }

    return failed ? 1 : 0;
}
