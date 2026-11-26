/**
 * @file  gpio.c
 * @brief Bare-metal GPIO driver for STM32F446xx.
 */
#include "gpio.h"

void gpio_clock_enable(GPIO_TypeDef *port)
{
    /* GPIO ports are spaced 0x400 apart starting at GPIOA; enable bit = index */
    uint32_t idx = ((uint32_t)port - GPIOA_BASE) / 0x400UL;
    if (idx <= 7U) {
        RCC->AHB1ENR |= (1UL << idx);
        (void)RCC->AHB1ENR; /* read-back: ensures clock is up before first access */
    }
}

void gpio_init(const gpio_config_t *cfg)
{
    GPIO_TypeDef *p = cfg->port;
    uint32_t pin = cfg->pin;

    gpio_clock_enable(p);

    p->MODER   = (p->MODER   & ~(3UL << (pin * 2U))) | ((uint32_t)cfg->mode  << (pin * 2U));
    p->OTYPER  = (p->OTYPER  & ~(1UL << pin))        | ((uint32_t)cfg->otype << pin);
    p->OSPEEDR = (p->OSPEEDR & ~(3UL << (pin * 2U))) | ((uint32_t)cfg->speed << (pin * 2U));
    p->PUPDR   = (p->PUPDR   & ~(3UL << (pin * 2U))) | ((uint32_t)cfg->pull  << (pin * 2U));

    if (cfg->mode == GPIO_MODE_AF) {
        uint32_t reg = pin >> 3;          /* 0 = AFRL, 1 = AFRH */
        uint32_t pos = (pin & 7U) * 4U;
        p->AFR[reg] = (p->AFR[reg] & ~(0xFUL << pos)) | ((uint32_t)(cfg->af & 0xFU) << pos);
    }
}
