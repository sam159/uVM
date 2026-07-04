# Memory Manager Design

## Overview

Replace the flat `memory[]` array in `main.c` with a memory manager that dispatches reads and writes to registered devices based on address ranges and priority. This is the first phase, implementing only the manager framework and the memory device. Serial and disk devices will follow in later phases.

## Architecture

```
  VM core (vm.c)
    | readAddr(addr, instruction)
    | writeAddr(addr, data)
    v
  Memory Manager (mm/mm.c)
    | scans device list, picks highest-priority match
    v
  Devices
    +-- Memory Device (devices/memory.c) -- RAM/ROM
    +-- Serial Device (future)
    +-- Disk Device   (future)
```

The VM struct (`vm.h`) is unchanged. The memory manager provides functions matching the existing `readAddr`/`writeAddr` signatures, wired up via thin trampolines in `main.c`.

## File Layout

```
mm/mm.h          -- MMDevice struct, MMManager struct, mm_* API
mm/mm.c          -- manager implementation (create, register, read, write, free)
devices/memory.h -- memory device factory function
devices/memory.c -- memory device implementation
```

## Device Interface

```c
typedef struct MMDevice_t {
    uint16_t base;
    uint16_t length;
    uint8_t  priority;
    void    *ctx;
    uint8_t (*read)(void *ctx, uint16_t addr);
    void    (*write)(void *ctx, uint16_t addr, uint8_t data);
} MMDevice;
```

- `read`/`write` receive addresses relative to `base` (0 to length-1).
- `ctx` holds device-specific state.
- No `free` callback on the device. Device-specific cleanup is handled by a destroy function per device type, called before `mm_free()`.

## Memory Manager API

```c
#define MM_MAX_DEVICES 16

typedef struct MMManager_t {
    MMDevice devices[MM_MAX_DEVICES];
    uint8_t  count;
} MMManager;

MMManager *mm_create(void);
void       mm_free(MMManager *mm);
bool       mm_register(MMManager *mm, MMDevice device);
uint8_t    mm_read(MMManager *mm, uint16_t addr, bool instruction);
void       mm_write(MMManager *mm, uint16_t addr, uint8_t data);
```

### Address Resolution

`mm_read` and `mm_write` perform a linear scan of the device list. For each device, check if `addr` falls within `[base, base + length)`. Among all matching devices, select the one with the highest `priority`. Call its `read`/`write` with `addr - base`.

- If no device matches: reads return `0x00`, writes are silently ignored.
- `mm_register` copies the device struct into the manager's array. Returns `false` if the array is full.

### VM Integration

`main.c` creates the manager, registers devices, then wires up trampolines:

```c
static MMManager *manager;

static uint8_t host_read(uint16_t addr, bool instruction) {
    return mm_read(manager, addr, instruction);
}

static void host_write(uint16_t addr, uint8_t data) {
    mm_write(manager, addr, data);
}
```

The `instruction` parameter is passed through but unused by the memory device. It is reserved for future use (e.g., execute-protection).

## Memory Device

```c
typedef struct MemoryDevice_t {
    uint8_t *data;
    uint16_t length;
    bool     writable;
} MemoryDevice;

MMDevice memory_device_rw(uint16_t base, uint16_t length);
MMDevice memory_device_rom(uint16_t base, uint16_t length);
void     memory_device_load(MMDevice *dev, uint16_t offset, const uint8_t *data, uint16_t len);
void     memory_device_destroy(MMDevice *dev);
```

### Behavior

- **Read**: returns `data[addr]`.
- **Write (RW)**: sets `data[addr] = value`.
- **Write (ROM)**: silently ignored.
- **`memory_device_load()`**: writes directly into `data` at `offset`, bypassing the writable flag. Used for loading binaries at init time.
- **`memory_device_destroy()`**: frees the backing buffer. Must be called before `mm_free()`.

### Factory Functions

- `memory_device_rw(base, length)`: allocates a zeroed buffer, sets `writable = true`.
- `memory_device_rom(base, length)`: allocates a zeroed buffer, sets `writable = false`.

### Default Setup

`main.c` creates one RW memory device at base `0x0000`, length `0xFFFF` (matching `VM_MEM_SIZE`). The static `memory[]` array is removed. Note: the current `memory[VM_MEM_SIZE + 1]` allocation is incorrect (0x10000 bytes exceeds the 16-bit addressable range); the memory device allocates exactly `0xFFFF` bytes.

Binary loading via `load_binary()` calls `memory_device_load()` on the memory device instead of writing directly to the flat array. `main.c` retains a pointer to the memory device's `MMDevice` struct for this purpose.

## Testing

Following existing patterns (plain C executables, no framework):

### `tests/test_mm.c`

- Register a device, read/write through the manager, verify correct dispatch.
- Register overlapping devices with different priorities, verify higher priority wins.
- Read from unmapped address returns `0x00`.
- Write to unmapped address is silently ignored.
- Register 17th device returns `false`.

### `tests/test_memory_device.c`

- Create RW device, write and read back.
- Create ROM device, `load()` data, verify reads return correct values.
- ROM device: runtime writes via `write()` are silently ignored (data unchanged).
- `load()` at various offsets and boundary conditions.

Both registered in `CMakeLists.txt` as test executables linking against `mm/mm.c` and `devices/memory.c`.

Existing `test_hlt_q` continues to work unchanged (provides its own memory hooks). Integration test (`test_integration.sh`) exercises the full pipeline through `main.c`.

## Scope Exclusions

- Serial device (stdin/stdout) -- future phase.
- Disk device (file-backed) -- future phase.
- Execute-protection or instruction-fetch routing -- reserved but not implemented.
- Changes to `vm.h` or `vm.c` -- none required.
