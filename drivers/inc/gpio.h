/**
 * @file  gpio.h
 * @brief Bare-metal GPIO driver for STM32F446xx.
 */
#ifndef GPIO_H
#define GPIO_H

#include "stm32f446xx.h"

typedef enum { GPIO_MODE_INPUT = 0, GPIO_MODE_OUTPUT = 1, GPIO_MODE_AF = 2, GPIO_MODE_ANALOG = 3 } gpio_mode_t;
typedef enum { GPIO_OTYPE_PP = 0, GPIO_OTYPE_OD = 1 } gpio_otype_t;
typedef enum { GPIO_SPEED_LOW = 0, GPIO_SPEED_MEDIUM = 1, GPIO_SPEED_FAST = 2, GPIO_SPEED_HIGH = 3 } gpio_speed_t;
typedef enum { GPIO_PULL_NONE = 0, GPIO_PULL_UP = 1, GPIO_PULL_DOWN = 2 } gpio_pull_t;

typedef struct {
    GPIO_TypeDef *port;
    uint8_t       pin;        /* 0..15 */
    gpio_mode_t   mode;
    gpio_otype_t  otype;
    gpio_speed_t  speed;
    gpio_pull_t   pull;
    uint8_t       af;         /* 0..15, only used when mode == GPIO_MODE_AF */
} gpio_config_t;

void gpio_clock_enable(GPIO_TypeDef *port);
void gpio_init(const gpio_config_t *cfg);

static inline void gpio_set(GPIO_TypeDef *port, uint8_t pin)    { port->BSRR = (1UL << pin); }
static inline void gpio_reset(GPIO_TypeDef *port, uint8_t pin)  { port->BSRR = (1UL << (pin + 16U)); }
static inline void gpio_toggle(GPIO_TypeDef *port, uint8_t pin) { port->ODR ^= (1UL << pin); }
static inline uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin){ return (uint8_t)((port->IDR >> pin) & 1U); }

#endif /* GPIO_H */
