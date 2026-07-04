#ifndef UVM_MM_H
#define UVM_MM_H

#include <stdint.h>
#include <stdbool.h>

#define MM_MAX_DEVICES 16

typedef struct MMDevice_t {
    uint16_t base;
    uint16_t length;
    uint8_t  priority;
    void    *ctx;
    uint8_t (*read)(void *ctx, uint16_t addr);
    void    (*write)(void *ctx, uint16_t addr, uint8_t data);
} MMDevice;

typedef struct MMManager_t {
    MMDevice devices[MM_MAX_DEVICES];
    uint8_t  count;
} MMManager;

MMManager *mm_create(void);
void       mm_free(MMManager *mm);
bool       mm_register(MMManager *mm, MMDevice device);
uint8_t    mm_read(MMManager *mm, uint16_t addr, bool instruction);
void       mm_write(MMManager *mm, uint16_t addr, uint8_t data);

#endif
