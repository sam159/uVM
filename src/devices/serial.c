#include "serial.h"
#include <stdlib.h>

static uint8_t serial_status(const SerialDevice *s) {
    uint8_t status = SERIAL_STATUS_TX_READY;
    status |= s->count == 0 ? SERIAL_STATUS_RX_EMPTY : SERIAL_STATUS_RX_READY;
    return status;
}

static uint8_t serial_read(void *ctx, uint16_t addr) {
    SerialDevice *s = ctx;
    switch (addr) {
        case 0x0:
            return serial_status(s);
        case 0x1:
            return 0x00;
        default:
            return 0x00;
    }
}

static void serial_write(void *ctx, uint16_t addr, uint8_t data) {
    (void)ctx;
    (void)addr;
    (void)data;
}

MMDevice serial_device_create(uint16_t base, int in_fd, int out_fd) {
    SerialDevice *s = calloc(1, sizeof(SerialDevice));
    if (!s) abort();
    s->in_fd = in_fd;
    s->out_fd = out_fd;

    MMDevice dev = {0};
    dev.base = base;
    dev.length = 2;
    dev.priority = 1;
    dev.ctx = s;
    dev.read = serial_read;
    dev.write = serial_write;
    return dev;
}

void serial_device_destroy(MMDevice *dev) {
    free(dev->ctx);
    dev->ctx = NULL;
}
