#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"

static const char *opcode_list[] = {
    "HLT", "LDA", "STA", "LDI", "ADD", "ADC", "SUB", "SBC",
    "NOT", "AND", "SHL", "SHR", "JMP", "JPF", "JNZ", "JPC", NULL
};

static const char *test_list[] = {
    "EQ", "LT", "LTE", "GT", "GTE", NULL
};

static const char *datatype_list[] = {
    "DB", "DW", "DX", "STZ", "STL", NULL
};

static const char *register_list[] = {
    "R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7",
    "R8", "R9", "RA", "RB", "RC", "RD", "RE", "RF",
    "RZ", "RX0", "RX1", "RX2", "RX3", "RX4", "RX5", "RX6",
    "RX7", "RX8", "RX9", "RXA", "RXB", "RXC", "RXD", "RXE", "RXF",
    "RXZ", NULL
};

static bool is_register(const char *s) {
    for (int i = 0; register_list[i]; i++) {
        if (strcmp(s, register_list[i]) == 0) return true;
    }
    return false;
}

static bool is_opcode(const char *s) {
    for (int i = 0; opcode_list[i]; i++) {
        if (strcmp(s, opcode_list[i]) == 0) return true;
    }
    return false;
}

static bool is_test(const char *s) {
    for (int i = 0; test_list[i]; i++) {
        if (strcmp(s, test_list[i]) == 0) return true;
    }
    return false;
}

static bool is_datatype(const char *s) {
    for (int i = 0; datatype_list[i]; i++) {
        if (strcmp(s, datatype_list[i]) == 0) return true;
    }
    return false;
}

static bool is_hex_char(char c) {
    return isxdigit((unsigned char)c) != 0;
}

static AsmToken *new_token(const AsmTokenType type, const char *value, const size_t value_len, const int line, const int col) {
    AsmToken *t = malloc(sizeof(AsmToken));
    if (!t) return NULL;
    t->type = type;
    t->value_len = value_len;
    t->line = line;
    t->col = col;
    t->value = (char *)malloc(value_len + 1);
    if (t->value) {
        memcpy(t->value, value, value_len);
        t->value[value_len] = '\0';
    }
    return t;
}

AsmToken *asm_token_copy(const AsmToken *src) {
    if (!src) return NULL;
    AsmToken *copy = malloc(sizeof(AsmToken));
    if (!copy) return NULL;
    copy->type = src->type;
    copy->line = src->line;
    copy->col = src->col;
    copy->value_len = src->value_len;
    copy->value = malloc(src->value_len + 1);
    if (!copy->value) {
        free(copy);
        return NULL;
    }
    memcpy(copy->value, src->value, src->value_len);
    copy->value[src->value_len] = '\0';
    return copy;
}

