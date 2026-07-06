#include <stdio.h>
#include "vm.h"

static uint8_t test_mem[0x10000];
static uint8_t last_error = 0;

static uint8_t test_read(uint16_t addr, bool instruction) {
    return test_mem[addr];
}

static void test_write(uint16_t addr, uint8_t data) {
    test_mem[addr] = data;
}

static void test_error(uint8_t err) {
    last_error = err;
}

int main(void) {
    VM *vm = new_vm();
    vm->readAddr = test_read;
    vm->writeAddr = test_write;
    vm->error = test_error;

    // Test 1: BIT AND (op=0) - R1 = R1 & R2
    // T-format: [0x9:4][rd:4][rx:4][imm4:4] = 0x9120 (rd=1, rx=2, imm=0)
    test_mem[0] = 0x91; test_mem[1] = 0x20;
    vm_reset(vm);
    vm_put_r(vm, 1, 0xFF);
    vm_put_r(vm, 2, 0x0F);
    vm_step(vm);
    if (vm_get_r(vm, 1) != 0x0F) {
        printf("FAIL: BIT AND expected R1=0x0F, got 0x%02X\n", vm_get_r(vm, 1));
        return 1;
    }

    // Test 2: BIT OR (op=1) - R1 = R1 | R2
    // T-format: [0x9:4][rd:4][rx:4][imm4:4] = 0x9121 (rd=1, rx=2, imm=1)
    test_mem[0] = 0x91; test_mem[1] = 0x21;
    vm_reset(vm);
    vm_put_r(vm, 1, 0xF0);
    vm_put_r(vm, 2, 0x0F);
    vm_step(vm);
    if (vm_get_r(vm, 1) != 0xFF) {
        printf("FAIL: BIT OR expected R1=0xFF, got 0x%02X\n", vm_get_r(vm, 1));
        return 1;
    }

    // Test 3: BIT XOR (op=2) - R1 = R1 ^ R2
    // T-format: [0x9:4][rd:4][rx:4][imm4:4] = 0x9122 (rd=1, rx=2, imm=2)
    test_mem[0] = 0x91; test_mem[1] = 0x22;
    vm_reset(vm);
    vm_put_r(vm, 1, 0xFF);
    vm_put_r(vm, 2, 0x0F);
    vm_step(vm);
    if (vm_get_r(vm, 1) != 0xF0) {
        printf("FAIL: BIT XOR expected R1=0xF0, got 0x%02X\n", vm_get_r(vm, 1));
        return 1;
    }

    // Test 4: BIT MOD (op=4) - R1 = R1 % R2
    // T-format: [0x9:4][rd:4][rx:4][imm4:4] = 0x9124 (rd=1, rx=2, imm=4)
    test_mem[0] = 0x91; test_mem[1] = 0x24;
    vm_reset(vm);
    vm_put_r(vm, 1, 10);
    vm_put_r(vm, 2, 3);
    vm_step(vm);
    if (vm_get_r(vm, 1) != 1) {
        printf("FAIL: BIT MOD expected R1=1, got %u\n", vm_get_r(vm, 1));
        return 1;
    }

    // Test 5: Invalid BIT operation (op=3) should trigger error
    // T-format: [0x9:4][rd:4][rx:4][imm4:4] = 0x9123 (rd=1, rx=2, imm=3)
    test_mem[0] = 0x91; test_mem[1] = 0x23;
    vm_reset(vm);
    last_error = 0;
    vm_put_r(vm, 1, 0xFF);
    vm_put_r(vm, 2, 0x0F);
    vm_step(vm);
    if (!vm->halted) {
        printf("FAIL: Invalid BIT operation should halt VM\n");
        return 1;
    }
    if (last_error != VM_ERR_INVALID_BIT_OP) {
        printf("FAIL: Expected error code 0x%02X, got 0x%02X\n", VM_ERR_INVALID_BIT_OP, last_error);
        return 1;
    }

    printf("PASS: BIT instruction VM tests\n");
    free_vm(vm);
    return 0;
}
