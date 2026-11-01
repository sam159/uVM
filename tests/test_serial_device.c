#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "mm.h"
#include "serial.h"

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } \
    else { printf("PASS: %s\n", msg); } \
} while (0)

int main(void) {
    int in_pipe[2];
    int out_pipe[2];
    if (pipe(in_pipe) != 0 || pipe(out_pipe) != 0) { perror("pipe"); return 1; }

    MMDevice dev = serial_device_create(0x0010, in_pipe[0], out_pipe[1]);
    CHECK(dev.base == 0x0010 && dev.length == 2 && dev.priority == 1, "device geometry");

    uint8_t status = dev.read(dev.ctx, 0x0);
    CHECK(status & SERIAL_STATUS_RX_EMPTY, "initial RX buffer reported empty");
    CHECK(!(status & SERIAL_STATUS_RX_READY), "initial RX not ready");
    CHECK(status & SERIAL_STATUS_TX_READY, "TX always ready");
    CHECK(!(status & SERIAL_STATUS_TX_NOT_EMPTY), "TX never pending");

    dev.write(dev.ctx, 0x0, 0xFF);
    CHECK(dev.read(dev.ctx, 0x0) == status, "STATUS writes ignored");

    CHECK(dev.read(dev.ctx, 0x1) == 0x00, "DATA read with empty buffer returns 0");

    serial_device_destroy(&dev);
    CHECK(dev.ctx == NULL, "destroy clears context");

    close(in_pipe[0]);
    close(in_pipe[1]);
    close(out_pipe[0]);
    close(out_pipe[1]);

    return failures ? 1 : 0;
}
