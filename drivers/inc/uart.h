/**
 * @file  uart.h
 * @brief Minimal polled USART2 driver for debug output via the Nucleo ST-LINK
 *        virtual COM port (PA2 = TX, PA3 = RX, AF7). printf() is routed here.
 */
#ifndef UART_H
#define UART_H

#include <stddef.h>
#include "stm32f446xx.h"

void uart2_init(uint32_t baud);
void uart2_putc(char c);
void uart2_write(const char *s, size_t len);
void uart2_puts(const char *s);

#endif /* UART_H */
