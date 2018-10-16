#include <stdio.h>

#include "vm.h"
#include "string.h"

int main() {
    VM* vm = new_vm();

    uint8_t data[] = {
        0x31, 0x05,
        0x32, 0x02,
        0x73, 0x12
    };
    memcpy(&vm->M, data, 6);

    printf("%i\n", (uint8_t)0x101);

    while(!vm->halted) {
        vm_step(vm);
    }

    free_vm(vm);
    return 0;
}