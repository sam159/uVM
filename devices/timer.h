#ifndef UVM_TIMER_DEVICE_H
#define UVM_TIMER_DEVICE_H

#include "mm.h"

typedef struct TimerDevice_t {
    uint8_t control;
    uint8_t timer_h;
    uint8_t timer_l;
} TimerDevice;

MMDevice timer_device_create(uint16_t base);
void     timer_device_destroy(MMDevice *dev);

#endif
