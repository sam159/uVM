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

static int parse_number(const char *s, int64_t *out) {
    if (!s || !out) return 0;
    if (*s == '+') s++;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        char *end;
        *out = strtoll(s, &end, 16);
        return *end == '\0';
    } else if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
        char *end;
        *out = strtoll(s + 2, &end, 2);
        return *end == '\0';
    } else {
        char *end;
        *out = strtoll(s, &end, 10);
        return *end == '\0';
    }
}

static int parse_register_id(const char *s) {
    if (!s) return -1;

    if (s[0] == 'R') {
        if (s[1] == 'Z') return 0;
        if (s[1] >= '0' && s[1] <= '9') return s[1] - '0';
        if (s[1] >= 'A' && s[1] <= 'F') return 10 + (s[1] - 'A');
    }

    if (s[0] == 'R' && s[1] == 'X') {
        if (s[2] == 'Z') return 0;
        if (s[2] >= '0' && s[2] <= '9') return s[2] - '0';
        if (s[2] >= 'A' && s[2] <= 'F') return 10 + (s[2] - 'A');
    }

    return -1;
}

static int parse_reg8_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type == ASM_TOKEN_REGISTER) {
        if (tok->value[0] == 'R' && tok->value[1] == 'X') {
            return 0;
        }
        int reg_id = parse_register_id(tok->value);
        if (reg_id < 0 || reg_id > 15) {
            return 0;
        }
        op->type = ASM_OPERAND_REG8;
        op->reg8 = (uint8_t)reg_id;
        (*pos)++;
        return 1;
    } else if (tok->type == ASM_TOKEN_IDENT && tok->value[0] == '$') {
        op->type = ASM_OPERAND_VARIABLE;
        op->variable = malloc(tok->value_len + 1);
        if (!op->variable) return 0;
        strcpy(op->variable, tok->value);
        (*pos)++;
        return 1;
    }

    return 0;
}

static int parse_reg16_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type != ASM_TOKEN_REGISTER) return 0;

    if (tok->value[0] == 'R' && tok->value[1] == 'X') {
        int reg_id = parse_register_id(tok->value);
        if (reg_id >= 0) {
            op->type = ASM_OPERAND_REG16;
            op->reg8 = (uint8_t)reg_id;
            (*pos)++;
            return 1;
        }
    }

    return 0;
}

static int parse_value_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type == ASM_TOKEN_NUMBER) {
        int64_t val;
        if (!parse_number(tok->value, &val)) {
            return 0;
        }
        op->type = ASM_OPERAND_VALUE16;
        op->value16 = (uint16_t)val;
        (*pos)++;
        return 1;
    } else if (tok->type == ASM_TOKEN_IDENT && tok->value[0] == '$') {
        op->type = ASM_OPERAND_VARIABLE;
        op->variable = malloc(tok->value_len + 1);
        if (!op->variable) return 0;
        strcpy(op->variable, tok->value);
        (*pos)++;
        return 1;
    } else if (tok->type == ASM_TOKEN_LABEL_REF) {
        const char *ref = tok->value;
        size_t len = tok->value_len;

        if (len >= 4 && ref[len-1] == 'h' && ref[len-2] == ':') {
            op->type = ASM_OPERAND_LABEL_HIGH;
            op->label = malloc(len - 2);
            if (!op->label) return 0;
            strncpy(op->label, ref + 1, len - 3);
            op->label[len - 3] = '\0';
        } else if (len >= 4 && ref[len-1] == 'l' && ref[len-2] == ':') {
            op->type = ASM_OPERAND_LABEL_LOW;
            op->label = malloc(len - 2);
            if (!op->label) return 0;
            strncpy(op->label, ref + 1, len - 3);
            op->label[len - 3] = '\0';
        } else {
            op->type = ASM_OPERAND_LABEL;
            op->label = malloc(len);
            if (!op->label) return 0;
            strncpy(op->label, ref + 1, len - 1);
            op->label[len - 1] = '\0';
        }
        (*pos)++;
        return 1;
    }

    return 0;
}

