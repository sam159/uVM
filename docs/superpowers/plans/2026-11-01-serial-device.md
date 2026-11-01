# Serial Device Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a poll-driven serial device at `0x10` giving uVM programs non-blocking access to stdin and stdout.

**Architecture:** A new `MMDevice` implementation (`src/devices/serial.c`) holds a 32-byte ring buffer and the input/output fds. Every `STATUS`/`DATA` read calls `poll(in_fd, POLLIN, 0)` and drains available bytes into the buffer; output bytes are written immediately with `write()`. `main.c` registers it at base `0x0010` with priority `1`, overriding the RAM device exactly as the timer does at `0x20`.

**Tech Stack:** C11, CMake, POSIX `poll`/`read`/`write`, plain C test executables (no framework).

**Spec:** `docs/superpowers/specs/2026-11-01-serial-device-design.md`

## Global Constraints

- Device layout: `STATUS` at offset `0x0`, `DATA` at offset `0x1`; `length = 2`, `priority = 1`.
- Status bits: `RX Ready` `0x01`, `RX Buffer Empty` `0x02` (set when empty), `TX Ready` `0x04` (always set), `TX Buffer Not Empty` `0x08` (always clear).
- Receive buffer capacity is `SERIAL_RX_CAPACITY = 32`.
- Never block: no threads, no `fcntl`, no changing fd flags, no terminal raw mode.
- Addresses passed to device `read`/`write` are relative to `base`.
- Read of an empty buffer or an out-of-range register returns `0x00`; writes to `STATUS` and out-of-range registers are ignored.
- Output write failures are ignored (device interface has no error channel).
- No changes to `vm.h`, `vm.c`, the assembler, `programs/`, or the Arduino port.
- Build: `cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build cmake-build-debug`.
- Tests run from `cmake-build-debug/test_*`; full suite via `bash tests/test_integration.sh`.

## Review Focus

- **Closed stdout:** `write()` raises `SIGPIPE`, terminating the process. This is normal Unix CLI behaviour and is not changed; the spec's "ignore write failures" covers `EPIPE` only when the signal is already ignored.
- **Invalid or closed input fd:** `poll` reports `POLLNVAL` or `read` fails; both must degrade to "no data", not crash or loop. Tested in Task 2.
- **Input burst larger than the buffer:** excess bytes stay in the kernel pipe and must be delivered later in order, not dropped. Tested in Task 2.
- **Ring buffer wrap-around after a partial drain:** byte order must survive wrapping. Tested in Task 2 (40-byte burst drains through a wrap).
- **Non-text bytes:** NUL and high bytes must pass through unchanged in both directions. Output side tested in Task 3.

---

### Task 1: Serial device skeleton and status register

**Files:**
- Create: `src/devices/serial.h`
- Create: `src/devices/serial.c`
- Test: `tests/test_serial_device.c`
- Modify: `CMakeLists.txt` (uVM source list, and a new test target after `test_timer_device`)

**Interfaces:**
- Consumes: `MMDevice` from `src/mm/mm.h`.
- Produces: `SERIAL_RX_CAPACITY`, status-bit macros, `SerialDevice` struct, `MMDevice serial_device_create(uint16_t base, int in_fd, int out_fd)`, `void serial_device_destroy(MMDevice *dev)`.

- [ ] **Step 1: Write the failing test and wire up the test target**

Create `tests/test_serial_device.c`:

```c
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
```

In `CMakeLists.txt`, add `src/devices/serial.c src/devices/serial.h` to the `add_executable(uVM ...)` source list, and add after the `test_timer_device` block:

```cmake
add_executable(test_serial_device tests/test_serial_device.c src/devices/serial.c src/mm/mm.c)
target_include_directories(test_serial_device PRIVATE src/mm src/devices)
add_test(NAME test_serial_device COMMAND test_serial_device)
```

- [ ] **Step 2: Run the build to verify it fails**

Run: `cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build cmake-build-debug`
Expected: configure/build error that `serial.h` or `serial.c` is missing.

- [ ] **Step 3: Create `src/devices/serial.h`**

```c
#ifndef UVM_SERIAL_DEVICE_H
#define UVM_SERIAL_DEVICE_H

#include "mm.h"

#define SERIAL_RX_CAPACITY 32

#define SERIAL_STATUS_RX_READY   0x01
#define SERIAL_STATUS_RX_EMPTY   0x02
#define SERIAL_STATUS_TX_READY   0x04
#define SERIAL_STATUS_TX_NOT_EMPTY 0x08

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
```

- [ ] **Step 4: Create `src/devices/serial.c` with status, empty DATA, and ignored writes**

`serial_status()` returns `SERIAL_STATUS_TX_READY` OR-ed with `SERIAL_STATUS_RX_EMPTY` when `count == 0`, otherwise `SERIAL_STATUS_RX_READY`. `serial_read` returns that for offset `0x0` and `0x00` otherwise. `serial_write` does nothing. `serial_device_create` allocates a zeroed `SerialDevice`, stores the fds, and fills the `MMDevice` (`base`, `length = 2`, `priority = 1`, `ctx`, `read`, `write`), aborting on allocation failure. `serial_device_destroy` frees `ctx` and nulls it.

- [ ] **Step 5: Build and run the test to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_serial_device`
Expected: all `PASS`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add src/devices/serial.h src/devices/serial.c tests/test_serial_device.c CMakeLists.txt
git commit -m "feat: add serial device skeleton and status register"
```

---

### Task 2: Non-blocking receive path

**Files:**
- Modify: `src/devices/serial.c`
- Test: `tests/test_serial_device.c`

