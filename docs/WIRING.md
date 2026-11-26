# Wiring: NUCLEO-F446RE

## SPI pin map used by the drivers/examples

| Signal | SPI1 (AF5) | SPI2 (AF5) |
|---|---|---|
| SCK  | PA5 (Arduino D13, also LED LD2) | PB13 (CN10 pin 30) |
| MISO | PA6 (D12) | PB14 (CN10 pin 28) |
| MOSI | PA7 (D11) | PB15 (CN10 pin 26) |
| NSS/CS | PA4 (A2), software GPIO | PB12 (CN10 pin 16), hardware NSS in |

Debug UART: USART2 PA2/PA3 are already connected to the ST-LINK VCP. No wiring needed.

## examples/spi_loopback

```
PA7 (D11) ──── PA6 (D12)     one jumper
```

After the tests finish, the firmware keeps sending `55 AA 0F F0` every 10 ms
(mode 0, 1 MHz) with CS on PA4. You can check the SCK/MOSI/CS waveforms on a scope or logic analyzer.

## examples/spi_master_slave

```
PA5  ──── PB13   SCK
PA7  ──── PB15   MOSI
PA6  ──── PB14   MISO
PA4  ──── PB12   CS -> NSS
```

Remove the loopback jumper first. Keep the wires short (< 15 cm).
