# STM32_utilities

Bare-metal (register-level, **no HAL, no CMSIS**) peripheral drivers for the
**STM32F446RE / NUCLEO-F446RE**, starting with a full SPI driver.

Firmware is written by Claude and validated on real hardware. Every example
prints a PASS/FAIL report over the ST-LINK virtual COM port, so test results
can be pasted straight back into an issue.

## What's inside

```
drivers/
  inc/stm32f446xx.h   Hand-written register map (RCC, GPIO, SPI, USART, SysTick, NVIC)
  inc/spi.h  src/spi.c      SPI1-4 driver: master/slave, modes 0-3, 8/16-bit, MSB/LSB,
                            soft/hard NSS, blocking + interrupt-driven transfers
  inc/gpio.h src/gpio.c     GPIO init / set / reset / toggle / read
  inc/uart.h src/uart.c     USART2 debug console, printf() support
  inc/systick.h src/systick.c  1 ms tick, millis(), delay_ms()
startup/              Vector table + Reset_Handler in C, newlib syscalls
linker/               STM32F446RETX_FLASH.ld (512K flash / 128K RAM)
examples/
  spi_loopback/       SPI1 self-test, 1 jumper wire (96 test cases)
  spi_master_slave/   SPI1 master <-> SPI2 slave on one board, 4 jumper wires
docs/                 CubeIDE setup, wiring, hardware test workflow
```

Clock: default 16 MHz HSI (no PLL yet), so APB1 = APB2 = 16 MHz and the
fastest SPI clock is 8 MHz (DIV2).

## Build

Requires `arm-none-eabi-gcc` and `make` (both ship with STM32CubeIDE).

```bash
make                              # examples/spi_loopback
make EXAMPLE=spi_master_slave
make flash                        # optional, needs st-flash
```

Output: `build/<example>/<example>.elf / .bin / .hex`

**STM32CubeIDE:** see [docs/CUBEIDE.md](docs/CUBEIDE.md) to import this as a
Makefile project and flash/debug it with the on-board ST-LINK.

## Test on hardware

| Example | Wiring | Expected |
|---|---|---|
| `spi_loopback` | PA7 (D11) ↔ PA6 (D12) | `RESULT: 96/96 passed -> ALL PASS` |
| `spi_master_slave` | PA5↔PB13, PA7↔PB15, PA6↔PB14, PA4↔PB12 | `RESULT: 12/12 passed -> ALL PASS` |

Open a serial terminal on the ST-LINK COM port at **115200 8N1**, press the
black RESET button, and read the report. Details: [docs/WIRING.md](docs/WIRING.md).

## Claude ↔ hardware workflow

1. Claude writes or changes firmware on a branch and opens a PR (CI must build).
2. You flash it, run the example, and paste the serial output into the PR or a
   **Hardware test report** issue.
3. Claude fixes failures from the log; repeat until ALL PASS, then merge.

See [docs/TESTING.md](docs/TESTING.md) and [CLAUDE.md](CLAUDE.md).

## SPI driver quick start

```c
#include "spi.h"

spi_handle_t spi;
spi_config_t cfg = {
    .instance = SPI1, .role = SPI_ROLE_MASTER, .mode = SPI_MODE_0,
    .baud = SPI_BAUD_DIV16,            /* 16 MHz / 16 = 1 MHz */
    .datasize = SPI_DATA_8BIT, .bitorder = SPI_MSB_FIRST, .nss = SPI_NSS_SOFT,
};
/* configure SCK/MISO/MOSI as AF5 and a CS GPIO first (see examples) */
spi_init(&spi, &cfg);

uint8_t tx[3] = { 0x9F, 0, 0 }, rx[3];
spi_cs_low(GPIOA, 4);
spi_status_t st = spi_transfer(&spi, tx, rx, sizeof tx);
spi_cs_high(GPIOA, 4);
```

## Roadmap

- [ ] PLL clock setup to 180 MHz
- [ ] SPI DMA transfers
- [ ] Device drivers on top of SPI (W25Qxx flash, MAX31855, SD card)
- [ ] I2C and UART (IRQ/DMA) drivers

## License

MIT
