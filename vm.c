//
// Created by Sam on 27/08/2018.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "vm.h"

VM *new_vm() {
    VM *vm = calloc(1, sizeof(VM));
    return vm;
}

void free_vm(VM *vm) {
    free(vm);
}

void vm_reset(VM *vm) {
    memset(vm->R, 0, sizeof(uint8_t) * VM_REG_SIZE);
    vm->PC = 0;
    vm->carry = 0;
    vm->halted = false;
}

void vm_clear(VM *vm) {
    vm_reset(vm);
    memset(&vm->M, 0, VM_MEM_SIZE);
}

inline uint8_t vm_get_r(VM *vm, uint8_t r) {
    if (r == 0) {
        return 0;
    }
    if (r < VM_REG_SIZE) {
        return vm->R[r];
    }
    return 0;
}

inline void vm_put_r(VM *vm, uint8_t r, uint8_t v) {
    if (r < VM_REG_SIZE) {
        vm->R[r] = v;
    }
}

inline void vm_decode_Q(VM_Instruction *inst, uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->imm = (uint8_t) (raw & 0x00FF);
}

inline void vm_decode_S(VM_Instruction *inst, uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->rx = (uint8_t) ((raw & 0x00F0) >> 4);
    inst->ry = (uint8_t) ((raw & 0x000F) >> 0);
}

inline void vm_decode_T(VM_Instruction *inst, uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->rx = (uint8_t) ((raw & 0x00F0) >> 4);
    inst->imm = (uint8_t) ((raw & 0x000F) >> 0);
}

inline uint8_t vm_subtract(uint8_t x, uint8_t y, uint8_t *ptr_borrow) {
    uint8_t borrow = *ptr_borrow,
    difference = 0,
    xb, yb;
    for (uint8_t i = 0; i < 8; i++) {
        xb = (uint8_t) ((x >> i) & 1);
        yb = (uint8_t) ((y >> i) & 1);
        difference |= (uint8_t) (((xb ^ yb) ^ borrow) & 1) << i;
        borrow = (uint8_t) (((~xb & yb) | ~(xb ^ yb) * borrow) & 1);
    }

    *ptr_borrow = borrow;
    return difference;
}

void vm_step(VM *vm) {
    VM_Instruction inst;
    memset(&inst, 0, sizeof(VM_Instruction));

    uint16_t PC = vm->PC;
    uint16_t raw = (vm->M[PC] << 8) + vm->M[PC + 1];
    vm->PC += 2;
    inst.op = (uint8_t) (raw >> 12);

    if (vm->halted) {
        return;
    }

    uint8_t temp = 0, x = 0, y = 0;
    uint16_t temp16 = 0;
    switch (inst.op) {
        case OP_LDA:
            vm_decode_Q(&inst, raw);
            vm_put_r(vm, inst.rd, vm->M[inst.imm]);
            break;
        case OP_STA:
            vm_decode_Q(&inst, raw);
            vm->M[inst.imm] = (uint8_t) vm_get_r(vm, inst.rd);
            break;
        case OP_LDI:
            vm_decode_Q(&inst, raw);
            vm_put_r(vm, inst.rd, inst.imm);
            break;
        case OP_ADD:
            vm_decode_S(&inst, raw);
            vm->carry = 0;
            temp = vm_get_r(vm, inst.rx) + vm_get_r(vm, inst.ry);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_ADC:
            vm_decode_S(&inst, raw);
            temp16 = vm_get_r(vm, inst.rx) + vm_get_r(vm, inst.ry) + vm->carry;
            vm->carry = (uint8_t) (temp16 >> 8);
            vm_put_r(vm, inst.rd, (uint8_t)temp16);
            break;
        case OP_SUB:
            vm_decode_S(&inst, raw);
            vm->carry = 0;
            temp = vm_get_r(vm, inst.rx) - vm_get_r(vm, inst.ry);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_SBC:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            temp = vm_subtract(x, y, &vm->carry);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_NOT:
            vm_decode_T(&inst, raw);
            temp = vm_get_r(vm, inst.rx);
            vm_put_r(vm, inst.rd, ~temp);
            break;
        case OP_AND:
            vm_decode_S(&inst, raw);
            temp = vm_get_r(vm, inst.rx) & vm_get_r(vm, inst.ry);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_SHL:
            vm_decode_T(&inst, raw);
            temp = vm_get_r(vm, inst.rx) << vm_get_r(vm, (uint8_t) inst.imm);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_SHR:
            vm_decode_T(&inst, raw);
            temp = vm_get_r(vm, inst.rx) >> vm_get_r(vm, (uint8_t) inst.imm);
            vm_put_r(vm, inst.rd, temp);
            break;
        case OP_SYS:
            vm_decode_Q(&inst, raw);
            if (vm->syscall != NULL) {
                temp = vm->syscall(vm, inst.imm);
                vm_put_r(vm, inst.rd, temp);
            } else {
                printf("SYSCALL #%d\n", inst.imm);
            }
            break;
        case OP_JMP:
            vm_decode_Q(&inst, raw);
            vm->PC = vm->M[inst.rd];
            break;
        case OP_JEQ:
            vm_decode_S(&inst, raw);
            if (vm_get_r(vm, inst.rx) == vm_get_r(vm, inst.ry)) {
                vm->PC = vm->M[inst.rd];
            }
            break;
        case OP_JLT:
            vm_decode_S(&inst, raw);
            if (vm_get_r(vm, inst.rx) < vm_get_r(vm, inst.ry)) {
                vm->PC = vm->M[inst.rd];
            }
            break;
        case OP_HLT:
            printf("Halted at 0x%04hX\n", PC);
            vm->halted = true;
            break;
        default:
            printf("Unknown Instruction at 0x%04hX: 0x%hhX\n", PC, inst.op);
            vm->halted = true;
            break;
    }
}