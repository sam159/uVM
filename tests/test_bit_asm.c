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

    asm_token_list_append(tokens, make_token(ASM_TOKEN_OPCODE, "BIT", 1, 1));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R1", 1, 5));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 7));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_IDENT, "OR", 1, 9));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_SYMBOL, ",", 1, 12));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_REGISTER, "R2", 1, 14));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_NEWLINE, "", 1, 14));
    asm_token_list_append(tokens, make_token(ASM_TOKEN_EOF, "", 2, 1));

    ASMProgram *prog = asm_parse("test.asm", tokens);
    if (!prog) {
        printf("FAIL: BIT parse returned NULL\n");
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.op != 9) {
        printf("FAIL: BIT expected opcode 9, got %u\n", prog->lines[0]->instruction.op);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 3) {
        printf("FAIL: BIT expected 3 operands, got %u\n", prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (!asm_resolve(prog)) {
        printf("FAIL: BIT resolve failed\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    FILE *f = tmpfile();
    uint16_t offset = 0;
    if (!asm_emit(prog, f, &offset)) {
        printf("FAIL: BIT emit failed\n");
        fclose(f);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    rewind(f);
    uint8_t buf[2];
    fread(buf, 1, 2, f);
    fclose(f);

    if (buf[0] != 0x91 || buf[1] != 0x21) {
        printf("FAIL: BIT encoding expected 0x9121, got 0x%02X%02X\n", buf[0], buf[1]);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: BIT assembler tests\n");
    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
