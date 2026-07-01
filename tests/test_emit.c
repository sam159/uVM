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

static int test_hlt_v_format(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "7", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 6));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [hlt]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [hlt]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [hlt]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    if (buf[0] != 0x00 || buf[1] != 0x07) {
        printf("FAIL [hlt]: got %02x %02x, expected 00 07\n", buf[0], buf[1]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [hlt]\n");
    return 0;
}

static int test_ldi_q_format(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "LDI", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R1", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "42", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [ldi]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [ldi]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [ldi]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    if (buf[0] != 0x31 || buf[1] != 0x2A) {
        printf("FAIL [ldi]: got %02x %02x, expected 31 2A\n", buf[0], buf[1]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [ldi]\n");
    return 0;
}

static int test_add_s_format(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "ADD", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R2", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R3", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 11));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R4", 1, 13));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 15));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [add]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [add]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [add]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    if (buf[0] != 0x42 || buf[1] != 0x34) {
        printf("FAIL [add]: got %02x %02x, expected 42 34\n", buf[0], buf[1]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [add]\n");
    return 0;
}

static int test_not_t_format(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "NOT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R5", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [not]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [not]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [not]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    if (buf[0] != 0x85 || buf[1] != 0x50) {
        printf("FAIL [not]: got %02x %02x, expected 85 50\n", buf[0], buf[1]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [not]\n");
    return 0;
}

static int test_jmp_label_v_format(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "JMP", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":end", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 6));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL, ":end", 3, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 3, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 3, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 3, 10));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 4, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [jmp_label]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [jmp_label]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [jmp_label]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[6];
    fread(buf, 1, 6, f);
    if (buf[0] != 0xC0 || buf[1] != 0x01 || buf[2] != 0x00 || buf[3] != 0x00 || buf[4] != 0x00 || buf[5] != 0x00) {
        printf("FAIL [jmp_label]: got %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [jmp_label]\n");
    return 0;
}

static int test_data_emit(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_LABEL_REF, ":mydata", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DB", 1, 15));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "42", 1, 18));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 20));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA, ":data", 2, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_DATA_TYPE, "DW", 2, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0x1234", 2, 10));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 16));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 3, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [data]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [data]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [data]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    uint8_t buf[3];
    fread(buf, 1, 3, f);
    if (buf[0] != 42 || buf[1] != 0x12 || buf[2] != 0x34) {
        printf("FAIL [data]: got %02x %02x %02x, expected 2A 12 34\n", buf[0], buf[1], buf[2]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [data]\n");
    return 0;
}

static int test_origin_emit(void) {
    AsmTokenList *tokens = asm_token_list_create();
    asm_token_list_append(tokens, make_token(ASM_TOKEN_ORIGIN, ":origin", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0x100", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 14));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "HLT", 2, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NUMBER, "0", 2, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 2, 6));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 3, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) { printf("FAIL [origin]: parse NULL\n"); asm_token_list_free(tokens); return 1; }
    if (!asm_resolve(prog)) { printf("FAIL [origin]: resolve\n"); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    FILE *f = tmpfile();
    if (!asm_emit(prog, f)) { printf("FAIL [origin]: emit\n"); fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1; }

    rewind(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    if (size != 258) {
        printf("FAIL [origin]: expected 258 bytes, got %ld\n", size);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fseek(f, 256, SEEK_SET);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    if (buf[0] != 0x00 || buf[1] != 0x00) {
        printf("FAIL [origin]: last bytes %02x %02x, expected 00 00\n", buf[0], buf[1]);
        fclose(f); asm_free_program(prog); asm_token_list_free(tokens); return 1;
    }
    fclose(f);
    asm_free_program(prog);
    asm_token_list_free(tokens);
    printf("PASS [origin]\n");
    return 0;
}

int main(void) {
    int failed = 0;
    failed += test_hlt_v_format();
    failed += test_ldi_q_format();
    failed += test_add_s_format();
    failed += test_not_t_format();
    failed += test_jmp_label_v_format();
    failed += test_data_emit();
    failed += test_origin_emit();
    return failed ? 1 : 0;
}
