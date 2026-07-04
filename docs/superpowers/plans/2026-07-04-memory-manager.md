# Memory Manager Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the flat memory array with a memory manager that dispatches reads/writes to registered devices based on address ranges and priority.

**Architecture:** The memory manager sits between the VM core and device implementations. Devices are registered with a base address, length, and priority. On each read/write, the manager scans the device list and dispatches to the highest-priority device covering that address. This phase implements only the manager framework and memory device (RAM/ROM).

**Tech Stack:** C11, CMake, plain C test executables (no framework)

## Global Constraints

- Address space is 0x0000 to 0xFFFE (length 0xFFFF, matching `VM_MEM_SIZE = 0xFFFF`)
- Devices receive addresses relative to their base (0 to length-1)
- Unmapped reads return 0x00, unmapped writes are silently ignored
- ROM writes at runtime are silently ignored (not an error)
- Maximum 16 devices registered simultaneously
- No changes to `vm.h` or `vm.c`

---

### Task 1: Define Memory Manager Types

**Files:**
- Create: `mm/mm.h`

**Interfaces:**
- Produces: `MMDevice` struct, `MMManager` struct, `mm_*` function declarations

- [ ] **Step 1: Create mm/mm.h**

```c
#ifndef UVM_MM_H
#define UVM_MM_H

#include <stdint.h>
#include <stdbool.h>

#define MM_MAX_DEVICES 16

typedef struct MMDevice_t {
    uint16_t base;
    uint16_t length;
    uint8_t  priority;
    void    *ctx;
    uint8_t (*read)(void *ctx, uint16_t addr);
    void    (*write)(void *ctx, uint16_t addr, uint8_t data);
} MMDevice;

typedef struct MMManager_t {
    MMDevice devices[MM_MAX_DEVICES];
    uint8_t  count;
} MMManager;

MMManager *mm_create(void);
void       mm_free(MMManager *mm);
bool       mm_register(MMManager *mm, MMDevice device);
uint8_t    mm_read(MMManager *mm, uint16_t addr, bool instruction);
void       mm_write(MMManager *mm, uint16_t addr, uint8_t data);

#endif
```

- [ ] **Step 2: Commit**

```bash
git add mm/mm.h
git commit -m "feat: add memory manager type definitions"
```

---

### Task 2: Implement Memory Device

**Files:**
- Create: `devices/memory.h`
- Create: `devices/memory.c`
- Create: `tests/test_memory_device.c`
- Modify: `CMakeLists.txt:75-77`

**Interfaces:**
- Consumes: `MMDevice` struct from `mm/mm.h`
- Produces: `memory_device_rw()`, `memory_device_rom()`, `memory_device_load()`, `memory_device_destroy()`

- [ ] **Step 1: Create devices/memory.h**

```c
#ifndef UVM_MEMORY_DEVICE_H
#define UVM_MEMORY_DEVICE_H

#include "mm.h"

typedef struct MemoryDevice_t {
    uint8_t *data;
    uint16_t length;
    bool     writable;
} MemoryDevice;

MMDevice memory_device_rw(uint16_t base, uint16_t length);
MMDevice memory_device_rom(uint16_t base, uint16_t length);
void     memory_device_load(MMDevice *dev, uint16_t offset, const uint8_t *data, uint16_t len);
void     memory_device_destroy(MMDevice *dev);

#endif
```

- [ ] **Step 2: Write failing test in tests/test_memory_device.c**

