# Serial Device Design

## Intent

Add the serial/console device described in `docs/Memory-Manager-And-Devices.md` so programs can
read stdin and write stdout through the memory manager. It is registered by default at `0x10`
(overlaying the RAM device at the same addresses), matching the document's default layout.

There is no serial code in `src/` today; `0x10` currently dispatches to the flat RW memory device.

## Behaviour

Two bytes at the device base:

| Offset | Name     | Mode | Behaviour                                        |
|--------|----------|------|--------------------------------------------------|
| 0x0    | `STATUS` | R/O  | Snapshot of the flags below; writes ignored      |
| 0x1    | `DATA`   | R/W  | Read next buffered byte (0x00 if empty); write one byte to stdout |

`STATUS` bits (matching the value-based numbering used by the timer doc and code):

| Value  | Name               | Meaning                                        |
|--------|--------------------|------------------------------------------------|
| `0x01` | RX Ready           | Set iff the receive buffer holds at least one byte |
| `0x02` | RX Buffer Empty    | Complement of RX Ready (set when empty)        |
| `0x04` | TX Ready           | Always set; host writes are synchronous        |
| `0x08` | TX Buffer Not Empty| Always clear                                   |

Unset/unused bits read as zero.

Correction to the source document: the "RX Buffer Empty" note says the bit is *clear* when empty.
That is a typo. This design follows the bit's name: bit 2 is **set** when the buffer is empty, so a
program can poll until it is set.

EOF (for example a closed pipe, or plain end of file) simply yields no new bytes, so RX Ready
clears once the buffer drains and stays clear. No EOF state is latched: if input becomes available
again later, the device picks it up.

The host terminal is left in its normal (cooked) mode. Line-at-a-time input from a terminal is
acceptable; the device does not switch the terminal to raw mode.

## Approach

**Chosen: poll on access, single-threaded.** Every `STATUS` or `DATA` read first calls
`poll(fd, POLLIN, 0)`. If the fd is readable, `read()` appends as much as fits into a small
internal ring buffer before the requested byte/status is produced. Nothing blocks the VM, no
threads or locks are introduced, and no fd flags are modified process-wide.

Rejected alternatives:

- **Background reader thread:** a thread blocking in `read()` fills the buffer, so bytes are
  captured even while the VM is not touching the device. Rejected: pthreads, teardown ordering and
  test complexity for no benefit under the documented poll-loop usage.
- **`fcntl(O_NONBLOCK)` on stdin:** simpler reads, but mutates stdin's flags for the whole process,
  including after the VM exits.

## Interface

```c
/* devices/serial.h */
#define SERIAL_RX_CAPACITY 32

typedef struct SerialDevice_t {
    int      in_fd;
    int      out_fd;
    uint8_t  buffer[SERIAL_RX_CAPACITY];
    uint8_t  head;   /* next byte to consume */
    uint8_t  count;  /* bytes currently buffered */
} SerialDevice;

MMDevice serial_device_create(uint16_t base, int in_fd, int out_fd);
void     serial_device_destroy(MMDevice *dev);
```

- The fd seam (rather than hard-coded stdio) lets tests drive the device with `pipe()`.
- `length = 2`, `priority = 1` (same override mechanism the timer already relies on).
- Write path: `write(out_fd, &byte, 1)`, retrying on `EINTR`. Write failures are ignored; the
  device interface has no error channel.

## Wiring

- `src/devices/serial.c` / `serial.h` implemented against the existing `MMDevice` interface.
- `src/main.c`: add `static MMDevice serial_dev;`, create with
  `(0x0010, STDIN_FILENO, STDOUT_FILENO)`, register with the manager, and destroy during teardown
  alongside the memory and timer devices.
- `CMakeLists.txt`: add `src/devices/serial.c`/`.h` to `uVM` and add a `test_serial_device`
  executable linking `serial.c` and `mm.c`, with `src/mm` and `src/devices` on the include path.

## Testing

New `tests/test_serial_device.c` (plain C executable, matching existing test style), using two
pipes:

- Initial status: RX Ready clear, RX Buffer Empty set, TX Ready set, TX Not Empty clear.
- Data written to the input pipe: RX Ready set, `DATA` returns the byte, then empty again.
- Several bytes arrive together and are returned in FIFO order.
- `DATA` read with an empty buffer returns `0x00`.
- Closing the input pipe's write end clears RX Ready once drained.
- `DATA` write appears on the output pipe.
- `STATUS` writes are ignored.
- A burst larger than `SERIAL_RX_CAPACITY` neither overruns nor loses ordering for the first
  capacity bytes.
- `serial_device_destroy()` releases the context.

## Non-goals

- Raw terminal mode, line editing, echo.
- CRLF translation, flow control, interrupts, or a real TX queue.
- Error reporting for output failures.
- A sample serial program under `programs/` (integration runs programs without stdin and
  discards stdout, so it would add no coverage).
- Changes to `vm.h`, `vm.c`, the assembler, or the Arduino port.
