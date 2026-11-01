#include "serial.h"
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <unistd.h>

static uint8_t serial_status(const SerialDevice *s) {
    uint8_t status = SERIAL_STATUS_TX_READY;
    status |= s->count == 0 ? SERIAL_STATUS_RX_EMPTY : SERIAL_STATUS_RX_READY;
    return status;
}

static void serial_refill(SerialDevice *s) {
    if (s->count >= SERIAL_RX_CAPACITY) return;

    struct pollfd pfd = { .fd = s->in_fd, .events = POLLIN };
    if (poll(&pfd, 1, 0) <= 0) return;

    uint8_t start = (uint8_t)((s->head + s->count) % SERIAL_RX_CAPACITY);
    uint8_t space = (uint8_t)(SERIAL_RX_CAPACITY - s->count);
    uint8_t contiguous = (uint8_t)(SERIAL_RX_CAPACITY - start);
    if (contiguous > space) contiguous = space;

    ssize_t n = read(s->in_fd, s->buffer + start, contiguous);
    if (n > 0) s->count = (uint8_t)(s->count + n);
}

static uint8_t serial_read(void *ctx, uint16_t addr) {
    SerialDevice *s = ctx;
    serial_refill(s);
    switch (addr) {
        case 0x0:
            return serial_status(s);
        case 0x1: {
            if (s->count == 0) return 0x00;
            uint8_t data = s->buffer[s->head];
            s->head = (uint8_t)((s->head + 1) % SERIAL_RX_CAPACITY);
            s->count--;
            return data;
        }
        default:
            return 0x00;
    }
}

static void serial_write(void *ctx, uint16_t addr, uint8_t data) {
    SerialDevice *s = ctx;
    if (addr != 0x1) return;
    while (write(s->out_fd, &data, 1) < 0 && errno == EINTR) {
    }
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