**Interfaces:**
- Consumes: `SerialDevice`, `SERIAL_RX_CAPACITY`, `SERIAL_STATUS_*` from Task 1.
- Produces: internal `static void serial_refill(SerialDevice *s)`; `read` on offset `0x1` returns and consumes the oldest buffered byte.

- [ ] **Step 1: Add the failing receive tests**

Add `#include <poll.h>` and these blocks to `tests/test_serial_device.c`, immediately before `serial_device_destroy(&dev);`:

```c
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

    /* EOF leaves the receive side empty */
    close(in_pipe[1]);
    CHECK(dev.read(dev.ctx, 0x0) & SERIAL_STATUS_RX_EMPTY, "EOF leaves RX empty");
```

Then remove the now-duplicate `close(in_pipe[1]);` from the cleanup block at the end (it is closed just above).

- [ ] **Step 2: Build and run to verify it fails**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_serial_device`
Expected: `FAIL: RX ready after input` and related failures; exit code 1.

- [ ] **Step 3: Implement the receive path**

Add `#include <errno.h>`, `#include <poll.h>`, `#include <unistd.h>` to `serial.c`. Implement `serial_refill` as:

```c
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
```

Call `serial_refill(s)` at the top of `serial_read`. On offset `0x1`, if `count == 0` return `0x00`; otherwise return `buffer[head]`, advance `head` modulo `SERIAL_RX_CAPACITY`, and decrement `count`. Reads of other offsets continue to return `0x00`.

- [ ] **Step 4: Build and run to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_serial_device`
Expected: all `PASS`, exit code 0.

- [ ] **Step 5: Commit**

```bash
git add src/devices/serial.c tests/test_serial_device.c
git commit -m "feat: add non-blocking serial receive buffer"
```

---

### Task 3: Transmit path

**Files:**
- Modify: `src/devices/serial.c`
- Test: `tests/test_serial_device.c`

**Interfaces:**
- Consumes: `SerialDevice.out_fd` from Task 1.
- Produces: `write` on offset `0x1` emits exactly one byte to `out_fd`; writes to other offsets emit nothing.

- [ ] **Step 1: Add the failing transmit tests**

Add these blocks to `tests/test_serial_device.c` immediately before `serial_device_destroy(&dev);` (after the receive tests):

```c
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
```

- [ ] **Step 2: Build and run to verify it fails**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_serial_device`
Expected: `FAIL: DATA write reaches output` after the 1-second poll timeout; exit code 1, no hang.

- [ ] **Step 3: Implement the transmit path**

Replace the empty `serial_write` body: return unless `addr == 0x1`; otherwise write the byte with `write(s->out_fd, &data, 1)`, retrying while the result is negative and `errno == EINTR`. Ignore all other write errors.

- [ ] **Step 4: Build and run to verify it passes**

Run: `cmake --build cmake-build-debug && ./cmake-build-debug/test_serial_device`
Expected: all `PASS`, exit code 0.

- [ ] **Step 5: Commit**

```bash
git add src/devices/serial.c tests/test_serial_device.c
git commit -m "feat: add serial transmit path"
```

---

### Task 4: Register the device in the VM and update the docs

**Files:**
- Modify: `src/main.c` (includes, static state, setup, both teardown paths)
- Modify: `docs/Memory-Manager-And-Devices.md`

**Interfaces:**
- Consumes: `serial_device_create(uint16_t base, int in_fd, int out_fd)` and `serial_device_destroy(MMDevice *)` from Task 1.

- [ ] **Step 1: Wire the device into `main.c`**

Add `#include <unistd.h>` and `#include "serial.h"` with the other includes. Add `static MMDevice serial_dev;` beside `mem_dev`/`timer_dev`. After the memory device registration, add:

```c
    serial_dev = serial_device_create(0x0010, STDIN_FILENO, STDOUT_FILENO);
    mm_register(manager, serial_dev);
```

Add `serial_device_destroy(&serial_dev);` before `timer_device_destroy(&timer_dev);` in **both** teardown sites (the compile-mode early return and the end of `main`).

- [ ] **Step 2: Build and smoke-test the wiring**

Run: `cmake --build cmake-build-debug`

Create `/tmp/serial_smoke.asm`:

```asm
:origin 0
        LDI R6, 0
        LDI R7, 0x10
        LDI R1, 0x41
        STA [RXB + 1], R1
        HLT R0, 0
```

Then run:

```bash
./cmake-build-debug/uVM -c /tmp/serial_smoke.asm -o /tmp/serial_smoke.bin
./cmake-build-debug/uVM /tmp/serial_smoke.bin | head -c 1
```

Expected: `A` (the program writes `0x41` to `0x11`, the serial `DATA` register, and the VM then prints its state).

- [ ] **Step 3: Update the source document**

In `docs/Memory-Manager-And-Devices.md`, change the heading `### Serial Device` to `### Serial Device (implemented)` and add this paragraph after the status-flag notes:

```markdown
Reads of `STATUS` and `DATA` poll stdin without blocking and buffer incoming bytes. `RX Ready` is set
while the buffer holds data and clears when it empties, including at end of input.
```

- [ ] **Step 4: Run the full suite**

Run: `bash tests/test_integration.sh`
Expected: every unit test and every example program `PASS`, summary shows `Failed: 0`.

- [ ] **Step 5: Commit**

```bash
git add src/main.c docs/Memory-Manager-And-Devices.md docs/superpowers/specs/2026-11-01-serial-device-design.md docs/superpowers/plans/2026-11-01-serial-device.md
git commit -m "feat: register serial device at 0x10 and document it"
```
