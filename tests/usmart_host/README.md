# USMART host-side parser test

Compiles the real `project/src/usmart.c` and `project/src/usmart_str.c` against a
stub port layer, so the command parser can be exercised on a PC without the
AT32 board.

## Run

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tests\usmart_host\run_test.ps1
```

Requires a native `gcc` (MinGW) on `PATH`. The test executable is written to
`build\host_test\`.

`inc\usmart_port.h` is a shim that mirrors the port macros from
`project/inc/usmart_port.h`; it is force-included with `-include` so that the
real AT32 port header is skipped on the host.

## Coverage

- function table lookup (`usmart_nametab`: `func` first, signature second)
- decimal / hex / negative parameter conversion, whitespace and parentheses
- parameter-count errors, unknown function
- `read_addr` / `write_addr` / `at32_led_on` / `at32_led_off` / `at32_led_toggle`
- system commands: `?`, `help`, `list`, `id`, `hex`, `dec`, `runtime`

Expected result: `25 checks, 0 failures` and exit code 0.

## Not covered

The USART1 RX DMA + IDLE interrupt path and the TMR3 scan interrupt are
hardware-specific; verify them on the target with `help`, `list`,
`at32_led_toggle(1)` and `read_addr(0x20000000)`.