static int parse_rel_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type == ASM_TOKEN_LABEL_REF) {
        op->type = ASM_OPERAND_LABEL;
        op->label = malloc(tok->value_len);
        if (!op->label) return 0;
        strncpy(op->label, tok->value + 1, tok->value_len - 1);
        op->label[tok->value_len - 1] = '\0';
        (*pos)++;
        return 1;
    }

    int sign = 1;
    if (tok->type == ASM_TOKEN_SYMBOL) {
        if (tok->value[0] == '+') {
            sign = 1;
        } else if (tok->value[0] == '-') {
            sign = -1;
        } else {
            return 0;
        }
        (*pos)++;
        tok = &tokens->tokens[*pos];
    }

    if (tok->type == ASM_TOKEN_NUMBER) {
        int64_t val;
        if (!parse_number(tok->value, &val)) {
            return 0;
        }
        op->type = ASM_OPERAND_REL;
        op->rel = (int16_t)(sign * val);
        (*pos)++;
        return 1;
    }

    return 0;
}

static int parse_mem_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != '[') {
        return 0;
    }
    (*pos)++;

    tok = &tokens->tokens[*pos];
    if (tok->type != ASM_TOKEN_REGISTER || tok->value[0] != 'R' || tok->value[1] != 'X') {
        return 0;
    }

    int reg_id = parse_register_id(tok->value);
    if (reg_id < 0) {
        return 0;
    }
    (*pos)++;

    tok = &tokens->tokens[*pos];
    if (tok->type == ASM_TOKEN_SYMBOL && tok->value[0] == ']') {
        op->type = ASM_OPERAND_REG16;
        op->reg8 = (uint8_t)reg_id;
        (*pos)++;
        return 1;
    }

    if (tok->type == ASM_TOKEN_SYMBOL && tok->value[0] == '+') {
        (*pos)++;
        tok = &tokens->tokens[*pos];

        int8_t offset = 0;
        if (tok->type == ASM_TOKEN_NUMBER) {
            int64_t val;
            if (!parse_number(tok->value, &val) || val < -128 || val > 127) {
                return 0;
            }
            offset = (int8_t)val;
            (*pos)++;
        } else if (tok->type == ASM_TOKEN_LABEL_REF) {
            op->type = ASM_OPERAND_LABEL;
            op->label = malloc(tok->value_len);
            if (!op->label) return 0;
            strncpy(op->label, tok->value + 1, tok->value_len - 1);
            op->label[tok->value_len - 1] = '\0';
            (*pos)++;

            tok = &tokens->tokens[*pos];
            if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ']') {
                return 0;
            }
            (*pos)++;
            return 1;
        } else {
            return 0;
        }

        tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ']') {
            return 0;
        }

        op->type = ASM_OPERAND_REG16_REL;
        op->reg16_rel.register_id = (uint8_t)reg_id;
        op->reg16_rel.offset = offset;
        (*pos)++;
        return 1;
    }

    return 0;
}

static int parse_test_operand(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionOperand *op) {
    AsmToken *tok = &tokens->tokens[*pos];

    bool negated = false;
    if (tok->type == ASM_TOKEN_SYMBOL && tok->value[0] == '~') {
        negated = true;
        (*pos)++;
        tok = &tokens->tokens[*pos];
    }

    if (tok->type != ASM_TOKEN_TEST) {
        return 0;
    }

    op->type = ASM_OPERAND_TEST;
    op->test.negated = negated;

    if (strcmp(tok->value, "EQ") == 0) {
        op->test.type = ASM_TEST_EQ;
    } else if (strcmp(tok->value, "LT") == 0) {
        op->test.type = ASM_TEST_LT;
    } else if (strcmp(tok->value, "LTE") == 0) {
        op->test.type = ASM_TEST_LTE;
    } else if (strcmp(tok->value, "GT") == 0) {
        op->test.type = ASM_TEST_GT;
    } else if (strcmp(tok->value, "GTE") == 0) {
        op->test.type = ASM_TEST_GTE;
    } else {
        return 0;
    }

    (*pos)++;
    return 1;
}

