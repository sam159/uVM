#ifndef UVM_TOKEN_H
#define UVM_TOKEN_H

typedef enum {
    ASM_TOKEN_IDENT,
    ASM_TOKEN_NUMBER,
    ASM_TOKEN_REGISTER,
    ASM_TOKEN_OPCODE,
    ASM_TOKEN_LABEL,
    ASM_TOKEN_ALIAS_DEF,
    ASM_TOKEN_ORIGIN,
    ASM_TOKEN_DATA,
    ASM_TOKEN_DATA_TYPE,
    ASM_TOKEN_STRING,
    ASM_TOKEN_SYMBOL,
    ASM_TOKEN_LABEL_REF,
    ASM_TOKEN_TEST,
    ASM_TOKEN_COMMENT,
    ASM_TOKEN_WHITESPACE,
    ASM_TOKEN_NEWLINE,
    ASM_TOKEN_EOF,
    ASM_TOKEN_ERROR
} AsmTokenType;

typedef struct {
    AsmTokenType type;
    char value[128];
    int line;
    int col;
} AsmToken;

int asm_tokenize_line(const char *line, int line_num, AsmToken **tokens);
void asm_free_tokens(AsmToken *tokens);
const char *asm_token_type_name(AsmTokenType type);

#endif //UVM_TOKEN_H
