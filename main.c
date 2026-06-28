#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "vm.h"

static uint8_t memory[VM_MEM_SIZE + 1];

static uint16_t load_addrs[32];
static long load_sizes[32];
static int load_count = 0;

static int overlaps(uint16_t addr, long size) {
    for (int i = 0; i < load_count; i++) {
        if (addr < (uint16_t)(load_addrs[i] + load_sizes[i]) && load_addrs[i] < addr + size) {
            return 1;
        }
    }
    return 0;
}

static int load_binary(const char *path, uint16_t addr) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Error: cannot open '%s': %s\n", path, strerror(errno));
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (addr + size > VM_MEM_SIZE) {
        fprintf(stderr, "Error: '%s' overflows memory at 0x%04X (%ld bytes)\n", path, addr, size);
        fclose(f);
        return -1;
    }

    if (overlaps(addr, size)) {
        fprintf(stderr, "Error: '%s' at 0x%04X overlaps with a previously loaded file\n", path, addr);
        fclose(f);
        return -1;
    }

    size_t read = fread(memory + addr, 1, size, f);
    fclose(f);

    if ((long)read != size) {
        fprintf(stderr, "Error: failed to read '%s'\n", path);
        return -1;
    }

    if (load_count < 32) {
        load_addrs[load_count] = addr;
        load_sizes[load_count] = size;
        load_count++;
    }

    printf("Loaded '%s' at 0x%04X (%ld bytes)\n", path, addr, size);
    return 0;
}

static void print_usage(const char *prog) {
    const char *basename = strrchr(prog, '/');
    if (basename) {
        basename++;
    } else {
        basename = strrchr(prog, '\\');
        if (basename) {
            basename++;
        } else {
            basename = prog;
        }
    }

    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s program.bin                          # load at 0x0000\n", basename);
    fprintf(stderr, "  %s -a 0x100 file1.bin -a 0x200 file2.bin  # multiple files at absolute addresses\n", basename);
    fprintf(stderr, "  %s -p 0x100 program.bin                 # set PC to 0x100\n", basename);
}

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

int main(int argc, char *argv[]) {
    VM *vm = new_vm();

    vm->readAddr = host_read;
    vm->writeAddr = host_write;
    vm->syscall = host_syscall;
    vm->error = host_error;

    uint16_t pc = 0;

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: missing address after -p\n");
                return 1;
            }
            i++;
            char *end;
            long val = strtol(argv[i], &end, 16);
            if (*end || val < 0) {
                fprintf(stderr, "Error: invalid hex address '%s'\n", argv[i]);
                return 1;
            }
            pc = (uint16_t)val;
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 2 >= argc) {
                fprintf(stderr, "Error: missing address and file after -a\n");
                return 1;
            }
            i++;
            char *end;
            long val = strtol(argv[i], &end, 16);
            if (*end || val < 0) {
                fprintf(stderr, "Error: invalid hex address '%s'\n", argv[i]);
                return 1;
            }
            i++;
            if (load_binary(argv[i], (uint16_t)val) != 0) return 1;
        } else {
            if (load_binary(argv[i], 0) != 0) return 1;
        }
    }

    vm->PC = pc;
    vm->halted = false;

    while (!vm->halted) {
        vm_step(vm);
    }

    vm_print_state(vm);
    free_vm(vm);
    return 0;
}
