#include <stdio.h>
#include <string.h>

#include "vm.h"

static uint8_t memory[VM_MEM_SIZE + 1];

static uint8_t host_read(uint16_t addr, bool instruction) {
    return memory[addr];
}

static void host_write(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

static uint8_t host_syscall(VM *vm, uint8_t callno, uint8_t imm) {
    switch (callno) {
        case 1:
            printf("R%d: %d\n", vm->R[4] >> 8, vm->R[4] & 0xFF);
            return 0;
        default:
            return 0;
    }
}

static void host_error(uint8_t err) {
    switch (err) {
        case VM_ERR_MISALIGN:
            printf("Error: instruction misaligned\n");
            break;
        case VM_ERR_UNKNOWN_OP:
            printf("Error: unknown opcode\n");
            break;
        case VM_ERR_OUT_OF_BOUNDS:
            printf("Error: out of bounds memory access\n");
            break;
        default:
            printf("Error: %d\n", err);
            break;
    }
}

static void vm_print_state(VM *vm) {
    printf("PC: %04X  carry: %d  halted: %d\n", vm->PC, vm->carry, vm->halted);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            uint16_t addr = i+j*4;
            printf("R%X: %02X (%03d) ", i, vm->R[addr], vm->R[addr]);
        }
        printf("\n");
    }

    printf("RX%X: %02X (%05d) ", 0xB, vm_get_rx(vm, 0xB), vm_get_rx(vm, 0xB));
    printf("RX%X: %02X (%05d)\n", 0xE, vm_get_rx(vm, 0xE), vm_get_rx(vm, 0xE));
    printf("RX%X: %02X (%05d) ", 0xC, vm_get_rx(vm, 0xC), vm_get_rx(vm, 0xC));
    printf("RX%X: %02X (%05d)\n", 0xF, vm_get_rx(vm, 0xF), vm_get_rx(vm, 0xF));
    printf("RX%X: %02X (%05d)\n", 0xD, vm_get_rx(vm, 0xD), vm_get_rx(vm, 0xD));
}

int main() {
    VM *vm = new_vm();

    vm->readAddr = host_read;
    vm->writeAddr = host_write;
    vm->syscall = host_syscall;
    vm->error = host_error;

    uint8_t data[] = {
        0x31, 0x05,
        0x32, 0x02,
        0x73, 0x12
    };
    memcpy(memory, data, sizeof(data));

    vm->PC = 0;
    vm->halted = false;

    while (!vm->halted) {
        vm_step(vm);
    }

    vm_print_state(vm);
    free_vm(vm);
    return 0;
}