```c
#include <stdio.h>
#include <string.h>
#include "memory.h"

int main(void) {
    MMDevice dev = memory_device_rw(0x0000, 0x1000);
    
    uint8_t test_data[] = {0x12, 0x34, 0x56};
    memory_device_load(&dev, 0x100, test_data, 3);
    
    if (dev.read(dev.ctx, 0x100) != 0x12) {
        printf("FAIL: RW device load+read offset 0x100\n");
        return 1;
    }
    if (dev.read(dev.ctx, 0x101) != 0x34) {
        printf("FAIL: RW device load+read offset 0x101\n");
        return 1;
    }
    
    dev.write(dev.ctx, 0x200, 0xAB);
    if (dev.read(dev.ctx, 0x200) != 0xAB) {
        printf("FAIL: RW device write+read\n");
        return 1;
    }
    
    memory_device_destroy(&dev);
    
    MMDevice rom = memory_device_rom(0x0000, 0x1000);
    uint8_t rom_data[] = {0xDE, 0xAD};
    memory_device_load(&rom, 0x000, rom_data, 2);
    
    if (rom.read(rom.ctx, 0x000) != 0xDE) {
        printf("FAIL: ROM device load+read\n");
        return 1;
    }
    
    rom.write(rom.ctx, 0x000, 0xFF);
    if (rom.read(rom.ctx, 0x000) != 0xDE) {
        printf("FAIL: ROM device write should be ignored\n");
        return 1;
    }
    
    memory_device_destroy(&rom);
    
    printf("PASS: memory device tests\n");
    return 0;
}
```

- [ ] **Step 3: Add test to CMakeLists.txt**

Add after line 77:

```cmake
add_executable(test_memory_device tests/test_memory_device.c devices/memory.c mm/mm.c)
target_include_directories(test_memory_device PRIVATE mm devices)
add_test(NAME test_memory_device COMMAND test_memory_device)
```

- [ ] **Step 4: Run test to verify it fails**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_memory_device`
Expected: Build fails (memory.c not implemented)

- [ ] **Step 5: Implement devices/memory.c**

```c
#include "memory.h"
#include <stdlib.h>
#include <string.h>

static uint8_t memory_read(void *ctx, uint16_t addr) {
    MemoryDevice *mem = (MemoryDevice *)ctx;
    return mem->data[addr];
}

static void memory_write(void *ctx, uint16_t addr, uint8_t data) {
    MemoryDevice *mem = (MemoryDevice *)ctx;
    if (mem->writable) {
        mem->data[addr] = data;
    }
}

static MMDevice create_device(uint16_t base, uint16_t length, bool writable) {
    MemoryDevice *mem = calloc(1, sizeof(MemoryDevice));
    mem->data = calloc(1, length);
    mem->length = length;
    mem->writable = writable;
    
    MMDevice dev = {0};
    dev.base = base;
    dev.length = length;
    dev.priority = 0;
    dev.ctx = mem;
    dev.read = memory_read;
    dev.write = memory_write;
    return dev;
}

MMDevice memory_device_rw(uint16_t base, uint16_t length) {
    return create_device(base, length, true);
}

MMDevice memory_device_rom(uint16_t base, uint16_t length) {
    return create_device(base, length, false);
}

void memory_device_load(MMDevice *dev, uint16_t offset, const uint8_t *data, uint16_t len) {
    MemoryDevice *mem = (MemoryDevice *)dev->ctx;
    memcpy(mem->data + offset, data, len);
}

void memory_device_destroy(MMDevice *dev) {
    MemoryDevice *mem = (MemoryDevice *)dev->ctx;
    free(mem->data);
    free(mem);
    dev->ctx = NULL;
}
```

- [ ] **Step 6: Run test to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_memory_device`
Expected: `PASS: memory device tests`

- [ ] **Step 7: Commit**

```bash
git add devices/memory.h devices/memory.c tests/test_memory_device.c CMakeLists.txt
git commit -m "feat: implement memory device with RW/ROM support"
```

---

### Task 3: Implement Memory Manager

**Files:**
- Create: `mm/mm.c`
- Create: `tests/test_mm.c`
- Modify: `CMakeLists.txt:78-80`

**Interfaces:**
- Consumes: `MMDevice` struct from `mm/mm.h`
- Produces: `mm_create()`, `mm_free()`, `mm_register()`, `mm_read()`, `mm_write()`

- [ ] **Step 1: Write failing test in tests/test_mm.c**

