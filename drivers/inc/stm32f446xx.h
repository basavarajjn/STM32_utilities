/**
 * @file    stm32f446xx.h
 * @brief   Minimal bare-metal register map for STM32F446xx (no CMSIS/HAL).
 *
 * Addresses and bit positions taken from RM0390 (STM32F446xx reference manual).
 * Only the peripherals used by this repo are described; extend as needed.
 */
#ifndef STM32F446XX_H
#define STM32F446XX_H

#include <stdint.h>

#define __IO volatile

/* ---------------------------------------------------------------- Memory map */
#define FLASH_BASE          0x08000000UL
#define SRAM_BASE           0x20000000UL
#define PERIPH_BASE         0x40000000UL
#define APB1PERIPH_BASE     PERIPH_BASE
#define APB2PERIPH_BASE     (PERIPH_BASE + 0x00010000UL)
#define AHB1PERIPH_BASE     (PERIPH_BASE + 0x00020000UL)

/* AHB1 */
#define GPIOA_BASE          (AHB1PERIPH_BASE + 0x0000UL)
#define GPIOB_BASE          (AHB1PERIPH_BASE + 0x0400UL)
#define GPIOC_BASE          (AHB1PERIPH_BASE + 0x0800UL)
#define GPIOD_BASE          (AHB1PERIPH_BASE + 0x0C00UL)
#define GPIOE_BASE          (AHB1PERIPH_BASE + 0x1000UL)
#define GPIOF_BASE          (AHB1PERIPH_BASE + 0x1400UL)
#define GPIOG_BASE          (AHB1PERIPH_BASE + 0x1800UL)
#define GPIOH_BASE          (AHB1PERIPH_BASE + 0x1C00UL)
#define RCC_BASE            (AHB1PERIPH_BASE + 0x3800UL)

/* APB1 */
#define SPI2_BASE           (APB1PERIPH_BASE + 0x3800UL)
#define SPI3_BASE           (APB1PERIPH_BASE + 0x3C00UL)
#define USART2_BASE         (APB1PERIPH_BASE + 0x4400UL)

/* APB2 */
#define SPI1_BASE           (APB2PERIPH_BASE + 0x3000UL)
#define SPI4_BASE           (APB2PERIPH_BASE + 0x3400UL)

/* Cortex-M4 core */
#define SYSTICK_BASE        0xE000E010UL
#define NVIC_ISER_BASE      0xE000E100UL
#define NVIC_ICER_BASE      0xE000E180UL
#define SCB_CPACR_ADDR      0xE000ED88UL

/* ---------------------------------------------------------- Register blocks */
typedef struct {
    __IO uint32_t MODER;    /* 0x00 */
    __IO uint32_t OTYPER;   /* 0x04 */
    __IO uint32_t OSPEEDR;  /* 0x08 */
    __IO uint32_t PUPDR;    /* 0x0C */
    __IO uint32_t IDR;      /* 0x10 */
    __IO uint32_t ODR;      /* 0x14 */
    __IO uint32_t BSRR;     /* 0x18 */
    __IO uint32_t LCKR;     /* 0x1C */
    __IO uint32_t AFR[2];   /* 0x20 AFRL, 0x24 AFRH */
} GPIO_TypeDef;

typedef struct {
    __IO uint32_t CR;          /* 0x00 */
    __IO uint32_t PLLCFGR;     /* 0x04 */
    __IO uint32_t CFGR;        /* 0x08 */
    __IO uint32_t CIR;         /* 0x0C */
    __IO uint32_t AHB1RSTR;    /* 0x10 */
    __IO uint32_t AHB2RSTR;    /* 0x14 */
    __IO uint32_t AHB3RSTR;    /* 0x18 */
    uint32_t      RESERVED0;   /* 0x1C */
    __IO uint32_t APB1RSTR;    /* 0x20 */
    __IO uint32_t APB2RSTR;    /* 0x24 */
    uint32_t      RESERVED1[2];/* 0x28-0x2C */
    __IO uint32_t AHB1ENR;     /* 0x30 */
    __IO uint32_t AHB2ENR;     /* 0x34 */
    __IO uint32_t AHB3ENR;     /* 0x38 */
    uint32_t      RESERVED2;   /* 0x3C */
    __IO uint32_t APB1ENR;     /* 0x40 */
    __IO uint32_t APB2ENR;     /* 0x44 */
} RCC_TypeDef;