static int opcode_from_string(const char *s, ASMProgramInstructionType *op) {
    if (strcmp(s, "HLT") == 0) { *op = ASM_INST_HLT; return 1; }
    if (strcmp(s, "LDA") == 0) { *op = ASM_INST_LDA; return 1; }
    if (strcmp(s, "STA") == 0) { *op = ASM_INST_STA; return 1; }
    if (strcmp(s, "LDI") == 0) { *op = ASM_INST_LDI; return 1; }
    if (strcmp(s, "ADD") == 0) { *op = ASM_INST_ADD; return 1; }
    if (strcmp(s, "ADC") == 0) { *op = ASM_INST_ADC; return 1; }
    if (strcmp(s, "SUB") == 0) { *op = ASM_INST_SUB; return 1; }
    if (strcmp(s, "SBC") == 0) { *op = ASM_INST_SBC; return 1; }
    if (strcmp(s, "NOT") == 0) { *op = ASM_INST_NOT; return 1; }
    if (strcmp(s, "AND") == 0) { *op = ASM_INST_AND; return 1; }
    if (strcmp(s, "SHL") == 0) { *op = ASM_INST_SHL; return 1; }
    if (strcmp(s, "SHR") == 0) { *op = ASM_INST_SHR; return 1; }
    if (strcmp(s, "JMP") == 0) { *op = ASM_INST_JMP; return 1; }
    if (strcmp(s, "JPF") == 0) { *op = ASM_INST_JPF; return 1; }
    if (strcmp(s, "JNZ") == 0) { *op = ASM_INST_JNZ; return 1; }
    if (strcmp(s, "JPC") == 0) { *op = ASM_INST_JPC; return 1; }
    return 0;
}

static int parse_operands_q(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line, int col) {
    if (opcode == ASM_INST_HLT) {
        *operands = malloc(sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        if (!parse_value_operand(tokens, pos, *operands)) {
            parse_error(filename, line, col, "expected value12 for HLT");
            free(*operands);
            return 0;
        }
        *operand_count = 1;
    } else if (opcode == ASM_INST_JPF) {
        *operands = malloc(sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        if (!parse_mem_operand(tokens, pos, *operands)) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel] for JPF");
            free(*operands);
            return 0;
        }
        *operand_count = 1;
    } else if (opcode == ASM_INST_JNZ) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        
        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8 for JNZ");
            free(*operands);
            return 0;
        }
        
        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ',' after reg8");
            free(*operands);
            return 0;
        }
        (*pos)++;
        
        if (!parse_rel_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected relative address for JNZ");
            free(*operands);
            return 0;
        }
        *operand_count = 2;
    }
    
    return 1;
}

static int parse_operands_s(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line, int col) {
    *operands = malloc(3 * sizeof(ASMProgramInstructionOperand));
    if (!*operands) return 0;

    for (int i = 0; i < 3; i++) {
        if (!parse_reg8_operand(tokens, pos, &(*operands)[i])) {
            parse_error(filename, line, col, "expected reg8");
            for (int j = 0; j < i; j++) {
                if ((*operands)[j].type == ASM_OPERAND_VARIABLE) {
                    free((*operands)[j].variable);
                }
            }
            free(*operands);
            return 0;
        }

        if (i < 2) {
            AsmToken *tok = &tokens->tokens[*pos];
            if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
                parse_error(filename, line, col, "expected ','");
                for (int j = 0; j <= i; j++) {
                    if ((*operands)[j].type == ASM_OPERAND_VARIABLE) {
                        free((*operands)[j].variable);
                    }
                }
                free(*operands);
                return 0;
            }
            (*pos)++;
        }
    }

    *operand_count = 3;
    return 1;
}

static int parse_operands_t(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line, int col) {
    if (opcode == ASM_INST_LDA) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg16_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg16 for LDA");
            free(*operands);
            return 0;
        }

        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_mem_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel]");
            free(*operands);
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_STA) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_mem_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel]");
            free(*operands);
            return 0;
        }

        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_reg16_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected reg16");
            free(*operands);
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_LDI) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free(*operands);
            return 0;
        }

        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected value8");
            free(*operands);
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_NOT) {
        *operands = malloc(sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free(*operands);
            return 0;
        }
        *operand_count = 1;
    } else if (opcode == ASM_INST_SHL || opcode == ASM_INST_SHR) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free(*operands);
            return 0;
        }

        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected value4");
            free(*operands);
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_JPC) {
        *operands = malloc(3 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free(*operands);
            return 0;
        }

        AsmToken *tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_test_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected test");
            free(*operands);
            return 0;
        }

        tok = &tokens->tokens[*pos];
        if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') {
            parse_error(filename, line, col, "expected ','");
            free(*operands);
            return 0;
        }
        (*pos)++;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[2])) {
            parse_error(filename, line, col, "expected reg8");
            free(*operands);
            return 0;
        }
        *operand_count = 3;
    }

    return 1;
}