```c
#include <stdio.h>
#include "mm.h"

static uint8_t test_read(void *ctx, uint16_t addr) {
    uint8_t *data = (uint8_t *)ctx;
    return data[addr];
}

static void test_write(void *ctx, uint16_t addr, uint8_t val) {
    uint8_t *data = (uint8_t *)ctx;
    data[addr] = val;
}

int main(void) {
    MMManager *mm = mm_create();
    
    uint8_t buf1[256] = {0};
    MMDevice dev1 = {0};
    dev1.base = 0x0000;
    dev1.length = 0x0100;
    dev1.priority = 0;
    dev1.ctx = buf1;
    dev1.read = test_read;
    dev1.write = test_write;
    
    if (!mm_register(mm, dev1)) {
        printf("FAIL: register device 1\n");
        return 1;
    }
    
    mm_write(mm, 0x0010, 0xAB);
    if (mm_read(mm, 0x0010, false) != 0xAB) {
        printf("FAIL: read/write through manager\n");
        return 1;
    }
    
    if (mm_read(mm, 0x0200, false) != 0x00) {
        printf("FAIL: unmapped read should return 0x00\n");
        return 1;
    }
    
    uint8_t buf2[256] = {0};
    MMDevice dev2 = {0};
    dev2.base = 0x0000;
    dev2.length = 0x0100;
    dev2.priority = 1;
    dev2.ctx = buf2;
    dev2.read = test_read;
    dev2.write = test_write;
    
    if (!mm_register(mm, dev2)) {
        printf("FAIL: register device 2\n");
        return 1;
    }
    
    mm_write(mm, 0x0010, 0xCD);
    if (buf1[0x10] != 0xAB) {
        printf("FAIL: lower priority device should not be written\n");
        return 1;
    }
    if (buf2[0x10] != 0xCD) {
        printf("FAIL: higher priority device should be written\n");
        return 1;
    }
    
    mm_free(mm);
    
    printf("PASS: memory manager tests\n");
    return 0;
}
```

- [ ] **Step 2: Add test to CMakeLists.txt**

Add after line 80:

```cmake
add_executable(test_mm tests/test_mm.c mm/mm.c)
target_include_directories(test_mm PRIVATE mm)
add_test(NAME test_mm COMMAND test_mm)
```

- [ ] **Step 3: Run test to verify it fails**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_mm`
Expected: Build fails (mm.c not implemented)

- [ ] **Step 4: Implement mm/mm.c**

```c
#include "mm.h"
#include <stdlib.h>

MMManager *mm_create(void) {
    return calloc(1, sizeof(MMManager));
}

void mm_free(MMManager *mm) {
    free(mm);
}

bool mm_register(MMManager *mm, MMDevice device) {
    if (mm->count >= MM_MAX_DEVICES) {
        return false;
    }
    mm->devices[mm->count] = device;
    mm->count++;
    return true;
}

uint8_t mm_read(MMManager *mm, uint16_t addr, bool instruction) {
    MMDevice *best = NULL;
    for (uint8_t i = 0; i < mm->count; i++) {
        MMDevice *d = &mm->devices[i];
        if (addr >= d->base && addr < d->base + d->length) {
            if (!best || d->priority > best->priority) {
                best = d;
            }
        }
    }
    if (!best) return 0x00;
    return best->read(best->ctx, addr - best->base);
}

void mm_write(MMManager *mm, uint16_t addr, uint8_t data) {
    MMDevice *best = NULL;
    for (uint8_t i = 0; i < mm->count; i++) {
        MMDevice *d = &mm->devices[i];
        if (addr >= d->base && addr < d->base + d->length) {
            if (!best || d->priority > best->priority) {
                best = d;
            }
        }
    }
    if (!best) return;
    best->write(best->ctx, addr - best->base, data);
}
```

- [ ] **Step 5: Run test to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_mm`
Expected: `PASS: memory manager tests`

- [ ] **Step 6: Commit**

```bash
git add mm/mm.c tests/test_mm.c CMakeLists.txt
git commit -m "feat: implement memory manager with priority-based device dispatch"
```

---

### Task 4: Integrate Memory Manager into main.c

**Files:**
- Modify: `main.c:1-223`
- Modify: `CMakeLists.txt:6-7`

**Interfaces:**
- Consumes: `mm_create()`, `mm_free()`, `mm_register()`, `mm_read()`, `mm_write()`, `memory_device_rw()`, `memory_device_load()`, `memory_device_destroy()`
- Produces: Updated main executable using memory manager

