# Hardware test workflow

Claude cannot run code on the board, so every change is verified by you on the
real hardware. To keep that loop fast:

1. **Build** (CI checks this on every push): `make EXAMPLE=<name>`
2. **Flash** with CubeIDE or `make flash`
3. **Run**: open the serial terminal (115200 8N1) and press RESET
4. **Report**: copy the full serial output into the PR, or open a
   *Hardware test report* issue. Include:
   - commit hash / branch
   - board (NUCLEO-F446RE rev) and wiring used
   - the full log, including any `FAIL (status=..., tx[0]=.., rx[0]=..)` lines
   - logic analyzer screenshots if you have them (SCK, MOSI, MISO, CS)

## Reading failures

| Symptom | Likely cause |
|---|---|
| All cases FAIL, rx = 0x00 or 0xFF | Loopback/jumper missing or on wrong pins |
| Only DIV2 cases fail | Signal integrity: long wires or a breadboard |
| `status=TIMEOUT` | Clock to SPI not enabled, or a wrong pin AF |
| `status=MODF` | NSS pulled low while master with hardware NSS |
| `status=OVR` | RX data not read in time (IT latency) |
| No serial output at all | Wrong COM port/baud, or the code faulted before `uart2_init` (check in the debugger) |
