#include "memory.h"
#include <stdlib.h>
#include <string.h>

static uint8_t memory_read(void *ctx, uint16_t addr) {
    MemoryDevice *mem = (MemoryDevice *)ctx;
    return mem->data[addr];
}

static void memory_write(void *ctx, uint16_t addr, uint8_t data) {
    MemoryDevice *mem = (MemoryDevice *)ctx;
    if (mem->writable) {
        mem->data[addr] = data;
    }
}

static MMDevice create_device(uint16_t base, uint16_t length, bool writable) {
    MemoryDevice *mem = calloc(1, sizeof(MemoryDevice));
    if (!mem) abort();
    mem->data = calloc(1, length);
    if (!mem->data) abort();
    mem->length = length;
    mem->writable = writable;

    MMDevice dev = {0};
    dev.base = base;
    dev.length = length;
    dev.priority = 0;
    dev.ctx = mem;
    dev.read = memory_read;
    dev.write = memory_write;
    return dev;
}

MMDevice memory_device_rw(uint16_t base, uint16_t length) {
    return create_device(base, length, true);
}

MMDevice memory_device_rom(uint16_t base, uint16_t length) {
    return create_device(base, length, false);
}

void memory_device_load(MMDevice *dev, uint16_t offset, const uint8_t *data, uint16_t len) {
    MemoryDevice *mem = (MemoryDevice *)dev->ctx;
    if ((uint32_t)offset + len > mem->length) return;
    memcpy(mem->data + offset, data, len);
}

void memory_device_destroy(MMDevice *dev) {
    MemoryDevice *mem = (MemoryDevice *)dev->ctx;
    free(mem->data);
    free(mem);
    dev->ctx = NULL;
}
