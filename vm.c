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
    vm->haltCode = 0;
}

void vm_clear(VM *vm) {
    vm_reset(vm);
}

inline uint8_t vm_get_r(VM *vm, const uint8_t r) {
    if (r == 0) {
        return 0;
    }
    if (r < VM_REG_SIZE) {
        return vm->R[r];
    }
    return 0;
}

inline uint16_t vm_get_rx(VM *vm, const uint8_t r) {
    if (r <= 0xA) {
        return vm_get_r(vm, r);
    }
    switch (r) {
        case 0xB:
            return (vm->R[0x6] << 8) + vm->R[0x7];
        case 0xC:
            return (vm->R[0x8] << 8) + vm->R[0x9];
        case 0xD:
            return (vm->R[0xA] << 8) + vm->R[0xB];
        case 0xE:
            return (vm->R[0xC] << 8) + vm->R[0xD];
        case 0xF:
            return (vm->R[0xE] << 8) + vm->R[0xF];
        default:
            return 0;
    }
}

inline void vm_put_r(VM *vm, const uint8_t r, const uint8_t v) {
    if (r == 0) {
        return;
    }
    if (r < VM_REG_SIZE) {
        vm->R[r] = v;
    }
}

inline void vm_decode_Q(VM_Instruction *inst, const uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->imm = (uint8_t) (raw & 0x00FF);
}

inline void vm_decode_S(VM_Instruction *inst, const uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->rx = (uint8_t) ((raw & 0x00F0) >> 4);
    inst->ry = (uint8_t) ((raw & 0x000F) >> 0);
}

inline void vm_decode_T(VM_Instruction *inst, const uint16_t raw) {
    inst->rd = (uint8_t) ((raw & 0x0F00) >> 8);
    inst->rx = (uint8_t) ((raw & 0x00F0) >> 4);
    inst->imm = (uint8_t) ((raw & 0x000F) >> 0);
}

inline void vm_decode_V(VM_Instruction *inst, uint16_t raw) {
    inst->immx = (uint16_t) ((raw & 0x0FFF) >> 0);
}

inline uint8_t vm_subtract(const uint8_t x, const uint8_t y, uint8_t *ptr_borrow) {
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

inline bool vm_test(const uint8_t a, const uint8_t b, const uint8_t test) {
    if (test == 0) {
        return false;
    }
    bool result = false;

    if (test & TEST_EQ) {
        result = a == b;
    }
    if (!result && test & TEST_GT) {
        result = a > b;
    }
    if (!result && test & TEST_LT) {
        result = a < b;
    }

    if (test & TEST_NOT) {
        return !result;
    }
    return result;
}

bool vm_step(VM *vm) {
    VM_Instruction inst;

    if (vm->halted) {
        return false;
    }

    const uint16_t PC = vm->PC;
    if (PC % 2 != 0) {
        if (vm->error != NULL) {
            vm->error(VM_ERR_MISALIGN);
        }
        vm->halted = true;
        return false;
    }
    const uint16_t raw = (vm->readAddr(PC, true) << 8) + vm->readAddr(PC + 1, true);
    vm->PC += 2;
    inst.op = (uint8_t) (raw >> 12);

    uint8_t rx = 0, ry = 0, rd = 0;
    uint16_t rdx = 0, rxx = 0, temp16 = 0;
    switch (inst.op) {
        case OP_HLT:
            vm_decode_V(&inst, raw);
            vm->halted = true;
            vm->haltCode = inst.immx;
            break;
        case OP_LDA:
            vm_decode_T(&inst, raw);
            rxx = vm_get_rx(vm, inst.rx);
            rxx += inst.imm;
            rd = vm->readAddr(rxx, false);
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_STA:
            vm_decode_T(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            rdx = vm_get_rx(vm, inst.rd);
            rdx += inst.imm;
            vm->writeAddr(rdx, rx);
            break;
        case OP_LDI:
            vm_decode_Q(&inst, raw);
            rd = inst.imm;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_ADD:
            vm_decode_S(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            ry = vm_get_r(vm, inst.ry);
            rd = rx + ry;
            vm->carry = 0;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_ADC:
            vm_decode_S(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            ry = vm_get_r(vm, inst.ry);
            temp16 = rx + ry + vm->carry;
            vm->carry = (uint8_t) (temp16 >> 8);
            rd = (uint8_t) temp16;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_SUB:
            vm_decode_S(&inst, raw);
            vm->carry = 0;
            rx = vm_get_r(vm, inst.rx);
            ry = vm_get_r(vm, inst.ry);
            rd = rx - ry;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_SBC:
            vm_decode_S(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            ry = vm_get_r(vm, inst.ry);
            // carry is modified by vm_subtract
            rd = vm_subtract(rx, ry, &vm->carry);
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_NOT:
            vm_decode_T(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            rd = ~rx;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_AND:
            vm_decode_S(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            ry = vm_get_r(vm, inst.ry);
            rd = rx & ry;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_SHL:
            vm_decode_T(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            rd = rx << inst.imm;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_SHR:
            vm_decode_T(&inst, raw);
            rx = vm_get_r(vm, inst.rx);
            rd = rx >> inst.imm;
            vm_put_r(vm, inst.rd, rd);
            break;
        case OP_JMP:
            vm_decode_V(&inst, raw);
            {
                int16_t offset = (int16_t)(inst.immx & 0x7FF);
                if (inst.immx & 0x800) offset -= 0x800;
                vm->PC += offset * 2;
            }
            break;
        case OP_JPF:
            vm_decode_Q(&inst, raw);
            rdx = vm_get_rx(vm, inst.rd);
            rdx += (int8_t)inst.imm * 2;
            vm->PC = rdx;
            break;
        case OP_JNZ:
            vm_decode_Q(&inst, raw);
            rd = vm_get_r(vm, inst.rd);
            if (rd != 0) {
                vm->PC += (int8_t)inst.imm * 2;
            }
            break;
        case OP_JPC:
            vm_decode_T(&inst, raw);
            rd = vm_get_r(vm, inst.rd);
            rx = vm_get_r(vm, inst.rx);
            if (vm_test(rd, rx, inst.imm)) {
                vm->PC += 2;
            }
            break;
        default:
            if (vm->error != NULL) {
                vm->error(VM_ERR_UNKNOWN_OP);
            }
            vm->halted = true;
            break;
    }

    if (vm->PC == PC) {
        if (vm->error != NULL) {
            vm->error(VM_ERR_JUMP_SELF);
        }
        vm->halted = true;
    }

    return !vm->halted;
}
