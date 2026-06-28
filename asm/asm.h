#ifndef UVM_ASM_H
#define UVM_ASM_H
#include <stdint.h>

typedef enum LineType {
    LINE_TYPE_DIRECTIVE,
    LINE_TYPE_VARIABLE,
    LINE_TYPE_INSTRUCTION
} LineType;

struct ASM_Line_Directive_t {
    char *name;
    char *value;
};
typedef struct  ASM_Line_Directive_t ASM_Line_Directive;

struct ASM_Line_Variable_t {
    const char *name;
    char *value;
};
typedef struct ASM_Line_Variable_t ASM_Line_Variable;

struct ASM_Line_Instruction_t {
    char *label;
    char op[3];
    char *rd;
    char *rx;
    char *ry;
    uint8_t imm;
    uint16_t immx;
};
typedef struct ASM_Line_Instruction_t ASM_Line_Instruction;

struct ASM_Line_t {
    LineType type;
    union {
        ASM_Line_Directive directive;
        ASM_Line_Variable variable;
        ASM_Line_Instruction instruction;
    };
};
typedef struct ASM_Line_t ASM_Line;

struct ASM_Compile_t {
    ASM_Line **lines;
    size_t lineCount;

};
typedef struct ASM_Compile_t ASM_Compile;

bool asm_compile(const char *input, const char *output);

#endif //UVM_ASM_H
