#ifndef UVM_ASM_TREE_H
#define UVM_ASM_TREE_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "token.h"

typedef struct ASMProgram ASMProgram;
typedef struct ASMProgramLine ASMProgramLine;

/* ASMProgram */
struct ASMProgram {
    char *filename;
    ASMProgramLine **lines;
    uint32_t line_count;
};

/* ASMProgramLineType */
typedef enum {
    ASM_PROGRAM_LINE_ORIGIN,
    ASM_PROGRAM_LINE_DATA,
    ASM_PROGRAM_LINE_ALIAS,
    ASM_PROGRAM_LINE_INSTRUCTION,
    ASM_PROGRAM_LINE_EMPTY,
} ASMProgramLineType;

/* ASMProgramOrigin */
typedef struct {
    uint16_t address;
} ASMProgramOrigin;

/* ASMProgramDataType */
typedef enum {
    ASM_DATA_DB,
    ASM_DATA_DW,
    ASM_DATA_DX,
    ASM_DATA_STZ,
    ASM_DATA_STL,
} ASMProgramDataType;

/* ASMProgramData */
typedef struct {
    ASMProgramDataType type;
    uint8_t *data;
    uint32_t data_length;
} ASMProgramData;

/* ASMProgramAlias */
typedef struct {
    char *variable;
    int64_t value;
} ASMProgramAlias;

/* ASMProgramInstructionType – mirrors vm.h opcodes */
typedef enum {
    ASM_INST_HLT = 0,
    ASM_INST_LDA = 1,
    ASM_INST_STA = 2,
    ASM_INST_LDI = 3,
    ASM_INST_ADD = 4,
    ASM_INST_ADC = 5,
    ASM_INST_SUB = 6,
    ASM_INST_SBC = 7,
    ASM_INST_NOT = 8,
    ASM_INST_AND = 9,
    ASM_INST_SHL = 0xa,
    ASM_INST_SHR = 0xb,
    ASM_INST_SYS = 0xc,
    ASM_INST_JMP = 0xd,
    ASM_INST_JEQ = 0xe,
    ASM_INST_JLT = 0xf,
} ASMProgramInstructionType;

/* ASMProgramInstructionOperandTestType */
typedef enum {
    ASM_TEST_EQ,
    ASM_TEST_LT,
    ASM_TEST_LTE,
    ASM_TEST_GT,
    ASM_TEST_GTE,
} ASMProgramInstructionOperandTestType;

/* reg16 rel sub-struct */
typedef struct {
    uint8_t register_id;
    int8_t offset;
} ASMReg16Rel;

/* test sub-struct */
typedef struct {
    ASMProgramInstructionOperandTestType type;
    bool negated;
} ASMTest;

/* ASMProgramInstructionOperandType */
typedef enum {
    ASM_OPERAND_VALUE16,
    ASM_OPERAND_VALUE8,
    ASM_OPERAND_VALUE4,
    ASM_OPERAND_REL,
    ASM_OPERAND_REG8,
    ASM_OPERAND_REG8_DOUBLE,
    ASM_OPERAND_REG16,
    ASM_OPERAND_REG16_REL,
    ASM_OPERAND_VARIABLE,
    ASM_OPERAND_LABEL_HIGH,
    ASM_OPERAND_LABEL_LOW,
    ASM_OPERAND_LABEL,
    ASM_OPERAND_TEST,
} ASMProgramInstructionOperandType;

/* ASMProgramInstructionOperand */
typedef struct {
    ASMProgramInstructionOperandType type;
    union {
        uint16_t value16;
        uint8_t value8;
        uint8_t value4;
        char *variable;
        int16_t rel;
        uint8_t reg8;
        ASMReg16Rel reg16_rel;
        char *label;
        ASMTest test;
    };
} ASMProgramInstructionOperand;

/* ASMProgramInstruction */
typedef struct {
    char *label;
    ASMProgramInstructionType op;
    ASMProgramInstructionOperand *operands;
    uint32_t operand_count;
} ASMProgramInstruction;

/* ASMProgramEmpty – empty line, no data */
typedef struct { } ASMProgramEmpty;

/* ASMProgramLine */
struct ASMProgramLine {
    uint16_t address;
    uint16_t line_number;
    ASMProgramLineType type;
    union {
        ASMProgramOrigin origin;
        ASMProgramData data;
        ASMProgramAlias alias;
        ASMProgramInstruction instruction;
        ASMProgramEmpty empty;
    };
    char *comment;
};

void asm_free_program(ASMProgram *program);

ASMProgram *asm_parse(const char *filename, AsmTokenList *tokens);

#endif //UVM_ASM_TREE_H