typedef struct {
    __IO uint32_t CR1;      /* 0x00 */
    __IO uint32_t CR2;      /* 0x04 */
    __IO uint32_t SR;       /* 0x08 */
    __IO uint32_t DR;       /* 0x0C */
    __IO uint32_t CRCPR;    /* 0x10 */
    __IO uint32_t RXCRCR;   /* 0x14 */
    __IO uint32_t TXCRCR;   /* 0x18 */
    __IO uint32_t I2SCFGR;  /* 0x1C */
    __IO uint32_t I2SPR;    /* 0x20 */
} SPI_TypeDef;

typedef struct {
    __IO uint32_t SR;       /* 0x00 */
    __IO uint32_t DR;       /* 0x04 */
    __IO uint32_t BRR;      /* 0x08 */
    __IO uint32_t CR1;      /* 0x0C */
    __IO uint32_t CR2;      /* 0x10 */
    __IO uint32_t CR3;      /* 0x14 */
    __IO uint32_t GTPR;     /* 0x18 */
} USART_TypeDef;

typedef struct {
    __IO uint32_t CTRL;
    __IO uint32_t LOAD;
    __IO uint32_t VAL;
    __IO uint32_t CALIB;
} SysTick_TypeDef;

/* ------------------------------------------------------ Peripheral pointers */
#define GPIOA   ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB   ((GPIO_TypeDef *)GPIOB_BASE)
#define GPIOC   ((GPIO_TypeDef *)GPIOC_BASE)
#define GPIOD   ((GPIO_TypeDef *)GPIOD_BASE)
#define GPIOE   ((GPIO_TypeDef *)GPIOE_BASE)
#define GPIOF   ((GPIO_TypeDef *)GPIOF_BASE)
#define GPIOG   ((GPIO_TypeDef *)GPIOG_BASE)
#define GPIOH   ((GPIO_TypeDef *)GPIOH_BASE)
#define RCC     ((RCC_TypeDef *)RCC_BASE)
#define SPI1    ((SPI_TypeDef *)SPI1_BASE)
#define SPI2    ((SPI_TypeDef *)SPI2_BASE)
#define SPI3    ((SPI_TypeDef *)SPI3_BASE)
#define SPI4    ((SPI_TypeDef *)SPI4_BASE)
#define USART2  ((USART_TypeDef *)USART2_BASE)
#define SYSTICK ((SysTick_TypeDef *)SYSTICK_BASE)
#define NVIC_ISER ((__IO uint32_t *)NVIC_ISER_BASE)
#define NVIC_ICER ((__IO uint32_t *)NVIC_ICER_BASE)
#define SCB_CPACR (*(__IO uint32_t *)SCB_CPACR_ADDR)

/* ----------------------------------------------------------- RCC bit fields */
#define RCC_AHB1ENR_GPIOAEN   (1UL << 0)
#define RCC_AHB1ENR_GPIOBEN   (1UL << 1)
#define RCC_AHB1ENR_GPIOCEN   (1UL << 2)
#define RCC_AHB1ENR_GPIODEN   (1UL << 3)
#define RCC_AHB1ENR_GPIOEEN   (1UL << 4)
#define RCC_AHB1ENR_GPIOFEN   (1UL << 5)
#define RCC_AHB1ENR_GPIOGEN   (1UL << 6)
#define RCC_AHB1ENR_GPIOHEN   (1UL << 7)

