# Memory Manager for uVM

Replace the flat memory region currently in use with a memory manager that can map memory reads and writes to devices.
Devices may overlap but can only provide one contiguous region; to accommodate this each will have a priority when added.

Devices include
- Memory R/W and R/O
- Serial device/console providing access to stdin and stdout
- Disk device mapped to a file

This replaces the current static allocated memory region in main.

## Devices

Devices appear to the memory manager only as an address, length, and methods to read and write.
The reading and writing methods will take addresses relative to their start address and not the main memory space.

By default, the following is loaded
- Memory R/W at 0x0 of length 0xFFFF
- Serial device on stdin and stdout at 0x10

### Memory Device

The most basic and essential. This has a length and a flag to allow program writes. For read-only writing is allowed 
at initialisation time to enable roms/static data to be loaded. Writing should be gated in the device not the memory manager.

### Serial Device

This is a simple UART style serial interface with no input/output buffer. Address is offset by its location in the manager.

| Address | Name     | Description                                                |
|---------|----------|------------------------------------------------------------|
| 0x1     | `STATUS` | Status flags                                               |
| 0x2     | `DATA`   | Read or write next byte                                    |
| 0x3     | `POLL_R` | Reads 0 if DATA available to read, 1 if not (use with JNZ) |
| 0x4     | `POLL_W` | Reads 0 if DATA can be written to, 1 if busy               |

#### Status flag bits

Unused bits always read zero. Writes have no effect.

| 8      | 7      | 6      | 5      | 4                  | 3                  | 2        | 1        |
|--------|--------|--------|--------|--------------------|--------------------|----------|----------|
| Unused | Unused | Unused | Unused | POLL Timeout bit 2 | POLL Timeout bit 1 | TX Ready | RX Ready |
|        |        |        |        | R/W                | R/W                | R/O      | R/O      |

Timeout modes, triggered upon reading `POLL_R` or `POLL_W` when no data is ready to read or written.

| Mode (bits 2:1) | Description    |
|-----------------|----------------|
| 00              | No timeout     |
| 01              | Wait 100ms     |
| 10              | Wait 1000ms/1s |
| 11              | Infinite wait  |

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
| 0x1     | `STATUS`       | R/W  | Status flags                                                            |
| 0x2     | `DATA`         | R/W  | Read or writes data at current address                                  |
| 0x3     | `BLOCK_INDEX`  | R/W  | current block selected                                                  |
| 0x4     | `BLOCK_COUNT`  | R/O  | Number of blocks on disk                                                |
| 0x5     | `BLOCK_SIZE_H` | R/O  | size of each block. high byte                                           |
| 0x6     | `BLOCK_SIZE_L` | R/O  | low byte of size                                                        |
| 0x7     | `ADDR_H`       | R/W  | read/write current address within block. Seek done on write of low byte |
| 0x8     | `ADDR_L`       | R/W  | low byte of address                                                     |

#### Status flags

Unused bits always read zero. Writes have no effect.

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
