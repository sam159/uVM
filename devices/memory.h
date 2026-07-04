#ifndef UVM_MEMORY_DEVICE_H
#define UVM_MEMORY_DEVICE_H

#include "mm.h"

typedef struct MemoryDevice_t {
    uint8_t *data;
    uint16_t length;
    bool     writable;
} MemoryDevice;

MMDevice memory_device_rw(uint16_t base, uint16_t length);
MMDevice memory_device_rom(uint16_t base, uint16_t length);
void     memory_device_load(MMDevice *dev, uint16_t offset, const uint8_t *data, uint16_t len);
void     memory_device_destroy(MMDevice *dev);

#endif
