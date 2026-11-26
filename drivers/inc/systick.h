/**
 * @file  systick.h
 * @brief 1 ms SysTick time base and blocking delays.
 */
#ifndef SYSTICK_H
#define SYSTICK_H

#include "stm32f446xx.h"

void     systick_init(void);          /* 1 kHz tick from SystemCoreClock */
uint32_t millis(void);
void     delay_ms(uint32_t ms);

#endif /* SYSTICK_H */
