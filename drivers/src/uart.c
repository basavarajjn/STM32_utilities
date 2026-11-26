/**
 * @file  uart.c
 * @brief Minimal polled USART2 driver + newlib _write() hook for printf().
 */
#include "uart.h"
#include "gpio.h"

void uart2_init(uint32_t baud)
{
    const gpio_config_t tx = { GPIOA, 2, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_NONE, 7 };
    const gpio_config_t rx = { GPIOA, 3, GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_UP,   7 };
    gpio_init(&tx);
    gpio_init(&rx);

    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->APB1ENR;

    /* Oversampling by 16: USARTDIV = PCLK1 / baud, rounded. PCLK1 = SystemCoreClock (APB1 div 1). */
    USART2->CR1 = 0;
    USART2->BRR = (SystemCoreClock + baud / 2U) / baud;
    USART2->CR2 = 0;
    USART2->CR3 = 0;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart2_putc(char c)
{
    while ((USART2->SR & USART_SR_TXE) == 0U) { }
    USART2->DR = (uint8_t)c;
}

void uart2_write(const char *s, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '\n') uart2_putc('\r');
        uart2_putc(s[i]);
    }
    while ((USART2->SR & USART_SR_TC) == 0U) { }
}

void uart2_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') uart2_putc('\r');
        uart2_putc(*s++);
    }
}

/* newlib hook: printf()/puts() end up here */
int _write(int fd, char *ptr, int len)
{
    (void)fd;
    uart2_write(ptr, (size_t)len);
    return len;
}
