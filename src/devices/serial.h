#ifndef UVM_SERIAL_DEVICE_H
#define UVM_SERIAL_DEVICE_H

#include "mm.h"

#define SERIAL_RX_CAPACITY 32

#define SERIAL_STATUS_RX_READY     0x01
#define SERIAL_STATUS_RX_EMPTY     0x02
#define SERIAL_STATUS_TX_READY     0x04

typedef struct SerialDevice_t {
    int     in_fd;
    int     out_fd;
    uint8_t buffer[SERIAL_RX_CAPACITY];
    uint8_t head;   /* next byte to consume */
    uint8_t count;  /* bytes currently buffered */
} SerialDevice;

MMDevice serial_device_create(uint16_t base, int in_fd, int out_fd);
void     serial_device_destroy(MMDevice *dev);

#endif
