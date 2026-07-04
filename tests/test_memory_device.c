#include <stdio.h>
#include <string.h>
#include "memory.h"

int main(void) {
    MMDevice dev = memory_device_rw(0x0000, 0x1000);

    uint8_t test_data[] = {0x12, 0x34, 0x56};
    memory_device_load(&dev, 0x100, test_data, 3);

    if (dev.read(dev.ctx, 0x100) != 0x12) {
        printf("FAIL: RW device load+read offset 0x100\n");
        return 1;
    }
    if (dev.read(dev.ctx, 0x101) != 0x34) {
        printf("FAIL: RW device load+read offset 0x101\n");
        return 1;
    }

    dev.write(dev.ctx, 0x200, 0xAB);
    if (dev.read(dev.ctx, 0x200) != 0xAB) {
        printf("FAIL: RW device write+read\n");
        return 1;
    }

    memory_device_destroy(&dev);

    MMDevice rom = memory_device_rom(0x0000, 0x1000);
    uint8_t rom_data[] = {0xDE, 0xAD};
    memory_device_load(&rom, 0x000, rom_data, 2);

    if (rom.read(rom.ctx, 0x000) != 0xDE) {
        printf("FAIL: ROM device load+read\n");
        return 1;
    }

    rom.write(rom.ctx, 0x000, 0xFF);
    if (rom.read(rom.ctx, 0x000) != 0xDE) {
        printf("FAIL: ROM device write should be ignored\n");
        return 1;
    }

    memory_device_destroy(&rom);

    printf("PASS: memory device tests\n");
    return 0;
}
