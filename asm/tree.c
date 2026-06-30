#define _POSIX_C_SOURCE 200809L
#include "tree.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

ASMProgram *asm_parse(const char *filename, AsmTokenList *tokens) {
    ASMProgram *program = (ASMProgram *)calloc(1, sizeof(ASMProgram));
    if (!program) {
        return NULL;
    }

    program->filename = malloc(strlen(filename) + 1);
    if (!program->filename) {
        free(program);
        return NULL;
    }
    strcpy(program->filename, filename);
    program->lines = NULL;
    program->line_count = 0;

    /* TODO: parse tokens into ASMProgramLine entries */
    return program;
}

void asm_free_program(ASMProgram *program) {
    if (!program) {
        return;
    }

    free(program->filename);
    free(program);
}
