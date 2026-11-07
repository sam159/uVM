#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CTRL_START   0x01
#define CTRL_RESET   0x02
#define CTRL_MILLIS  0x04

static uint8_t timer_read(void *ctx, uint16_t addr) {
    const TimerDevice *t = ctx;
    switch (addr) {
        case 0x0: return t->control & ~CTRL_START;
        case 0x1: return t->timer_h;
        case 0x2: return t->timer_l;
        default:  return 0;
    }
}

static void timer_write(void *ctx, uint16_t addr, uint8_t data) {
    TimerDevice *t = ctx;
    switch (addr) {
        case 0x0: {
            // update control register, masking off unused bits;
            t->control = t->control & ~0x06 | data & 0x06;
            if (data & CTRL_START) {
                const uint16_t duration = (uint16_t)t->timer_h << 8 | t->timer_l;
                if (t->control & CTRL_MILLIS) {
#ifdef TIMER_DEBUG
                    printf("Sleeping for %d milliseconds\n", duration);
#endif
                    usleep((useconds_t)duration * 1000);
                } else {
#ifdef TIMER_DEBUG
                    printf("Sleeping for %d microseconds\n", duration);
#endif
                    usleep(duration);
                }
                if (t->control & CTRL_RESET) {
                    t->timer_h = 0;
                    t->timer_l = 0;
                }
            }
            break;
        }
        case 0x1:
            t->timer_h = data;
#ifdef TIMER_DEBUG
            printf("Timer set to %d\n",  (uint16_t)t->timer_h << 8 | t->timer_l);
#endif
            break;
        case 0x2:
            t->timer_l = data;
#ifdef TIMER_DEBUG
            printf("Timer set to %d\n",  (uint16_t)t->timer_h << 8 | t->timer_l);
#endif
            break;
        default:
            break;
    }
}

MMDevice timer_device_create(const uint16_t base) {
    TimerDevice *t = calloc(1, sizeof(TimerDevice));
    if (!t) abort();

    MMDevice dev = {0};
    dev.base = base;
    dev.length = 3;
    dev.priority = 1;
    dev.ctx = t;
    dev.read = timer_read;
    dev.write = timer_write;
    return dev;
}

void timer_device_destroy(MMDevice *dev) {
    free(dev->ctx);
    dev->ctx = NULL;
}