- [ ] **Step 1: Update CMakeLists.txt to include new sources**

Change line 6 from:
```cmake
add_executable(uVM main.c vm.c vm.h utils.c utils.h asm/asm.c asm/asm.h asm/token.c asm/token.h asm/tree.c asm/tree.h)
target_include_directories(uVM PRIVATE asm)
```

To:
```cmake
add_executable(uVM main.c vm.c vm.h utils.c utils.h asm/asm.c asm/asm.h asm/token.c asm/token.h asm/tree.c asm/tree.h mm/mm.c mm/mm.h devices/memory.c devices/memory.h)
target_include_directories(uVM PRIVATE asm mm devices)
```

- [ ] **Step 2: Update main.c includes and globals**

Replace lines 1-15 with:

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>

#include "vm.h"
#include "asm.h"
#include "mm.h"
#include "memory.h"

static MMManager *manager;
static MMDevice mem_dev;

static uint16_t load_addrs[32];
static long load_sizes[32];
static int load_count = 0;
```

- [ ] **Step 3: Update host_read and host_write**

Replace lines 87-93 with:

```c
static uint8_t host_read(uint16_t addr, bool instruction) {
    return mm_read(manager, addr, instruction);
}

static void host_write(uint16_t addr, uint8_t data) {
    mm_write(manager, addr, data);
}
```

- [ ] **Step 4: Update load_binary to use memory_device_load**

Replace line 49 (`size_t read = fread(memory + addr, 1, size, f);`) with:

```c
    uint8_t *buf = malloc(size);
    size_t read = fread(buf, 1, size, f);
    fclose(f);
    
    if ((long)read != size) {
        fprintf(stderr, "Error: failed to read '%s'\n", path);
        free(buf);
        return -1;
    }
    
    memory_device_load(&mem_dev, addr, buf, (uint16_t)size);
    free(buf);
```

Remove the duplicate `fclose(f);` on line 50.

- [ ] **Step 5: Update main() to initialize memory manager**

After line 134 (`VM *vm = new_vm();`), add:

```c
    manager = mm_create();
    mem_dev = memory_device_rw(0x0000, VM_MEM_SIZE);
    mm_register(manager, mem_dev);
```

- [ ] **Step 6: Update main() cleanup**

Before line 221 (`free_vm(vm);`), add:

```c
    memory_device_destroy(&mem_dev);
    mm_free(manager);
```

- [ ] **Step 7: Build and test**

Run: `cmake --build cmake-build-debug`
Expected: Build succeeds

Run: `./cmake-build-debug/test_hlt_q`
Expected: `PASS: HLT Q format tests`

Run: `./cmake-build-debug/test_mm`
Expected: `PASS: memory manager tests`

Run: `./cmake-build-debug/test_memory_device`
Expected: `PASS: memory device tests`

- [ ] **Step 8: Run integration test**

Run: `bash tests/test_integration.sh`
Expected: All tests pass

- [ ] **Step 9: Commit**

```bash
git add main.c CMakeLists.txt
git commit -m "feat: integrate memory manager into main executable"
```

---

### Task 5: Final Verification

**Files:**
- None (verification only)

- [ ] **Step 1: Clean build**

Run: `rm -rf cmake-build-debug && cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build cmake-build-debug`
Expected: Clean build with no warnings

- [ ] **Step 2: Run all unit tests**

Run: `cd cmake-build-debug && ctest --output-on-failure`
Expected: All tests pass (test_hlt_q, test_mm, test_memory_device, plus all assembler tests)

- [ ] **Step 3: Run integration test**

Run: `bash tests/test_integration.sh`
Expected: All integration tests pass

- [ ] **Step 4: Verify memory allocation is correct**

Check that `devices/memory.c` allocates exactly `VM_MEM_SIZE` (0xFFFF) bytes, not 0x10000.

Run: `grep -n "calloc" devices/memory.c`
Expected: `calloc(1, length)` where length is passed as parameter

- [ ] **Step 5: Final commit if any fixes were needed**

```bash
git add -A
git commit -m "fix: address any issues found during verification"
```
