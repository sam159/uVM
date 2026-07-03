#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "tree.h"

int main(void) {
    AsmTokenList *tokens = asm_token_list_create();

    // HLT R0, 0
    AsmToken t1 = {ASM_TOKEN_OPCODE, strdup("HLT"), 3, 1, 1};
    asm_token_list_append(tokens, t1);
    AsmToken t_reg = {ASM_TOKEN_REGISTER, strdup("R0"), 2, 1, 5};
    asm_token_list_append(tokens, t_reg);
    AsmToken t_comma = {ASM_TOKEN_SYMBOL, strdup(","), 1, 1, 7};
    asm_token_list_append(tokens, t_comma);
    AsmToken t_operand = {ASM_TOKEN_NUMBER, strdup("0"), 1, 1, 9};
    asm_token_list_append(tokens, t_operand);
    AsmToken t2 = {ASM_TOKEN_NEWLINE, strdup(""), 0, 1, 10};
    asm_token_list_append(tokens, t2);

    // EOF
    AsmToken t3 = {ASM_TOKEN_EOF, strdup(""), 0, 2, 1};
    asm_token_list_append(tokens, t3);

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

    if (prog->lines[0]->instruction.op != ASM_INST_HLT) {
        printf("FAIL: expected HLT opcode\n");
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    if (prog->lines[0]->instruction.operand_count != 2) {
        printf("FAIL: expected 2 operands, got %u\n",
               prog->lines[0]->instruction.operand_count);
        asm_free_program(prog);
        asm_token_list_free(tokens);
        return 1;
    }

    printf("PASS: HLT instruction parsed\n");

    asm_free_program(prog);
    asm_token_list_free(tokens);
    return 0;
}
