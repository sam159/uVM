//
// Created by Sam on 27/08/2018.
//

#ifndef UVM_VM_H
#define UVM_VM_H

#include <stdint.h>
#include <stdbool.h>

#define VM_MEM_SIZE (1<<8)-1
#define VM_REG_SIZE 15

#define OP_HLT 0x0
#define OP_LDA 0x1
#define OP_STA 0x2
#define OP_LDI 0x3
#define OP_ADD 0x4
#define OP_ADC 0x5
#define OP_SUB 0x6
#define OP_SBC 0x7
#define OP_NOT 0x8
#define OP_AND 0x9
#define OP_SHL 0xa
#define OP_SHR 0xb
#define OP_SYS 0xc
#define OP_JMP 0xd
#define OP_JEQ 0xe
#define OP_JLT 0xf

struct VM_Instruction_t {
    uint8_t op;
    uint8_t rd;
    uint8_t rx;
    uint8_t ry;
    uint8_t imm;
};
typedef struct VM_Instruction_t VM_Instruction;

struct VM_t {
    uint8_t R[VM_REG_SIZE];
    uint16_t PC;
    uint8_t M[VM_MEM_SIZE];
    uint8_t carry;
    bool halted;
    uint8_t (*syscall)(struct VM_t* vm, uint8_t callno);
};
typedef struct VM_t VM;

VM * new_vm();

void free_vm(VM *vm);

void vm_reset(VM *vm);

void vm_clear(VM *vm);

uint8_t vm_get_r(VM *vm, uint8_t r);

void vm_put_r(VM *vm, uint8_t r, uint8_t v);

void vm_decode_Q(VM_Instruction *inst, uint16_t raw);
void vm_decode_S(VM_Instruction *inst, uint16_t raw);
void vm_decode_T(VM_Instruction *inst, uint16_t raw);

uint8_t vm_subtract(uint8_t x, uint8_t y, uint8_t *ptr_borrow);

void vm_step(VM *vm);

#endif //UVM_VM_H
