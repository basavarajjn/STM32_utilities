/**
 * @file  systick.c
 * @brief 1 ms SysTick time base.
 */
#include "systick.h"

static volatile uint32_t s_ticks;

void systick_init(void)
{
    SYSTICK->LOAD = (SystemCoreClock / 1000U) - 1U;
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

void SysTick_Handler(void)
{
    s_ticks++;
}

uint32_t millis(void)
{
    return s_ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = s_ticks;
    while ((s_ticks - start) < ms) { }
}