static int parse_operands_v(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line, int col) {
    *operands = malloc(sizeof(ASMProgramInstructionOperand));
    if (!*operands) return 0;

    if (!parse_rel_operand(tokens, pos, &(*operands)[0])) {
        parse_error(filename, line, col, "expected relative address");
        free(*operands);
        return 0;
    }
    *operand_count = 1;
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

        if (tok->type == ASM_TOKEN_ORIGIN) {
            pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                parse_error(filename, tok->line, tok->col, "expected number after :origin");
                asm_free_program(program);
                return NULL;
            }
            int64_t addr;
            if (!parse_number(tokens->tokens[pos].value, &addr) || addr < 0 || addr > 0xFFFF) {
                parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                           "invalid origin address");
                asm_free_program(program);
                return NULL;
            }
            current_address = (uint16_t)addr;
            pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, tok->line, tok->col, "expected newline after :origin");
                asm_free_program(program);
                return NULL;
            }
            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_ORIGIN;
            line->line_number = tok->line;
            line->address = current_address;
            line->origin.address = current_address;
            if (!program_append_line(program, line)) {
                free(line);
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_IDENT && tok->value[0] == '$') {
            int start_line = tok->line;
            int start_col = tok->col;

            char *var_name = malloc(tok->value_len + 1);
            if (!var_name) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                asm_free_program(program);
                return NULL;
            }
            strcpy(var_name, tok->value);
            pos++;

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_SYMBOL ||
                tokens->tokens[pos].value[0] != '=') {
                parse_error(filename, start_line, start_col, "expected '=' after alias name");
                free(var_name);
                asm_free_program(program);
                return NULL;
            }
            pos++;

            if (pos >= tokens->count) {
                parse_error(filename, start_line, start_col, "expected value after '='");
                free(var_name);
                asm_free_program(program);
                return NULL;
            }

            int64_t value = 0;
            if (tokens->tokens[pos].type == ASM_TOKEN_NUMBER) {
                if (!parse_number(tokens->tokens[pos].value, &value)) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "invalid number");
                    free(var_name);
                    asm_free_program(program);
                    return NULL;
                }
            } else if (tokens->tokens[pos].type == ASM_TOKEN_REGISTER) {
                int reg_id = parse_register_id(tokens->tokens[pos].value);
                if (reg_id < 0) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "invalid register");
                    free(var_name);
                    asm_free_program(program);
                    return NULL;
                }
                value = reg_id;
            } else {
                parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                           "expected number or register");
                free(var_name);
                asm_free_program(program);
                return NULL;
            }
            pos++;

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after alias");
                free(var_name);
                asm_free_program(program);
                return NULL;
            }

            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, start_line, start_col, "out of memory");
                free(var_name);
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_ALIAS;
            line->line_number = start_line;
            line->address = current_address;
            line->alias.variable = var_name;
            line->alias.value = value;

            if (!program_append_line(program, line)) {
                free(var_name);
                free(line);
                parse_error(filename, start_line, start_col, "out of memory");
                asm_free_program(program);
                return NULL;
            }

            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_DATA) {
            int start_line = tok->line;
            int start_col = tok->col;
            pos++;

            char *data_label = NULL;
            if (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_LABEL_REF) {
                data_label = malloc(tokens->tokens[pos].value_len + 1);
                if (!data_label) {
                    parse_error(filename, start_line, start_col, "out of memory");
                    asm_free_program(program);
                    return NULL;
                }
                strcpy(data_label, tokens->tokens[pos].value);
                pos++;
            }

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_DATA_TYPE) {
                parse_error(filename, start_line, start_col, "expected data type");
                free(data_label);
                asm_free_program(program);
                return NULL;
            }

            ASMProgramDataType dtype;
            if (strcmp(tokens->tokens[pos].value, "DB") == 0) {
                dtype = ASM_DATA_DB;
            } else if (strcmp(tokens->tokens[pos].value, "DW") == 0) {
                dtype = ASM_DATA_DW;
            } else if (strcmp(tokens->tokens[pos].value, "DX") == 0) {
                dtype = ASM_DATA_DX;
            } else if (strcmp(tokens->tokens[pos].value, "STZ") == 0) {
                dtype = ASM_DATA_STZ;
            } else if (strcmp(tokens->tokens[pos].value, "STL") == 0) {
                dtype = ASM_DATA_STL;
            } else {
                parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                           "unknown data type");
                free(data_label);
                asm_free_program(program);
                return NULL;
            }
            pos++;

            uint8_t *data_buf = NULL;
            uint32_t data_len = 0;

            if (dtype == ASM_DATA_DB) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                    parse_error(filename, start_line, start_col, "expected number for DB");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                int64_t val;
                if (!parse_number(tokens->tokens[pos].value, &val) || val < 0 || val > 255) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "DB value out of range");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                data_buf = malloc(1);
                if (!data_buf) {
                    parse_error(filename, start_line, start_col, "out of memory");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                data_buf[0] = (uint8_t)val;
                data_len = 1;
                pos++;
            } else if (dtype == ASM_DATA_DW) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                    parse_error(filename, start_line, start_col, "expected number for DW");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                int64_t val;
                if (!parse_number(tokens->tokens[pos].value, &val) || val < 0 || val > 0xFFFF) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "DW value out of range");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                data_buf = malloc(2);
                if (!data_buf) {
                    parse_error(filename, start_line, start_col, "out of memory");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }
                data_buf[0] = (uint8_t)(val >> 8);
                data_buf[1] = (uint8_t)val;
                data_len = 2;
                pos++;
            } else if (dtype == ASM_DATA_STZ || dtype == ASM_DATA_STL) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_STRING) {
                    parse_error(filename, start_line, start_col, "expected string");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }

                const char *str = tokens->tokens[pos].value;
                size_t str_len = tokens->tokens[pos].value_len;

                size_t needed = str_len;
                if (dtype == ASM_DATA_STZ) {
                    needed += 1;
                } else {
                    needed += 2;
                }

                data_buf = malloc(needed);
                if (!data_buf) {
                    parse_error(filename, start_line, start_col, "out of memory");
                    free(data_label);
                    asm_free_program(program);
                    return NULL;
                }

                if (dtype == ASM_DATA_STL) {
                    data_buf[0] = (uint8_t)(str_len >> 8);
                    data_buf[1] = (uint8_t)str_len;
                    memcpy(data_buf + 2, str, str_len);
                    data_len = 2 + str_len;
                } else {
                    memcpy(data_buf, str, str_len);
                    data_buf[str_len] = 0;
                    data_len = str_len + 1;
                }
                pos++;
            } else {
                parse_error(filename, start_line, start_col, "DX not yet implemented");
                free(data_label);
                asm_free_program(program);
                return NULL;
            }

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after :data");
                free(data_buf);
                free(data_label);
                asm_free_program(program);
                return NULL;
            }

            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, start_line, start_col, "out of memory");
                free(data_buf);
                free(data_label);
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_DATA;
            line->line_number = start_line;
            line->address = current_address;
            line->data.label = data_label;
            line->data.type = dtype;
            line->data.data = data_buf;
            line->data.data_length = data_len;

            current_address += data_len;

            if (!program_append_line(program, line)) {
                free(data_buf);
                free(data_label);
                free(line);
                parse_error(filename, start_line, start_col, "out of memory");
                asm_free_program(program);
                return NULL;
            }

            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_LABEL || tok->type == ASM_TOKEN_OPCODE) {
            int start_line = tok->line;
            int start_col = tok->col;

            char *label = NULL;

            if (tok->type == ASM_TOKEN_LABEL) {
                label = malloc(tok->value_len + 1);
                if (!label) {
                    parse_error(filename, tok->line, tok->col, "out of memory");
                    asm_free_program(program);
                    return NULL;
                }
                strcpy(label, tok->value);
                pos++;

                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_OPCODE) {
                    parse_error(filename, start_line, start_col, "expected opcode after label");
                    free(label);
                    asm_free_program(program);
                    return NULL;
                }
                tok = &tokens->tokens[pos];
            }

            ASMProgramInstructionType opcode;
            if (!opcode_from_string(tok->value, &opcode)) {
                parse_error(filename, tok->line, tok->col, "unknown opcode");
                free(label);
                asm_free_program(program);
                return NULL;
            }
            pos++;

            ASMProgramInstructionOperand *operands = NULL;
            uint32_t operand_count = 0;

            if (opcode == ASM_INST_HLT || opcode == ASM_INST_JPF || opcode == ASM_INST_JNZ) {
                if (!parse_operands_q(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    free(label);
                    asm_free_program(program);
                    return NULL;
                }
            } else if (opcode == ASM_INST_ADD || opcode == ASM_INST_ADC || opcode == ASM_INST_SUB ||
                       opcode == ASM_INST_SBC || opcode == ASM_INST_AND) {
                if (!parse_operands_s(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    free(label);
                    asm_free_program(program);
                    return NULL;
                }
            } else if (opcode == ASM_INST_LDA || opcode == ASM_INST_STA || opcode == ASM_INST_LDI ||
                       opcode == ASM_INST_NOT || opcode == ASM_INST_SHL || opcode == ASM_INST_SHR ||
                       opcode == ASM_INST_JPC) {
                if (!parse_operands_t(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    free(label);
                    asm_free_program(program);
                    return NULL;
                }
            } else if (opcode == ASM_INST_JMP) {
                if (!parse_operands_v(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    free(label);
                    asm_free_program(program);
                    return NULL;
                }
            }

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after instruction");
                free(label);
                if (operands) {
                    for (uint32_t i = 0; i < operand_count; i++) {
                        if (operands[i].type == ASM_OPERAND_VARIABLE) {
                            free(operands[i].variable);
                        } else if (operands[i].type == ASM_OPERAND_LABEL ||
                                   operands[i].type == ASM_OPERAND_LABEL_HIGH ||
                                   operands[i].type == ASM_OPERAND_LABEL_LOW) {
                            free(operands[i].label);
                        }
                    }
                    free(operands);
                }
                asm_free_program(program);
                return NULL;
            }

            ASMProgramLine *line = calloc(1, sizeof(ASMProgramLine));
            if (!line) {
                parse_error(filename, start_line, start_col, "out of memory");
                free(label);
                if (operands) {
                    for (uint32_t i = 0; i < operand_count; i++) {
                        if (operands[i].type == ASM_OPERAND_VARIABLE) {
                            free(operands[i].variable);
                        } else if (operands[i].type == ASM_OPERAND_LABEL ||
                                   operands[i].type == ASM_OPERAND_LABEL_HIGH ||
                                   operands[i].type == ASM_OPERAND_LABEL_LOW) {
                            free(operands[i].label);
                        }
                    }
                    free(operands);
                }
                asm_free_program(program);
                return NULL;
            }
            line->type = ASM_PROGRAM_LINE_INSTRUCTION;
            line->line_number = start_line;
            line->address = current_address;
            line->instruction.label = label;
            line->instruction.op = opcode;
            line->instruction.operands = operands;
            line->instruction.operand_count = operand_count;

            current_address += 1;

            if (!program_append_line(program, line)) {
                free(label);
                if (operands) {
                    for (uint32_t i = 0; i < operand_count; i++) {
                        if (operands[i].type == ASM_OPERAND_VARIABLE) {
                            free(operands[i].variable);
                        } else if (operands[i].type == ASM_OPERAND_LABEL ||
                                   operands[i].type == ASM_OPERAND_LABEL_HIGH ||
                                   operands[i].type == ASM_OPERAND_LABEL_LOW) {
                            free(operands[i].label);
                        }
                    }
                    free(operands);
                }
                free(line);
                parse_error(filename, start_line, start_col, "out of memory");
                asm_free_program(program);
                return NULL;
            }

            pos++;
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
                    free(line->data.label);
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
