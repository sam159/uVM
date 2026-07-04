#include "mm.h"
#include <stdlib.h>

MMManager *mm_create(void) {
    return calloc(1, sizeof(MMManager));
}

void mm_free(MMManager *mm) {
    free(mm);
}

bool mm_register(MMManager *mm, MMDevice device) {
    if (mm->count >= MM_MAX_DEVICES) {
        return false;
    }
    mm->devices[mm->count] = device;
    mm->count++;
    return true;
}

uint8_t mm_read(MMManager *mm, uint16_t addr, bool instruction) {
    (void)instruction;
    MMDevice *best = NULL;
    for (uint8_t i = 0; i < mm->count; i++) {
        MMDevice *d = &mm->devices[i];
        if (addr >= d->base && (uint32_t)addr < (uint32_t)d->base + d->length) {
            if (!best || d->priority > best->priority) {
                best = d;
            }
        }
    }
    if (!best) return 0x00;
    return best->read(best->ctx, addr - best->base);
}

void mm_write(MMManager *mm, uint16_t addr, uint8_t data) {
    MMDevice *best = NULL;
    for (uint8_t i = 0; i < mm->count; i++) {
        MMDevice *d = &mm->devices[i];
        if (addr >= d->base && (uint32_t)addr < (uint32_t)d->base + d->length) {
            if (!best || d->priority > best->priority) {
                best = d;
            }
        }
    }
    if (!best) return;
    best->write(best->ctx, addr - best->base, data);
}
