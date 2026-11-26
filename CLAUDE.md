# Notes for Claude

This repo holds bare-metal firmware for the **STM32F446RE (NUCLEO-F446RE)**. The
owner tests every change on real hardware and reports back with the serial log.

## Rules

- **No HAL, no LL, no CMSIS.** Use only `drivers/inc/stm32f446xx.h`. Add any
  registers or bits you need there, with the RM0390 section in a comment.
- Check every register address and bit position against **RM0390** (reference
  manual) and **DS10693** (datasheet, AF table). Cite the section in comments.
- Every new feature needs a self-checking example in `examples/<name>/main.c`
  that prints `PASS`/`FAIL` per case and a final `RESULT: x/y passed` line over USART2.
- Polling loops need timeouts. Don't let a missing wire hang the board silently.
- `make EXAMPLE=<name>` must build with **zero warnings** (`-Wall -Wextra`).
- Document any new wiring in `docs/WIRING.md`.
- Clocks: 16 MHz HSI by default (`SystemCoreClock`). If you change clocks,
  update `SystemCoreClock`, the UART BRR, and the SPI baud notes.

## When a hardware test fails

Read the log the owner pasted, find which cases failed and their status codes,
compare them with `docs/TESTING.md`, and fix the root cause. Don't change the
test to make it pass.