AsmTokenList *asm_tokenize_line(const char *line, int line_num) {
    AsmTokenList *list = asm_token_list_create();
    if (!list) return NULL;

    const char *p = line;
    int col = 0;
    bool at_line_start = true;

    // Handle empty lines: emit a single NEWLINE token.
    AsmToken *t;
    if (!*p) {
        return list;
    }

    while (*p) {
        // Skip leading whitespace
        int start_col = col;
        while (*p == ' ' || *p == '\t') {
            p++;
            col++;
        }

        // Check for end of line (trailing whitespace or after a token)
        if (!*p) {
            t = new_token(ASM_TOKEN_NEWLINE, "", 0, line_num, col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            break;
        }

        if (*p == '\n') {
            p++;
            col++;
            t = new_token(ASM_TOKEN_NEWLINE, "", 0, line_num, col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            continue;
        }

        // Comment
        if (*p == ';') {
            const char *cstart = p;
            while (*p && *p != '\n') {
                p++;
                col++;
            }
            t = new_token(ASM_TOKEN_COMMENT, cstart, (int)(p - cstart), line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            continue;
        }

        // String literal (double-quoted only, per ASM.md)
        if (*p == '"') {
            char quote = *p;
            p++;
            col++;
            const char *sstart = p;
            while (*p && *p != quote) {
                if (*p == '\\' && *(p + 1)) {
                    p += 2;
                    col += 2;
                } else {
                    p++;
                    col++;
                }
            }
            if (*p == quote) {
                p++;
                col++;
            }
            t = new_token(ASM_TOKEN_STRING, sstart, (int)(p - sstart), line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            continue;
        }

        // Colon-prefixed: :origin, :data, or :label reference
        if (*p == ':') {
            p++;
            col++;
            if (*p == ':') {
                // :: is not valid, treat as two colons
                t = new_token(ASM_TOKEN_ERROR, p, 1, line_num, col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                continue;
            }
            // Read the identifier after colon
            const char *idstart = p;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                p++;
                col++;
            }
            int len = (int)(p - idstart);

            // Check for :h (high byte) or :l (low byte) suffix on a label reference.
            // Only consumed when followed by a non-identifier character so that
            // ":label:hello" still tokenizes as ":label" + ":hello".
            if (*p == ':' && (p[1] == 'h' || p[1] == 'l')
                && !isalnum((unsigned char)p[2]) && p[2] != '_') {
                p += 2;
                col += 2;
                len += 2;
            }

            // Build ":<identifier>" or ":<identifier>:h/:l"
            char *colon_val = (char *)malloc((size_t)len + 2);
            if (!colon_val) {
                asm_token_list_free(list);
                return NULL;
            }
            colon_val[0] = ':';
            memcpy(colon_val + 1, idstart, (size_t)len);
            colon_val[len + 1] = '\0';

            // Check for special colon-directives
            if (strcmp(colon_val, ":origin") == 0) {
                t = new_token(ASM_TOKEN_ORIGIN, colon_val, len + 1, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                free(colon_val);
                continue;
            }
            if (strcmp(colon_val, ":data") == 0) {
                t = new_token(ASM_TOKEN_DATA, colon_val, len + 1, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                free(colon_val);
                continue;
            }

            // Generic :label reference
            t = new_token(ASM_TOKEN_LABEL_REF, colon_val, len + 1, line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            free(colon_val);
            continue;
        }

        // Alias definition $name = value
        if (*p == '$') {
            p++;
            col++;
            const char *idstart = p;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                p++;
                col++;
            }
            int len = (int)(p - idstart);
            char *dollar_val = (char *)malloc((size_t)len + 2);
            if (!dollar_val) {
                asm_token_list_free(list);
                return NULL;
            }
            dollar_val[0] = '$';
            memcpy(dollar_val + 1, idstart, (size_t)len);
            dollar_val[len + 1] = '\0';
            t = new_token(ASM_TOKEN_IDENT, dollar_val, len + 1, line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            free(dollar_val);
            continue;
        }

        // Number (including + and - prefixes for relative)
        if (isdigit((unsigned char)*p) || (*p == '+' && isdigit((unsigned char)*(p + 1))) || (*p == '-' && isdigit((unsigned char)*(p + 1)))) {
            const char *nstart = p;
            if (*p == '+' || *p == '-') {
                p++;
                col++;
            }
            if (*p == '0' && (p[1] == 'x' || p[1] == 'X')) {
                p += 2;
                col += 2;
                while (*p && is_hex_char(*p)) { p++; col++; }
            } else if (*p == '0' && (p[1] == 'b' || p[1] == 'B')) {
                p += 2;
                col += 2;
                while (*p && (*p == '0' || *p == '1')) { p++; col++; }
            } else {
                while (*p && isdigit((unsigned char)*p)) { p++; col++; }
            }
            int len = (int)(p - nstart);
            t = new_token(ASM_TOKEN_NUMBER, nstart, len, line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            continue;
        }

        // Identifier / keyword
        if (isalpha((unsigned char)*p) || *p == '_') {
            const char *idstart = p;
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                p++;
                col++;
            }
            int len = (int)(p - idstart);

            // Check for label:
            if (at_line_start && p[0] == ':') {
                p++;
                col++;
                t = new_token(ASM_TOKEN_LABEL, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }

            // Check known keywords
            if (strcmp(idstart, "origin") == 0 && at_line_start) {
                t = new_token(ASM_TOKEN_ORIGIN, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }
            if (strcmp(idstart, "data") == 0 && at_line_start) {
                t = new_token(ASM_TOKEN_DATA, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }
            if (is_opcode(idstart)) {
                t = new_token(ASM_TOKEN_OPCODE, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }
            if (is_test(idstart)) {
                t = new_token(ASM_TOKEN_TEST, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }
            if (is_datatype(idstart)) {
                t = new_token(ASM_TOKEN_DATA_TYPE, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                at_line_start = false;
                continue;
            }
            if (is_register(idstart)) {
                t = new_token(ASM_TOKEN_REGISTER, idstart, len, line_num, start_col);
                if (t) {
                    asm_token_list_append(list, *t);
                    free(t);
                }
                t = NULL;
                continue;
            }

            // Generic identifier
            t = new_token(ASM_TOKEN_IDENT, idstart, len, line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            continue;
        }

        // Symbols
        if (*p == '=' || *p == '+' || *p == '-' || *p == '~' || *p == ',' || *p == '[' || *p == ']') {
            p++;
            col++;
            t = new_token(ASM_TOKEN_SYMBOL, p - 1, 1, line_num, start_col);
            if (t) {
                asm_token_list_append(list, *t);
                free(t);
            }
            t = NULL;
            at_line_start = false;
            continue;
        }

        // Unknown character - error token
        t = new_token(ASM_TOKEN_ERROR, p, 1, line_num, start_col);
        if (t) {
            asm_token_list_append(list, *t);
            free(t);
        }
        t = NULL;
        p++;
        col++;
        at_line_start = false;
    }

    return list;
}

void asm_free_tokens(AsmToken *tokens, const size_t count) {
    if (!tokens) return;
    for (int i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}

static int list_ensure_capacity(AsmTokenList *list, size_t needed) {
    if (needed <= list->capacity) return 1;
    size_t new_cap = list->capacity * 2;
    AsmToken *tmp = realloc(list->tokens, sizeof(AsmToken) * new_cap);
    if (!tmp) return 0;
    list->tokens = tmp;
    list->capacity = new_cap;
    return 1;
}

AsmTokenList *asm_token_list_create(void) {
    AsmTokenList *list = malloc(sizeof(AsmTokenList));
    if (!list) return NULL;
    list->capacity = 32;
    list->count = 0;
    list->tokens = malloc(sizeof(AsmToken) * list->capacity);
    if (!list->tokens) {
        free(list);
        return NULL;
    }
    return list;
}

void asm_token_list_free(AsmTokenList *list) {
    if (!list) return;
    if (list->tokens) {
        for (int i = 0; i < list->count; i++) {
            free(list->tokens[i].value);
        }
        free(list->tokens);
    }
    free(list);
}

int asm_token_list_append(AsmTokenList *list, AsmToken token) {
    if (!list) return 0;
    if (!list_ensure_capacity(list, list->count + 1)) return 0;
    list->tokens[list->count++] = token;
    return 1;
}

int asm_token_list_concat(AsmTokenList *dest, const AsmTokenList *src) {
    if (!dest || !src) return 0;
    if (!list_ensure_capacity(dest, dest->count + src->count)) return 0;
    for (int i = 0; i < src->count; i++) {
        const AsmToken *s = &src->tokens[i];
        AsmToken *d = &dest->tokens[dest->count++];
        d->type = s->type;
        d->line = s->line;
        d->col = s->col;
        d->value_len = s->value_len;
        d->value = malloc(s->value_len + 1);
        if (!d->value) return 0;
        memcpy(d->value, s->value, s->value_len);
        d->value[s->value_len] = '\0';
    }
    return 1;
}

const char *asm_token_type_name(AsmTokenType type) {
    switch (type) {
        case ASM_TOKEN_IDENT:       return "IDENT";
        case ASM_TOKEN_NUMBER:      return "NUMBER";
        case ASM_TOKEN_REGISTER:    return "REGISTER";
        case ASM_TOKEN_OPCODE:      return "OPCODE";
        case ASM_TOKEN_LABEL:       return "LABEL";
        case ASM_TOKEN_ALIAS_DEF:   return "ALIAS_DEF";
        case ASM_TOKEN_ORIGIN:      return "ORIGIN";
        case ASM_TOKEN_DATA:        return "DATA";
        case ASM_TOKEN_DATA_TYPE:   return "DATA_TYPE";
        case ASM_TOKEN_STRING:      return "STRING";
        case ASM_TOKEN_SYMBOL:      return "SYMBOL";
        case ASM_TOKEN_LABEL_REF:   return "LABEL_REF";
        case ASM_TOKEN_TEST:        return "TEST";
        case ASM_TOKEN_COMMENT:     return "COMMENT";
        case ASM_TOKEN_WHITESPACE:  return "WHITESPACE";
        case ASM_TOKEN_NEWLINE:     return "NEWLINE";
        case ASM_TOKEN_EOF:         return "EOF";
        case ASM_TOKEN_ERROR:       return "ERROR";
        default:                    return "UNKNOWN";
    }
}
