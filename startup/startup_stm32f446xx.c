/**
 * @file  startup_stm32f446xx.c
 * @brief Bare-metal startup for STM32F446xx: vector table, Reset_Handler,
 *        .data/.bss init, FPU enable. Written in C, no CMSIS required.
 */
#include <stdint.h>
#include "stm32f446xx.h"

uint32_t SystemCoreClock = HSI_VALUE;   /* 16 MHz HSI after reset */

/* Symbols from the linker script */
extern uint32_t _estack;
extern uint32_t _sidata, _sdata, _edata;
extern uint32_t _sbss, _ebss;

extern int  main(void);
extern void __libc_init_array(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Core exceptions (weak: override by defining a function with the same name) */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* Peripheral IRQs used by this repo (add more names as needed) */
void SPI1_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));
void SPI4_IRQHandler(void)    __attribute__((weak, alias("Default_Handler")));

#define NUM_IRQS 97   /* STM32F446: IRQ0..IRQ96 */

typedef void (*vector_t)(void);

/* Range-initialise all IRQs to Default_Handler, then override the named ones */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

__attribute__((section(".isr_vector"), used))
const vector_t g_vectors[16 + NUM_IRQS] = {
    (vector_t)(uintptr_t)&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    /* External interrupts: everything defaults to Default_Handler */
    [16 ... (16 + NUM_IRQS - 1)] = Default_Handler,
    [16 + SPI1_IRQn]   = SPI1_IRQHandler,
    [16 + SPI2_IRQn]   = SPI2_IRQHandler,
    [16 + USART2_IRQn] = USART2_IRQHandler,
    [16 + SPI3_IRQn]   = SPI3_IRQHandler,
    [16 + SPI4_IRQn]   = SPI4_IRQHandler,
};
#pragma GCC diagnostic pop

void Reset_Handler(void)
{
    /* Copy .data from flash to RAM */
    uint32_t *src = &_sidata;
    for (uint32_t *dst = &_sdata; dst < &_edata; ) *dst++ = *src++;

    /* Zero .bss */
    for (uint32_t *dst = &_sbss; dst < &_ebss; ) *dst++ = 0;

    /* Enable FPU (CP10/CP11 full access) - required for -mfloat-abi=hard */
    SCB_CPACR |= (0xFUL << 20);
    __asm volatile ("dsb\n isb");

    __libc_init_array();
    main();
    for (;;) { }
}

/* Unhandled interrupt / fault: park here so a debugger shows where we are */
void Default_Handler(void)
{
    for (;;) { }
}
