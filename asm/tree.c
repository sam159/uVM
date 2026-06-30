#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "token.h"

static void parse_error(const char *filename, int line, int col, const char *msg) {
    fprintf(stderr, "%s:%d:%d: error: %s\n", filename, line, col, msg);
}

static int program_append_line(ASMProgram *program, ASMProgramLine *line) {
    ASMProgramLine **tmp = realloc(program->lines,
                                   sizeof(ASMProgramLine*) * (program->line_count + 1));
    if (!tmp) return 0;
    program->lines = tmp;
    program->lines[program->line_count++] = line;
    return 1;
}

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

    size_t pos = 0;
    uint16_t current_address = 0;

    while (pos < tokens->count) {
        AsmToken *tok = &tokens->tokens[pos];

        if (tok->type == ASM_TOKEN_WHITESPACE) {
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_EOF) {
            break;
        }

        if (tok->type == ASM_TOKEN_NEWLINE) {
            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_EMPTY;
            line->line_number = tok->line;
            line->address = current_address;
            if (!program_append_line(program, line)) {
                free(line);
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_COMMENT) {
            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_EMPTY;
            line->line_number = tok->line;
            line->address = current_address;
            line->comment = malloc(tok->value_len + 1);
            if (line->comment) {
                strcpy(line->comment, tok->value);
            }
            if (!program_append_line(program, line)) {
                free(line->comment);
                free(line);
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            pos++;
            while (pos < tokens->count && tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                pos++;
            }
            if (pos < tokens->count) pos++;
            continue;
        }

        parse_error(filename, tok->line, tok->col, "unexpected token");
        asm_free_program(program);
        return NULL;
    }

    if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_EOF) {
        parse_error(filename, 0, 0, "unexpected end of input, expected EOF");
        asm_free_program(program);
        return NULL;
    }

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
