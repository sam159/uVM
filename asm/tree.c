#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "token.h"

ASMProgram *asm_parse(const char *filename, AsmTokenList *tokens) {
    ASMProgram *program = calloc(1, sizeof(ASMProgram));
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

    if (program->lines) {
        for (uint32_t i = 0; i < program->line_count; i++) {
            ASMProgramLine *line = program->lines[i];
            if (!line) continue;

            switch (line->type) {
                case ASM_PROGRAM_LINE_INSTRUCTION:
                    free(line->instruction.label);
                    if (line->instruction.operands) {
                        for (uint32_t j = 0; j < line->instruction.operand_count; j++) {
                            ASMProgramInstructionOperand *op = &line->instruction.operands[j];
                            if (op->type == ASM_OPERAND_VARIABLE) {
                                free(op->variable);
                            } else if (op->type == ASM_OPERAND_LABEL ||
                                       op->type == ASM_OPERAND_LABEL_HIGH ||
                                       op->type == ASM_OPERAND_LABEL_LOW) {
                                free(op->label);
                            }
                        }
                        free(line->instruction.operands);
                    }
                    break;
                case ASM_PROGRAM_LINE_DATA:
                    free(line->data.data);
                    break;
                case ASM_PROGRAM_LINE_ALIAS:
                    free(line->alias.variable);
                    break;
                default:
                    break;
            }

            free(line->comment);
            free(line);
        }
        free(program->lines);
    }

    free(program);
}
