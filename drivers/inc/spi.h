/**
 * @file  spi.h
 * @brief Bare-metal, register-level SPI driver for STM32F446xx (SPI1..SPI4).
 *
 * Features:
 *  - Master or slave, all 4 SPI modes (CPOL/CPHA), MSB/LSB first, 8/16-bit frames
 *  - Software (GPIO) or hardware NSS
 *  - Blocking full-duplex transfer with timeout and error reporting
 *  - Interrupt-driven (non-blocking) full-duplex transfer with completion callback
 *
 * Polling functions use a simple loop-count timeout so the driver has no
 * dependency on SysTick. Tune SPI_TIMEOUT_LOOPS if you use very slow clocks.
 */
#ifndef SPI_H
#define SPI_H

#include <stddef.h>
#include <stdbool.h>
#include "stm32f446xx.h"
#include "gpio.h"

#ifndef SPI_TIMEOUT_LOOPS
#define SPI_TIMEOUT_LOOPS  100000UL
#endif

typedef enum {
    SPI_OK = 0,
    SPI_ERR_TIMEOUT,
    SPI_ERR_MODF,      /* mode fault (NSS pulled low while master) */
    SPI_ERR_OVR,       /* overrun: RX data not read in time */
    SPI_ERR_BUSY,      /* an IT transfer is already running */
    SPI_ERR_PARAM,
} spi_status_t;

typedef enum { SPI_ROLE_SLAVE = 0, SPI_ROLE_MASTER = 1 } spi_role_t;

/* Mode 0: CPOL=0 CPHA=0, Mode 1: CPOL=0 CPHA=1, Mode 2: CPOL=1 CPHA=0, Mode 3: CPOL=1 CPHA=1 */
typedef enum { SPI_MODE_0 = 0, SPI_MODE_1, SPI_MODE_2, SPI_MODE_3 } spi_mode_t;

/* SCK = f_PCLK / divider. SPI1/SPI4 on APB2, SPI2/SPI3 on APB1. */
typedef enum {
    SPI_BAUD_DIV2 = 0, SPI_BAUD_DIV4, SPI_BAUD_DIV8, SPI_BAUD_DIV16,
    SPI_BAUD_DIV32, SPI_BAUD_DIV64, SPI_BAUD_DIV128, SPI_BAUD_DIV256
} spi_baud_t;

typedef enum { SPI_DATA_8BIT = 0, SPI_DATA_16BIT = 1 } spi_datasize_t;
typedef enum { SPI_MSB_FIRST = 0, SPI_LSB_FIRST = 1 } spi_bitorder_t;

typedef enum {
    SPI_NSS_SOFT = 0,   /* SSM=1: NSS managed by software, CS via any GPIO (recommended for master) */
    SPI_NSS_HARD_OUT,   /* master drives NSS pin automatically (SSOE=1) */
    SPI_NSS_HARD_IN,    /* slave uses NSS input pin */
} spi_nss_t;

typedef struct spi_handle spi_handle_t;
typedef void (*spi_callback_t)(spi_handle_t *h, spi_status_t status);

typedef struct {
    SPI_TypeDef    *instance;   /* SPI1, SPI2, SPI3 or SPI4 */
    spi_role_t      role;
    spi_mode_t      mode;
    spi_baud_t      baud;
    spi_datasize_t  datasize;
    spi_bitorder_t  bitorder;
    spi_nss_t       nss;
} spi_config_t;

struct spi_handle {
    spi_config_t      cfg;
    /* Interrupt-driven transfer state (internal) */
    const uint8_t    *tx_buf;
    uint8_t          *rx_buf;
    size_t            len;       /* in frames */
    volatile size_t   tx_count;
    volatile size_t   rx_count;
    volatile bool     busy;
    spi_callback_t    on_complete;
};

/* Core API ---------------------------------------------------------------- */
spi_status_t spi_init(spi_handle_t *h, const spi_config_t *cfg);
void         spi_deinit(spi_handle_t *h);

/** Full-duplex transfer of len frames. tx or rx may be NULL (send 0xFF / discard).
 *  For 16-bit frames, buffers are uint16_t arrays cast to uint8_t* and len is in frames. */
spi_status_t spi_transfer(spi_handle_t *h, const uint8_t *tx, uint8_t *rx, size_t len);
spi_status_t spi_write(spi_handle_t *h, const uint8_t *tx, size_t len);
spi_status_t spi_read(spi_handle_t *h, uint8_t *rx, size_t len);
spi_status_t spi_transfer_byte(spi_handle_t *h, uint8_t tx, uint8_t *rx);

/** Non-blocking transfer. Call spi_irq_handler(h) from the SPIx_IRQHandler. */
spi_status_t spi_transfer_it(spi_handle_t *h, const uint8_t *tx, uint8_t *rx, size_t len, spi_callback_t cb);
void         spi_irq_handler(spi_handle_t *h);
static inline bool spi_is_busy(const spi_handle_t *h) { return h->busy; }

/** Wait until the bus is idle (TXE=1, BSY=0). Call before raising CS. */
spi_status_t spi_wait_idle(spi_handle_t *h);

/* Software chip-select helpers -------------------------------------------- */
static inline void spi_cs_low(GPIO_TypeDef *port, uint8_t pin)  { gpio_reset(port, pin); }
static inline void spi_cs_high(GPIO_TypeDef *port, uint8_t pin) { gpio_set(port, pin); }

#endif /* SPI_H */
