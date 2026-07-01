#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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

static void free_operands(ASMProgramInstructionOperand *ops, uint32_t count) {
    if (!ops) return;
    for (uint32_t i = 0; i < count; i++) {
        if (ops[i].type == ASM_OPERAND_VARIABLE) {
            free(ops[i].variable);
        } else if (ops[i].type == ASM_OPERAND_LABEL ||
                   ops[i].type == ASM_OPERAND_LABEL_HIGH ||
                   ops[i].type == ASM_OPERAND_LABEL_LOW) {
            free(ops[i].label);
        }
    }
    free(ops);
}

static int expect_comma(AsmTokenList *tokens, size_t *pos) {
    AsmToken *tok = &tokens->tokens[*pos];
    if (tok->type != ASM_TOKEN_SYMBOL || tok->value[0] != ',') return 0;
    (*pos)++;
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
        int64_t scaled = sign * val;
        if (scaled < -32768 || scaled > 32767) {
            return 0;
        }
        op->type = ASM_OPERAND_REL;
        op->rel = (int16_t)scaled;
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
                free(op->label);
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
    if (opcode == ASM_INST_JPF) {
        *operands = malloc(sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        if (!parse_mem_operand(tokens, pos, *operands)) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel] for JPF");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
        *operand_count = 1;
    } else if (opcode == ASM_INST_JNZ) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;
        
        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8 for JNZ");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
        
        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ',' after reg8");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        
        if (!parse_rel_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected relative address for JNZ");
            free_operands(*operands, 1); *operands = NULL;
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
            free_operands(*operands, i); *operands = NULL;
            return 0;
        }

        if (i < 2 && !expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, i + 1); *operands = NULL;
            return 0;
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
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!parse_mem_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel]");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_STA) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_mem_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected [reg16] or [reg16+rel]");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!parse_reg16_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected reg16");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_LDI) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected value8");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_NOT) {
        *operands = malloc(sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
        *operand_count = 1;
    } else if (opcode == ASM_INST_SHL || opcode == ASM_INST_SHR) {
        *operands = malloc(2 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!parse_value_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected value4");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }
        *operand_count = 2;
    } else if (opcode == ASM_INST_JPC) {
        *operands = malloc(3 * sizeof(ASMProgramInstructionOperand));
        if (!*operands) return 0;

        if (!parse_reg8_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected reg8");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!parse_test_operand(tokens, pos, &(*operands)[1])) {
            parse_error(filename, line, col, "expected test");
            free_operands(*operands, 1); *operands = NULL;
            return 0;
        }

        if (!expect_comma(tokens, pos)) {
            parse_error(filename, line, col, "expected ','");
            free_operands(*operands, 2); *operands = NULL;
            return 0;
        }

        if (!parse_reg8_operand(tokens, pos, &(*operands)[2])) {
            parse_error(filename, line, col, "expected reg8");
            free_operands(*operands, 2); *operands = NULL;
            return 0;
        }
        *operand_count = 3;
    } else {
        parse_error(filename, line, col, "unknown T format opcode");
        return 0;
    }

    return 1;
}

static int parse_operands_v(AsmTokenList *tokens, size_t *pos, ASMProgramInstructionType opcode,
                            ASMProgramInstructionOperand **operands, uint32_t *operand_count,
                            const char *filename, int line, int col) {
    *operands = malloc(sizeof(ASMProgramInstructionOperand));
    if (!*operands) return 0;

    if (opcode == ASM_INST_HLT) {
        if (!parse_value_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected value12 for HLT");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
    } else {
        if (!parse_rel_operand(tokens, pos, &(*operands)[0])) {
            parse_error(filename, line, col, "expected relative address");
            free_operands(*operands, 0); *operands = NULL;
            return 0;
        }
    }
    *operand_count = 1;
    return 1;
}

ASMProgram *asm_parse(const char *filename, AsmTokenList *tokens) {
    ASMProgram *program = calloc(1, sizeof(ASMProgram));
    if (!program) return NULL;

    program->filename = strdup(filename);
    if (!program->filename) { free(program); return NULL; }

    size_t pos = 0;
    uint16_t current_address = 0;
    char *label = NULL, *data_label = NULL, *var_name = NULL;
    ASMProgramInstructionOperand *operands = NULL;
    uint32_t operand_count = 0;
    uint8_t *data_buf = NULL;
    ASMProgramLine *line = NULL;

    while (pos < tokens->count) {
        AsmToken *tok = &tokens->tokens[pos];
        label = NULL; data_label = NULL; var_name = NULL;
        operands = NULL; operand_count = 0; data_buf = NULL; line = NULL;

        if (tok->type == ASM_TOKEN_WHITESPACE) { pos++; continue; }
        if (tok->type == ASM_TOKEN_EOF) break;

        if (tok->type == ASM_TOKEN_NEWLINE) {
            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, tok->line, tok->col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_EMPTY;
            line->line_number = tok->line;
            line->address = current_address;
            if (!program_append_line(program, line)) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                goto fail;
            }
            line = NULL;
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_COMMENT) {
            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, tok->line, tok->col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_EMPTY;
            line->line_number = tok->line;
            line->address = current_address;
            line->comment = malloc(tok->value_len + 1);
            if (line->comment) strcpy(line->comment, tok->value);
            if (!program_append_line(program, line)) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                goto fail;
            }
            line = NULL;
            pos++;
            while (pos < tokens->count && tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) pos++;
            if (pos < tokens->count) pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_ORIGIN) {
            pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                parse_error(filename, tok->line, tok->col, "expected number after :origin");
                goto fail;
            }
            int64_t addr;
            if (!parse_number(tokens->tokens[pos].value, &addr) || addr < 0 || addr > 0xFFFF) {
                parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                           "invalid origin address");
                goto fail;
            }
            current_address = (uint16_t)addr;
            pos++;
            while (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_COMMENT) pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, tok->line, tok->col, "expected newline after :origin");
                goto fail;
            }
            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, tok->line, tok->col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_ORIGIN;
            line->line_number = tok->line;
            line->address = current_address;
            line->origin.address = current_address;
            if (!program_append_line(program, line)) {
                parse_error(filename, tok->line, tok->col, "out of memory");
                goto fail;
            }
            line = NULL;
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_IDENT && tok->value[0] == '$') {
            int start_line = tok->line;
            int start_col = tok->col;

            var_name = malloc(tok->value_len + 1);
            if (!var_name) { parse_error(filename, tok->line, tok->col, "out of memory"); goto fail; }
            strcpy(var_name, tok->value);
            pos++;

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_SYMBOL ||
                tokens->tokens[pos].value[0] != '=') {
                parse_error(filename, start_line, start_col, "expected '=' after alias name");
                goto fail;
            }
            pos++;

            if (pos >= tokens->count) {
                parse_error(filename, start_line, start_col, "expected value after '='");
                goto fail;
            }

            int64_t value = 0;
            if (tokens->tokens[pos].type == ASM_TOKEN_NUMBER) {
                if (!parse_number(tokens->tokens[pos].value, &value)) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "invalid number");
                    goto fail;
                }
            } else if (tokens->tokens[pos].type == ASM_TOKEN_REGISTER) {
                int reg_id = parse_register_id(tokens->tokens[pos].value);
                if (reg_id < 0) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "invalid register");
                    goto fail;
                }
                value = reg_id;
            } else {
                parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                           "expected number or register");
                goto fail;
            }
            pos++;

            while (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_COMMENT) pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after alias");
                goto fail;
            }

            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_ALIAS;
            line->line_number = start_line;
            line->address = current_address;
            line->alias.variable = var_name;
            line->alias.value = value;

            if (!program_append_line(program, line)) {
                parse_error(filename, start_line, start_col, "out of memory");
                goto fail;
            }
            var_name = NULL; line = NULL;
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_DATA) {
            int start_line = tok->line;
            int start_col = tok->col;
            pos++;

            if (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_LABEL_REF) {
                data_label = malloc(tokens->tokens[pos].value_len + 1);
                if (!data_label) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
                strcpy(data_label, tokens->tokens[pos].value);
                pos++;
            }

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_DATA_TYPE) {
                parse_error(filename, start_line, start_col, "expected data type");
                goto fail;
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
                goto fail;
            }
            pos++;

            uint32_t data_len = 0;

            if (dtype == ASM_DATA_DB) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                    parse_error(filename, start_line, start_col, "expected number for DB");
                    goto fail;
                }
                int64_t val;
                if (!parse_number(tokens->tokens[pos].value, &val) || val < 0 || val > 255) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "DB value out of range");
                    goto fail;
                }
                data_buf = malloc(1);
                if (!data_buf) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
                data_buf[0] = (uint8_t)val;
                data_len = 1;
                pos++;
            } else if (dtype == ASM_DATA_DW) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                    parse_error(filename, start_line, start_col, "expected number for DW");
                    goto fail;
                }
                int64_t val;
                if (!parse_number(tokens->tokens[pos].value, &val) || val < 0 || val > 0xFFFF) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "DW value out of range");
                    goto fail;
                }
                data_buf = malloc(2);
                if (!data_buf) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
                data_buf[0] = (uint8_t)(val >> 8);
                data_buf[1] = (uint8_t)val;
                data_len = 2;
                pos++;
            } else if (dtype == ASM_DATA_STZ || dtype == ASM_DATA_STL) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_STRING) {
                    parse_error(filename, start_line, start_col, "expected string");
                    goto fail;
                }

                const char *str = tokens->tokens[pos].value;
                size_t str_len = tokens->tokens[pos].value_len;

                size_t needed = str_len + (dtype == ASM_DATA_STZ ? 1 : 2);
                data_buf = malloc(needed);
                if (!data_buf) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }

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
            } else if (dtype == ASM_DATA_DX) {
                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NUMBER) {
                    parse_error(filename, start_line, start_col, "expected hex number for DX");
                    goto fail;
                }
                const char *hex_str = tokens->tokens[pos].value;
                const char *digits = hex_str;
                if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X')) {
                    digits += 2;
                }
                size_t digit_len = strlen(digits);
                if (digit_len == 0 || digit_len % 2 != 0) {
                    parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                               "DX requires an even number of hex digits");
                    goto fail;
                }
                for (size_t i = 0; i < digit_len; i++) {
                    if (!isxdigit((unsigned char)digits[i])) {
                        parse_error(filename, tokens->tokens[pos].line, tokens->tokens[pos].col,
                                   "DX: invalid hex digit");
                        goto fail;
                    }
                }
                data_len = (uint32_t)(digit_len / 2);
                data_buf = malloc(data_len);
                if (!data_buf) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
                for (uint32_t i = 0; i < data_len; i++) {
                    char byte_str[3] = {digits[i * 2], digits[i * 2 + 1], '\0'};
                    data_buf[i] = (uint8_t)strtoul(byte_str, NULL, 16);
                }
                pos++;
            } else {
                parse_error(filename, start_line, start_col, "unknown data type");
                goto fail;
            }

            while (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_COMMENT) pos++;
            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after :data");
                goto fail;
            }

            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_DATA;
            line->line_number = start_line;
            line->address = current_address;
            line->data.label = data_label;
            line->data.type = dtype;
            line->data.data = data_buf;
            line->data.data_length = data_len;

            current_address += data_len;

            if (!program_append_line(program, line)) {
                parse_error(filename, start_line, start_col, "out of memory");
                goto fail;
            }
            data_label = NULL; data_buf = NULL; line = NULL;
            pos++;
            continue;
        }

        if (tok->type == ASM_TOKEN_LABEL || tok->type == ASM_TOKEN_OPCODE) {
            int start_line = tok->line;
            int start_col = tok->col;

            if (tok->type == ASM_TOKEN_LABEL) {
                label = malloc(tok->value_len + 1);
                if (!label) { parse_error(filename, tok->line, tok->col, "out of memory"); goto fail; }
                strcpy(label, tok->value);
                pos++;

                if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_OPCODE) {
                    parse_error(filename, start_line, start_col, "expected opcode after label");
                    goto fail;
                }
                tok = &tokens->tokens[pos];
            }

            ASMProgramInstructionType opcode;
            if (!opcode_from_string(tok->value, &opcode)) {
                parse_error(filename, tok->line, tok->col, "unknown opcode");
                goto fail;
            }
            pos++;

            if (opcode == ASM_INST_JPF || opcode == ASM_INST_JNZ) {
                if (!parse_operands_q(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    goto fail;
                }
            } else if (opcode == ASM_INST_ADD || opcode == ASM_INST_ADC || opcode == ASM_INST_SUB ||
                       opcode == ASM_INST_SBC || opcode == ASM_INST_AND) {
                if (!parse_operands_s(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    goto fail;
                }
            } else if (opcode == ASM_INST_LDA || opcode == ASM_INST_STA || opcode == ASM_INST_LDI ||
                       opcode == ASM_INST_NOT || opcode == ASM_INST_SHL || opcode == ASM_INST_SHR ||
                       opcode == ASM_INST_JPC) {
                if (!parse_operands_t(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    goto fail;
                }
            } else if (opcode == ASM_INST_JMP || opcode == ASM_INST_HLT) {
                if (!parse_operands_v(tokens, &pos, opcode, &operands, &operand_count,
                                      filename, start_line, start_col)) {
                    goto fail;
                }
            }

            while (pos < tokens->count && tokens->tokens[pos].type == ASM_TOKEN_COMMENT) pos++;

            if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_NEWLINE) {
                parse_error(filename, start_line, start_col, "expected newline after instruction");
                goto fail;
            }

            line = calloc(1, sizeof(ASMProgramLine));
            if (!line) { parse_error(filename, start_line, start_col, "out of memory"); goto fail; }
            line->type = ASM_PROGRAM_LINE_INSTRUCTION;
            line->line_number = start_line;
            line->address = current_address;
            line->instruction.label = label;
            line->instruction.op = opcode;
            line->instruction.operands = operands;
            line->instruction.operand_count = operand_count;

            current_address += 2;

            if (!program_append_line(program, line)) {
                parse_error(filename, start_line, start_col, "out of memory");
                goto fail;
            }
            label = NULL; operands = NULL; operand_count = 0; line = NULL;

            pos++;
            continue;
        }

        parse_error(filename, tok->line, tok->col, "unexpected token");
        goto fail;
    }

    if (pos >= tokens->count || tokens->tokens[pos].type != ASM_TOKEN_EOF) {
        parse_error(filename, 0, 0, "unexpected end of input, expected EOF");
        goto fail;
    }

    return program;

