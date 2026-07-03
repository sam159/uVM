#include <stdio.h>
#include "vm.h"

static uint8_t test_mem[0x10000];

static uint8_t test_read(uint16_t addr, bool instruction) {
    return test_mem[addr];
}

static void test_write(uint16_t addr, uint8_t data) {
    test_mem[addr] = data;
}

int main(void) {
    VM *vm = new_vm();
    vm->readAddr = test_read;
    vm->writeAddr = test_write;

    // Test 1: HLT R0, 0 -> haltCode = 0
    test_mem[0] = 0x00; test_mem[1] = 0x00;  // HLT R0, 0
    vm_reset(vm);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 0) {
        printf("FAIL: HLT R0, 0 expected haltCode=0, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 2: HLT R0, 2 -> haltCode = 2
    test_mem[0] = 0x00; test_mem[1] = 0x02;  // HLT R0, 2
    vm_reset(vm);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 2) {
        printf("FAIL: HLT R0, 2 expected haltCode=2, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 3: HLT R1, 0 with R1=5 -> haltCode = 5
    test_mem[0] = 0x01; test_mem[1] = 0x00;  // HLT R1, 0
    vm_reset(vm);
    vm_put_r(vm, 1, 5);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 5) {
        printf("FAIL: HLT R1, 0 with R1=5 expected haltCode=5, got %u\n", vm->haltCode);
        return 1;
    }

    // Test 4: HLT R1, 0x42 with R1=1 -> haltCode = 0x43
    test_mem[0] = 0x01; test_mem[1] = 0x42;  // HLT R1, 0x42
    vm_reset(vm);
    vm_put_r(vm, 1, 1);
    vm_step(vm);
    if (!vm->halted || vm->haltCode != 0x43) {
        printf("FAIL: HLT R1, 0x42 with R1=1 expected haltCode=0x43, got %u\n", vm->haltCode);
        return 1;
    }

    printf("PASS: HLT Q format tests\n");
    free_vm(vm);
    return 0;
}
