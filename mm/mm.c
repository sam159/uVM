#include "mm.h"
#include <stdlib.h>

MMManager *mm_create(void) {
    MMManager *mm = calloc(1, sizeof(MMManager));
    return mm;
}

void mm_free(MMManager *mm) {
    free(mm);
}

bool mm_register(MMManager *mm, MMDevice device) {
    (void)mm;
    (void)device;
    return false;
}

uint8_t mm_read(MMManager *mm, uint16_t addr, bool instruction) {
    (void)mm;
    (void)addr;
    (void)instruction;
    return 0;
}

void mm_write(MMManager *mm, uint16_t addr, uint8_t data) {
    (void)mm;
    (void)addr;
    (void)data;
}