fail:
    free(label);
    free(data_label);
    free(var_name);
    free(data_buf);
    free_operands(operands, operand_count);
    if (line) { free(line->comment); free(line); }
    asm_free_program(program);
    return NULL;
}

static int symbol_table_add(ASMSymbolTable *table, const char *name, uint16_t address) {
    for (uint32_t i = 0; i < table->count; i++) {
        if (strcmp(table->entries[i].name, name) == 0) {
            return 0;
        }
    }
    if (table->count >= table->capacity) {
        uint32_t new_cap = table->capacity ? table->capacity * 2 : 16;
        ASMSymbolEntry *tmp = realloc(table->entries, new_cap * sizeof(ASMSymbolEntry));
        if (!tmp) return -1;
        table->entries = tmp;
        table->capacity = new_cap;
    }
    table->entries[table->count].name = strdup(name);
    if (!table->entries[table->count].name) return -1;
    table->entries[table->count].address = address;
    table->count++;
    return 1;
}

typedef struct {
    char *name;
    int64_t value;
} AliasEntry;

bool asm_resolve(ASMProgram *program) {
    if (!program) return false;

    for (uint32_t i = 0; i < program->line_count; i++) {
        ASMProgramLine *line = program->lines[i];
        char *label = NULL;
        if (line->type == ASM_PROGRAM_LINE_INSTRUCTION && line->instruction.label) {
            label = line->instruction.label;
        } else if (line->type == ASM_PROGRAM_LINE_DATA && line->data.label) {
            label = line->data.label;
        }
        if (label) {
            const char *name = label;
            if (name[0] == ':') name++;
            int result = symbol_table_add(&program->symbol_table, name, line->address);
            if (result == 0) {
                parse_error(program->filename, line->line_number, 0, "duplicate label");
                return false;
            } else if (result < 0) {
                parse_error(program->filename, line->line_number, 0, "out of memory");
                return false;
            }
        }
    }

    AliasEntry *aliases = NULL;
    uint32_t alias_count = 0;
    uint32_t alias_capacity = 0;

    for (uint32_t i = 0; i < program->line_count; i++) {
        ASMProgramLine *line = program->lines[i];
        if (line->type == ASM_PROGRAM_LINE_ALIAS) {
            if (alias_count >= alias_capacity) {
                uint32_t new_cap = alias_capacity ? alias_capacity * 2 : 16;
                AliasEntry *tmp = realloc(aliases, new_cap * sizeof(AliasEntry));
                if (!tmp) {
                    parse_error(program->filename, line->line_number, 0, "out of memory");
                    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                    free(aliases);
                    return false;
                }
                aliases = tmp;
                alias_capacity = new_cap;
            }
            aliases[alias_count].name = strdup(line->alias.variable);
            if (!aliases[alias_count].name) {
                parse_error(program->filename, line->line_number, 0, "out of memory");
                for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                free(aliases);
                return false;
            }
            aliases[alias_count].value = line->alias.value;
            alias_count++;
        }
    }

    for (uint32_t i = 0; i < program->line_count; i++) {
        ASMProgramLine *line = program->lines[i];
        if (line->type != ASM_PROGRAM_LINE_INSTRUCTION) continue;

        for (uint32_t j = 0; j < line->instruction.operand_count; j++) {
            ASMProgramInstructionOperand *op = &line->instruction.operands[j];

            if (op->type == ASM_OPERAND_VARIABLE) {
                int64_t value = 0;
                int found = 0;
                for (uint32_t k = 0; k < alias_count; k++) {
                    if (strcmp(op->variable, aliases[k].name) == 0) {
                        value = aliases[k].value;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    parse_error(program->filename, line->line_number, 0, "undefined variable");
                    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                    free(aliases);
                    return false;
                }
                free(op->variable);
                op->variable = NULL;
                if (value >= 0 && value <= 15) {
                    op->type = ASM_OPERAND_REG8;
                    op->reg8 = (uint8_t)value;
                } else {
                    op->type = ASM_OPERAND_VALUE16;
                    op->value16 = (uint16_t)value;
                }
            } else if (op->type == ASM_OPERAND_LABEL) {
                uint16_t target_addr = 0;
                int found = 0;
                for (uint32_t k = 0; k < program->symbol_table.count; k++) {
                    if (strcmp(op->label, program->symbol_table.entries[k].name) == 0) {
                        target_addr = program->symbol_table.entries[k].address;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    parse_error(program->filename, line->line_number, 0, "undefined label");
                    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                    free(aliases);
                    return false;
                }
                free(op->label);
                op->label = NULL;
                if (line->instruction.op == ASM_INST_JMP ||
                    (line->instruction.op == ASM_INST_JNZ && j == 1)) {
                    int16_t rel = (int16_t)(target_addr - line->address - 2);
                    op->type = ASM_OPERAND_REL;
                    op->rel = rel;
                } else {
                    op->type = ASM_OPERAND_VALUE16;
                    op->value16 = target_addr;
                }
            } else if (op->type == ASM_OPERAND_LABEL_HIGH) {
                uint16_t addr = 0;
                int found = 0;
                for (uint32_t k = 0; k < program->symbol_table.count; k++) {
                    if (strcmp(op->label, program->symbol_table.entries[k].name) == 0) {
                        addr = program->symbol_table.entries[k].address;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    parse_error(program->filename, line->line_number, 0, "undefined label");
                    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                    free(aliases);
                    return false;
                }
                free(op->label);
                op->label = NULL;
                op->type = ASM_OPERAND_VALUE8;
                op->value8 = (uint8_t)((addr >> 8) & 0xFF);
            } else if (op->type == ASM_OPERAND_LABEL_LOW) {
                uint16_t addr = 0;
                int found = 0;
                for (uint32_t k = 0; k < program->symbol_table.count; k++) {
                    if (strcmp(op->label, program->symbol_table.entries[k].name) == 0) {
                        addr = program->symbol_table.entries[k].address;
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    parse_error(program->filename, line->line_number, 0, "undefined label");
                    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
                    free(aliases);
                    return false;
                }
                free(op->label);
                op->label = NULL;
                op->type = ASM_OPERAND_VALUE8;
                op->value8 = (uint8_t)(addr & 0xFF);
            }
        }
    }

    for (uint32_t j = 0; j < alias_count; j++) free(aliases[j].name);
    free(aliases);

    return true;
}

void asm_free_program(ASMProgram *program) {
    if (!program) {
        return;
    }

    free(program->filename);

    for (uint32_t i = 0; i < program->symbol_table.count; i++) {
        free(program->symbol_table.entries[i].name);
    }
    free(program->symbol_table.entries);

    if (program->lines) {
        for (uint32_t i = 0; i < program->line_count; i++) {
            ASMProgramLine *line = program->lines[i];
            if (!line) continue;

            switch (line->type) {
                case ASM_PROGRAM_LINE_INSTRUCTION:
                    free(line->instruction.label);
                    free_operands(line->instruction.operands, line->instruction.operand_count);
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