#define RCC_APB1ENR_SPI2EN    (1UL << 14)
#define RCC_APB1ENR_SPI3EN    (1UL << 15)
#define RCC_APB1ENR_USART2EN  (1UL << 17)

#define RCC_APB2ENR_SPI1EN    (1UL << 12)
#define RCC_APB2ENR_SPI4EN    (1UL << 13)

#define RCC_APB1RSTR_SPI2RST  (1UL << 14)
#define RCC_APB1RSTR_SPI3RST  (1UL << 15)
#define RCC_APB2RSTR_SPI1RST  (1UL << 12)
#define RCC_APB2RSTR_SPI4RST  (1UL << 13)

/* ----------------------------------------------------------- SPI bit fields */
#define SPI_CR1_CPHA      (1UL << 0)
#define SPI_CR1_CPOL      (1UL << 1)
#define SPI_CR1_MSTR      (1UL << 2)
#define SPI_CR1_BR_Pos    3U
#define SPI_CR1_BR_Msk    (7UL << SPI_CR1_BR_Pos)
#define SPI_CR1_SPE       (1UL << 6)
#define SPI_CR1_LSBFIRST  (1UL << 7)
#define SPI_CR1_SSI       (1UL << 8)
#define SPI_CR1_SSM       (1UL << 9)
#define SPI_CR1_RXONLY    (1UL << 10)
#define SPI_CR1_DFF       (1UL << 11)
#define SPI_CR1_CRCNEXT   (1UL << 12)
#define SPI_CR1_CRCEN     (1UL << 13)
#define SPI_CR1_BIDIOE    (1UL << 14)
#define SPI_CR1_BIDIMODE  (1UL << 15)

#define SPI_CR2_RXDMAEN   (1UL << 0)
#define SPI_CR2_TXDMAEN   (1UL << 1)
#define SPI_CR2_SSOE      (1UL << 2)
#define SPI_CR2_FRF       (1UL << 4)
#define SPI_CR2_ERRIE     (1UL << 5)
#define SPI_CR2_RXNEIE    (1UL << 6)
#define SPI_CR2_TXEIE     (1UL << 7)

#define SPI_SR_RXNE       (1UL << 0)
#define SPI_SR_TXE        (1UL << 1)
#define SPI_SR_UDR        (1UL << 3)
#define SPI_SR_CRCERR     (1UL << 4)
#define SPI_SR_MODF       (1UL << 5)
#define SPI_SR_OVR        (1UL << 6)
#define SPI_SR_BSY        (1UL << 7)
#define SPI_SR_FRE        (1UL << 8)

/* --------------------------------------------------------- USART bit fields */
#define USART_SR_RXNE     (1UL << 5)
#define USART_SR_TC       (1UL << 6)
#define USART_SR_TXE      (1UL << 7)
#define USART_CR1_RE      (1UL << 2)
#define USART_CR1_TE      (1UL << 3)
#define USART_CR1_UE      (1UL << 13)

/* ------------------------------------------------------- SysTick bit fields */
#define SYSTICK_CTRL_ENABLE    (1UL << 0)
#define SYSTICK_CTRL_TICKINT   (1UL << 1)
#define SYSTICK_CTRL_CLKSOURCE (1UL << 2)
#define SYSTICK_CTRL_COUNTFLAG (1UL << 16)

/* ------------------------------------------------------------ IRQ numbers */
#define SPI1_IRQn    35
#define SPI2_IRQn    36
#define USART2_IRQn  38
#define SPI3_IRQn    51
#define SPI4_IRQn    84

/* Default core clock after reset: 16 MHz HSI, all bus prescalers = 1 */
#define HSI_VALUE          16000000UL
extern uint32_t SystemCoreClock;

static inline void nvic_enable_irq(uint32_t irqn)  { NVIC_ISER[irqn >> 5] = 1UL << (irqn & 31U); }
static inline void nvic_disable_irq(uint32_t irqn) { NVIC_ICER[irqn >> 5] = 1UL << (irqn & 31U); }

#endif /* STM32F446XX_H */
