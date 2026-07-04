#include <stdio.h>
#include "mm.h"

static uint8_t test_read(void *ctx, uint16_t addr) {
    uint8_t *data = (uint8_t *)ctx;
    return data[addr];
}

static void test_write(void *ctx, uint16_t addr, uint8_t val) {
    uint8_t *data = (uint8_t *)ctx;
    data[addr] = val;
}

int main(void) {
    MMManager *mm = mm_create();

    uint8_t buf1[256] = {0};
    MMDevice dev1 = {0};
    dev1.base = 0x0000;
    dev1.length = 0x0100;
    dev1.priority = 0;
    dev1.ctx = buf1;
    dev1.read = test_read;
    dev1.write = test_write;

    if (!mm_register(mm, dev1)) {
        printf("FAIL: register device 1\n");
        return 1;
    }

    mm_write(mm, 0x0010, 0xAB);
    if (mm_read(mm, 0x0010, false) != 0xAB) {
        printf("FAIL: read/write through manager\n");
        return 1;
    }

    if (mm_read(mm, 0x0200, false) != 0x00) {
        printf("FAIL: unmapped read should return 0x00\n");
        return 1;
    }

    uint8_t buf2[256] = {0};
    MMDevice dev2 = {0};
    dev2.base = 0x0000;
    dev2.length = 0x0100;
    dev2.priority = 1;
    dev2.ctx = buf2;
    dev2.read = test_read;
    dev2.write = test_write;

    if (!mm_register(mm, dev2)) {
        printf("FAIL: register device 2\n");
        return 1;
    }

    mm_write(mm, 0x0010, 0xCD);
    if (buf1[0x10] != 0xAB) {
        printf("FAIL: lower priority device should not be written\n");
        return 1;
    }
    if (buf2[0x10] != 0xCD) {
        printf("FAIL: higher priority device should be written\n");
        return 1;
    }

    mm_free(mm);

    printf("PASS: memory manager tests\n");
    return 0;
}
