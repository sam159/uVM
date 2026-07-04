# Memory Manager for uVM

(framework implemented)

Replace the flat memory region currently in use with a memory manager that can map memory reads and writes to devices.
Devices may overlap but can only provide one contiguous region; to accommodate this each will have a priority when added.

Devices include
- Memory R/W and R/O
- Serial device/console providing access to stdin and stdout
- Timer device
- Disk device mapped to a file

This replaces the current static allocated memory region in main.

## Devices

Devices appear to the memory manager only as an address, length, and methods to read and write.
The reading and writing methods will take addresses relative to their start address and not the main memory space.

By default, the following is loaded
- Memory R/W at 0x0 of length 0xFFFF
- Serial device on stdin and stdout at 0x10
- Timer device at 0x20

### Memory Device (implemented)

The most basic and essential. This has a length and a flag to allow program writes. For read-only writing is allowed 
at initialisation time to enable roms/static data to be loaded. Writing should be gated in the device, not the memory manager.

### Timer Device (implemented)

This device is to allow the VM to pause execution while waiting for devices to become ready. Address is offset by its location in the manager.

A single 16-bit timer value is available to set. The timer duration is in microseconds unless bit 3 of the control flags
is set which switches to milliseconds. 
Starting the timer is triggered by setting a bit in the control flags. The next instruction after setting the start bit 
of the control flags does not execute until the timer runs to zero.

| Address | Name      | Description               |
|---------|-----------|---------------------------|
| 0x0     | `CONTROL` | Control flags             |
| 0x1     | `TIMER_H` | Timer duration. high byte |
| 0x2     | `TIMER_L` | Timer duration. low byte  |

#### Control flag bits

Unused bits always read zero, writes have no effect.

|             | 8      | 7      | 6      | 5      | 4      | 3           | 2               | 1     |
|-------------|--------|--------|--------|--------|--------|-------------|-----------------|-------|
| Description | Unused | Unused | Unused | Unused | Unused | Miliseconds | Reset after run | Start |
| Mode        |        |        |        |        |        | R/W         | R/W             | W/O   |
| Default     |        |        |        |        |        | 0           | 0               | 0     |

- If bit two is set, `TIMER_H` and `TIMER_L` are set to zero after the timer has run.
- If bit three is set, the timer is measured in milliseconds instead of microseconds.

### Serial Device

This is a simple UART style serial interface with a small internal buffer. Address is offset by its location in the manager.

| Address | Name     | Description             |
|---------|----------|-------------------------|
| 0x0     | `STATUS` | Status flags            |
| 0x1     | `DATA`   | Read or write next byte |

#### Status flag bits

Unused bits always read zero, writes have no effect.

| 8      | 7      | 6      | 5      | 4                   | 3        | 2               | 1        |
|--------|--------|--------|--------|---------------------|----------|-----------------|----------|
| Unused | Unused | Unused | Unused | TX Buffer Not Empty | TX Ready | RX Buffer Empty | RX Ready |
|        |        |        |        | R/O                 | R/O      | R/0             | R/O      |

- Bit 4 is set while there is data in the buffer being sent to allow loops to wait for it to empty
- Bit 2 is clear when the receive buffer is empty again to allow looping until empty

### Disk device

This allows R/W access to a file of fixed size optionally read-only. Access is via control registers for seeking, reading, and writing.
Presented as eight bytes of memory offset by its location in memory.

The disk is presented as blocks of data, blocks are uniform and can be up-to 64KB in size with up-to 256 blocks representing 16MB of 
addressable disk space.

Selecting a block beyond the available blocks will cause the device to go offline with an out-of-bounds error.

Seeking is achieved by writing to `ADDR_H` first, then `ADDR_L` which performs the seek. `ADDR_H` and `ADDR_L` are updated
on any read or write of `DATA`. Trying to seek an out-of-bounds address is an error and causes the device to go offline.

Reading and writing `DATA` will advance the current address, if new address is beyond the current block, the address is 
reset, and the block number is incremented, if the new address is out-of-bounds (beyond the last address of the last block)
this will cause an error and offline the device.

| Address | Name           | Mode | Description                                                             |
|---------|----------------|------|-------------------------------------------------------------------------|
| 0x0     | `STATUS`       | R/W  | Status flags                                                            |
| 0x1     | `DATA`         | R/W  | Read or writes data at current address                                  |
| 0x2     | `BLOCK_INDEX`  | R/W  | current block selected                                                  |
| 0x3     | `BLOCK_COUNT`  | R/O  | Number of blocks on disk                                                |
| 0x4     | `BLOCK_SIZE_H` | R/O  | size of each block. high byte                                           |
| 0x5     | `BLOCK_SIZE_L` | R/O  | low byte of size                                                        |
| 0x6     | `ADDR_H`       | R/W  | read/write current address within block. Seek done on write of low byte |
| 0x7     | `ADDR_L`       | R/W  | low byte of address                                                     |

#### Status flags

Unused bits always read zero, writes have no effect.

| 8      | 7      | 6      | 5     | 4           | 3           | 2         | 1      |
|--------|--------|--------|-------|-------------|-------------|-----------|--------|
| Unused | Unused | Unused | Reset | Error Bit 1 | Error Bit 0 | Read only | Online |
|        |        |        | W/O   | R/O         | R/O         | R/O       | R/O    |

Notes
- Online indicates the device is ready to read/write
- Error bits are cleared by setting the reset bit.
- Resetting the device clears errors and sets the current address to zero.

Error Codes

| Error bits (1:0) | Description          |
|------------------|----------------------|
| 00               | No error             |
| 01               | Out-of-bounds access |
| 10               | R/W error            |
| 11               | Not used             |

No error and offline indicates no medium loaded in disk, i.e. no file loaded.
