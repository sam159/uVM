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

static void token_init(AsmToken *t, AsmTokenType type, const char *value, int value_len, const int line, const int col) {
    t->type = type;
    t->value_len = value_len;
    t->line = line;
    t->col = col;
    t->value = (char *)malloc((size_t)value_len + 1);
    if (t->value) {
        memcpy(t->value, value, (size_t)value_len);
        t->value[value_len] = '\0';
    }
}

static bool ensure_capacity(AsmToken **arr, int *capacity, const int needed) {
    if (needed <= *capacity) return true;
    int new_cap = *capacity * 2;
    AsmToken *tmp = realloc(*arr, sizeof(AsmToken) * (size_t)new_cap);
    if (!tmp) return false;
    *arr = tmp;
    *capacity = new_cap;
    return true;
}

int asm_tokenize_line(const char *line, int line_num, AsmToken **tokens) {
    *tokens = NULL;
    int capacity = 32;
    int count = 0;
    AsmToken *arr = malloc(sizeof(AsmToken) * (size_t)capacity);
    if (!arr) return 0;

    const char *p = line;
    int col = 0;
    bool at_line_start = true;

    // Handle empty lines: emit a single NEWLINE token using the already-allocated array.
    if (!*p) {
        token_init(&arr[0], ASM_TOKEN_NEWLINE, "", 0, line_num, 0);
        *tokens = arr;
        return 1;
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
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_NEWLINE, "", 0, line_num, col);
            break;
        }

        // Comment
        if (*p == ';') {
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            const char *cstart = p;
            while (*p) {
                p++;
                col++;
            }
            int len = (int)(p - cstart);
            token_init(&arr[count++], ASM_TOKEN_COMMENT, cstart, len, line_num, start_col);
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
            int len = (int)(p - sstart);
            if (*p == quote) {
                p++;
                col++;
            }
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_STRING, sstart, len, line_num, start_col);
            at_line_start = false;
            continue;
        }

        // Colon-prefixed: :origin, :data, or :label reference
        if (*p == ':') {
            p++;
            col++;
            if (*p == ':') {
                // :: is not valid, treat as two colons
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_SYMBOL, ":", 1, line_num, start_col);
                start_col = col;
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
            if (!colon_val) { free(arr); return 0; }
            colon_val[0] = ':';
            memcpy(colon_val + 1, idstart, (size_t)len);
            colon_val[len + 1] = '\0';

            // Check for special colon-directives
            if (strcmp(colon_val, ":origin") == 0) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(colon_val); free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_ORIGIN, colon_val, len + 1, line_num, start_col);
                at_line_start = false;
                free(colon_val);
                continue;
            }
            if (strcmp(colon_val, ":data") == 0) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(colon_val); free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_DATA, colon_val, len + 1, line_num, start_col);
                at_line_start = false;
                free(colon_val);
                continue;
            }

            // Generic :label reference
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(colon_val); free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_LABEL_REF, colon_val, len + 1, line_num, start_col);
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
            if (!dollar_val) { free(arr); return 0; }
            dollar_val[0] = '$';
            memcpy(dollar_val + 1, idstart, (size_t)len);
            dollar_val[len + 1] = '\0';
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(dollar_val); free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_IDENT, dollar_val, len + 1, line_num, start_col);
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
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_NUMBER, nstart, len, line_num, start_col);
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
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_LABEL, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }

            // Check known keywords
            if (strcmp(idstart, "origin") == 0 && at_line_start) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_ORIGIN, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }
            if (strcmp(idstart, "data") == 0 && at_line_start) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_DATA, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }
            if (is_opcode(idstart)) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_OPCODE, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }
            if (is_test(idstart)) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_TEST, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }
            if (is_datatype(idstart)) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_DATA_TYPE, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }
            if (is_register(idstart)) {
                if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
                token_init(&arr[count++], ASM_TOKEN_REGISTER, idstart, len, line_num, start_col);
                at_line_start = false;
                continue;
            }

            // Generic identifier
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_IDENT, idstart, len, line_num, start_col);
            at_line_start = false;
            continue;
        }

        // Symbols
        if (*p == '=' || *p == '+' || *p == '-' || *p == '~' || *p == ',' || *p == '[' || *p == ']') {
            p++;
            col++;
            if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
            token_init(&arr[count++], ASM_TOKEN_SYMBOL, p - 1, 1, line_num, start_col);
            at_line_start = false;
            continue;
        }

        // Unknown character - error token
        if (!ensure_capacity(&arr, &capacity, count + 1)) { free(arr); return 0; }
        token_init(&arr[count++], ASM_TOKEN_ERROR, p, 1, line_num, start_col);
        p++;
        col++;
        at_line_start = false;
    }

    *tokens = arr;
    return count;
}

void asm_free_tokens(AsmToken *tokens, const int count) {
    if (!tokens) return;
    for (int i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
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
