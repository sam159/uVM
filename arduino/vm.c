/*
Copyright 2018 Sam Stevens

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
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

    uint8_t PC = vm->PC;
    if (PC % 2 != 0) {
        if (vm->error != NULL) {
            vm->error(VM_ERR_MISALIGN);
        }
        vm->halted = true;
        return;
    }
    uint16_t raw = (vm->readAddr(PC, true) << 8) + vm->readAddr(PC + 1, true);
    vm->PC += 2;
    inst.op = (uint8_t) (raw >> 12);

    if (vm->halted) {
        return;
    }

    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t d = 0;
    uint16_t temp16 = 0;
    switch (inst.op) {
        case OP_LDA:
            vm_decode_T(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            temp16 = x;
            temp16 += inst.imm;
            if (temp16 < VM_MEM_SIZE) {
              d = vm->readAddr(temp16, false);
            } else {
              if (vm->error != NULL) {
                vm->error(VM_ERR_OUT_OF_BOUNDS);
              }
              vm->halted = true;
            }
            break;
        case OP_STA:
            vm_decode_T(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            temp16 = vm_get_r(vm, inst.rd);
            temp16 += inst.imm;
            if (temp16 < VM_MEM_SIZE) {
              vm->writeAddr(temp16, x);
            } else {
              if (vm->error != NULL) {
                vm->error(VM_ERR_OUT_OF_BOUNDS);
              }
              vm->halted = true;
            }
            break;
        case OP_LDI:
            vm_decode_Q(&inst, raw);
            d = inst.imm;
            break;
        case OP_ADD:
            vm_decode_S(&inst, raw);
            vm->carry = 0;
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            d = x + y;
            break;
        case OP_ADC:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            temp16 = x + y + vm->carry;
            vm->carry = (uint8_t) (temp16 >> 8);
            d = (uint8_t) temp16;
            break;
        case OP_SUB:
            vm_decode_S(&inst, raw);
            vm->carry = 0;
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            d = x - y;
            break;
        case OP_SBC:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            d = vm_subtract(x, y, &vm->carry);
            break;
        case OP_NOT:
            vm_decode_T(&inst, raw);
            d = ~x;
            break;
        case OP_AND:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            d = x & y;
            break;
        case OP_SHL:
            vm_decode_T(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            d = x << vm_get_r(vm, inst.imm);
            break;
        case OP_SHR:
            vm_decode_T(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            d = x >> vm_get_r(vm, inst.imm);
            break;
        case OP_SYS:
            vm_decode_T(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            if (vm->syscall != NULL) {
                d = vm->syscall(vm, x, inst.imm);
            } else {
                d = 0;
            }
            break;
        case OP_JMP:
            vm_decode_Q(&inst, raw);
            vm->PC = vm_get_r(vm, inst.rd);
            break;
        case OP_JEQ:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            if (x == y) {
                vm->PC = vm_get_r(vm, inst.rd);
            }
            break;
        case OP_JLT:
            vm_decode_S(&inst, raw);
            x = vm_get_r(vm, inst.rx);
            y = vm_get_r(vm, inst.ry);
            if (x < y) {
                vm->PC = vm_get_r(vm, inst.rd);
            }
            break;
        case OP_HLT:
            vm->halted = true;
            break;
        default:
            if (vm->error != NULL) {
              vm->error(VM_ERR_UNKNOWN_OP);
            }
            vm->halted = true;
            break;
    }
    vm_put_r(vm, inst.rd, d);
}
