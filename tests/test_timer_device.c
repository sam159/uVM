#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "mm.h"
#include "timer.h"

static int failures = 0;

static double elapsed_us(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1e6 + (end->tv_nsec - start->tv_nsec) / 1e3;
}

int main(void) {
    MMDevice dev = timer_device_create(0x8000);

    if (dev.read(dev.ctx, 0x0) != 0 || dev.read(dev.ctx, 0x1) != 0 || dev.read(dev.ctx, 0x2) != 0) {
        printf("FAIL: initial registers should be 0\n"); failures++;
    } else {
        printf("PASS: initial registers are 0\n");
    }

    dev.write(dev.ctx, 0x1, 0xAB);
    dev.write(dev.ctx, 0x2, 0xCD);
    if (dev.read(dev.ctx, 0x1) != 0xAB || dev.read(dev.ctx, 0x2) != 0xCD) {
        printf("FAIL: timer_h/timer_l write+read\n"); failures++;
    } else {
        printf("PASS: timer_h/timer_l write+read\n");
    }

    dev.write(dev.ctx, 0x0, 0x06);
    if (dev.read(dev.ctx, 0x0) != 0x06) {
        printf("FAIL: control R/W bits write+read\n"); failures++;
    } else {
        printf("PASS: control R/W bits write+read\n");
    }
    dev.write(dev.ctx, 0x1, 0x00);
    dev.write(dev.ctx, 0x2, 0x00);
    dev.write(dev.ctx, 0x0, 0x07);
    if (dev.read(dev.ctx, 0x0) != 0x06) {
        printf("FAIL: start bit should read as 0\n"); failures++;
    } else {
        printf("PASS: start bit reads as 0\n");
    }

    dev.write(dev.ctx, 0x0, 0x00);
    dev.write(dev.ctx, 0x1, 0x03);
    dev.write(dev.ctx, 0x2, 0xE8);
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    dev.write(dev.ctx, 0x0, 0x01);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double us = elapsed_us(&t1, &t2);
    if (us >= 500 && us <= 1500) {
        printf("PASS: timer blocked ~1000us (measured %.0fus)\n", us);
    } else {
        printf("FAIL: timer blocked %.0fus, expected ~1000us\n", us); failures++;
    }

    dev.write(dev.ctx, 0x0, 0x00);
    dev.write(dev.ctx, 0x1, 0x03);
    dev.write(dev.ctx, 0x2, 0xE8);
    dev.write(dev.ctx, 0x0, 0x03);
    if (dev.read(dev.ctx, 0x1) != 0 || dev.read(dev.ctx, 0x2) != 0) {
        printf("FAIL: reset-after-run should zero timer_h/timer_l\n"); failures++;
    } else {
        printf("PASS: reset-after-run zeros timer_h/timer_l\n");
    }

    dev.write(dev.ctx, 0x0, 0x00);
    dev.write(dev.ctx, 0x1, 0x00);
    dev.write(dev.ctx, 0x2, 0x01);
    dev.write(dev.ctx, 0x0, 0x04);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    dev.write(dev.ctx, 0x0, 0x05);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    us = elapsed_us(&t1, &t2);
    if (us >= 500 && us <= 2500) {
        printf("PASS: millisecond mode blocked ~1ms (measured %.0fus)\n", us);
    } else {
        printf("FAIL: millisecond mode blocked %.0fus, expected ~1000us\n", us); failures++;
    }

    timer_device_destroy(&dev);

    return failures ? 1 : 0;
}
