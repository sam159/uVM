#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <poll.h>
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

    /* A buffered byte becomes visible and is consumed once */
    if (write(in_pipe[1], "A", 1) != 1) { perror("write"); return 1; }
    status = dev.read(dev.ctx, 0x0);
    CHECK((status & SERIAL_STATUS_RX_READY) && !(status & SERIAL_STATUS_RX_EMPTY),
          "RX ready after input");
    CHECK(dev.read(dev.ctx, 0x1) == 'A', "DATA returns buffered byte");
    status = dev.read(dev.ctx, 0x0);
    CHECK(!(status & SERIAL_STATUS_RX_READY) && (status & SERIAL_STATUS_RX_EMPTY),
          "RX empty after drain");
    CHECK(dev.read(dev.ctx, 0x1) == 0x00, "DATA returns 0 when drained");

    /* FIFO order */
    if (write(in_pipe[1], "XYZ", 3) != 3) { perror("write"); return 1; }
    CHECK(dev.read(dev.ctx, 0x1) == 'X' &&
          dev.read(dev.ctx, 0x1) == 'Y' &&
          dev.read(dev.ctx, 0x1) == 'Z', "RX preserves order");
    CHECK(dev.read(dev.ctx, 0x1) == 0x00 &&
          (dev.read(dev.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY), "RX empty after FIFO drain");

    /* A burst larger than the buffer drains in order through a wrap */
    uint8_t burst[SERIAL_RX_CAPACITY + 8];
    for (int i = 0; i < (int)sizeof burst; i++) burst[i] = (uint8_t)i;
    if (write(in_pipe[1], burst, sizeof burst) != (ssize_t)sizeof burst) { perror("write"); return 1; }
    bool ordered = true;
    for (int i = 0; i < (int)sizeof burst; i++) {
        if (dev.read(dev.ctx, 0x1) != burst[i]) ordered = false;
    }
    CHECK(ordered, "burst longer than buffer drains in order");
    CHECK(dev.read(dev.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY, "RX empty after burst drain");

    /* An unusable input fd is treated as no input */
    MMDevice bad = serial_device_create(0x0010, -1, out_pipe[1]);
    CHECK(bad.read(bad.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY, "invalid input fd reports empty");
    serial_device_destroy(&bad);

    /* A closed but valid fd makes poll/read fail; that must count as no data */
    int ghost = dup(in_pipe[0]);
    close(ghost);
    MMDevice closed = serial_device_create(0x0010, ghost, out_pipe[1]);
    CHECK(closed.read(closed.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY, "closed input fd reports empty");
    serial_device_destroy(&closed);

    /* EOF leaves the receive side empty */
    close(in_pipe[1]);
    CHECK(dev.read(dev.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY, "EOF leaves RX empty");

    /* DATA writes reach the output pipe unmodified */
    struct pollfd out_poll = { .fd = out_pipe[0], .events = POLLIN };
    dev.write(dev.ctx, 0x1, 'B');
    dev.write(dev.ctx, 0x1, 0x00);
    int out_ready = poll(&out_poll, 1, 1000);
    CHECK(out_ready == 1, "DATA write reaches output");
    if (out_ready == 1) {
        uint8_t out[2];
        CHECK(read(out_pipe[0], out, 1) == 1 && out[0] == 'B', "first output byte correct");
        CHECK(read(out_pipe[0], out, 1) == 1 && out[0] == 0x00, "output byte not modified");
    }

    /* STATUS writes emit nothing */
    dev.write(dev.ctx, 0x0, 0xFF);
    CHECK(poll(&out_poll, 1, 0) == 0, "STATUS writes emit nothing");

    serial_device_destroy(&dev);
    CHECK(dev.ctx == NULL, "destroy clears context");

    close(in_pipe[0]);
    close(out_pipe[0]);
    close(out_pipe[1]);

    return failures ? 1 : 0;
}
