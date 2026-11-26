/**
 * @file  spi.c
 * @brief Bare-metal, register-level SPI driver for STM32F446xx.
 *
 * Reference: RM0390 section 26 (SPI/I2S). Transmit/receive procedure follows
 * 26.3.9 "Data transmission and reception procedures" for full-duplex mode:
 * write DR when TXE=1, read DR when RXNE=1, and wait TXE=1 && BSY=0 before
 * disabling the peripheral or releasing chip select.
 */
#include "spi.h"

/* ------------------------------------------------------------ helpers */

static void spi_clock_enable(SPI_TypeDef *spi, bool en)
{
    if (spi == SPI1)      { if (en) RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; else RCC->APB2ENR &= ~RCC_APB2ENR_SPI1EN; }
    else if (spi == SPI2) { if (en) RCC->APB1ENR |= RCC_APB1ENR_SPI2EN; else RCC->APB1ENR &= ~RCC_APB1ENR_SPI2EN; }
    else if (spi == SPI3) { if (en) RCC->APB1ENR |= RCC_APB1ENR_SPI3EN; else RCC->APB1ENR &= ~RCC_APB1ENR_SPI3EN; }
    else if (spi == SPI4) { if (en) RCC->APB2ENR |= RCC_APB2ENR_SPI4EN; else RCC->APB2ENR &= ~RCC_APB2ENR_SPI4EN; }
    (void)RCC->APB2ENR;
}

static void spi_reset(SPI_TypeDef *spi)
{
    if (spi == SPI1)      { RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST; RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST; }
    else if (spi == SPI2) { RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST; RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI2RST; }
    else if (spi == SPI3) { RCC->APB1RSTR |= RCC_APB1RSTR_SPI3RST; RCC->APB1RSTR &= ~RCC_APB1RSTR_SPI3RST; }
    else if (spi == SPI4) { RCC->APB2RSTR |= RCC_APB2RSTR_SPI4RST; RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI4RST; }
}

static int spi_irqn(SPI_TypeDef *spi)
{
    if (spi == SPI1) return SPI1_IRQn;
    if (spi == SPI2) return SPI2_IRQn;
    if (spi == SPI3) return SPI3_IRQn;
    if (spi == SPI4) return SPI4_IRQn;
    return -1;
}

static bool spi_valid(SPI_TypeDef *spi)
{
    return spi == SPI1 || spi == SPI2 || spi == SPI3 || spi == SPI4;
}

/* Checks error flags and clears them using the sequences from RM0390 26.4. */
static spi_status_t spi_check_errors(SPI_TypeDef *spi)
{
    uint32_t sr = spi->SR;
    if (sr & SPI_SR_OVR) {
        (void)spi->DR;           /* OVR cleared by read DR then read SR */
        (void)spi->SR;
        return SPI_ERR_OVR;
    }
    if (sr & SPI_SR_MODF) {
        uint32_t cr1 = spi->CR1; /* MODF cleared by read SR then write CR1 */
        spi->CR1 = cr1;          /* SPE/MSTR were cleared by HW; restore */
        return SPI_ERR_MODF;
    }
    return SPI_OK;
}

static spi_status_t spi_wait_flag(SPI_TypeDef *spi, uint32_t flag)
{
    uint32_t n = SPI_TIMEOUT_LOOPS;
    while ((spi->SR & flag) == 0U) {
        spi_status_t e = spi_check_errors(spi);
        if (e != SPI_OK) return e;
        if (--n == 0U)   return SPI_ERR_TIMEOUT;
    }
    return SPI_OK;
}

static inline void spi_put_frame(spi_handle_t *h, const uint8_t *tx, size_t i)
{
    SPI_TypeDef *spi = h->cfg.instance;
    if (h->cfg.datasize == SPI_DATA_16BIT) {
        spi->DR = tx ? ((const uint16_t *)tx)[i] : 0xFFFFU;
    } else {
        *(__IO uint8_t *)&spi->DR = tx ? tx[i] : 0xFFU;  /* 8-bit access to DR */
    }
}

static inline void spi_get_frame(spi_handle_t *h, uint8_t *rx, size_t i)
{
    SPI_TypeDef *spi = h->cfg.instance;
    if (h->cfg.datasize == SPI_DATA_16BIT) {
        uint16_t v = (uint16_t)spi->DR;
        if (rx) ((uint16_t *)rx)[i] = v;
    } else {
        uint8_t v = *(__IO uint8_t *)&spi->DR;
        if (rx) rx[i] = v;
    }
}

/* --------------------------------------------------------------- API */

spi_status_t spi_init(spi_handle_t *h, const spi_config_t *cfg)
{
    if (h == NULL || cfg == NULL || !spi_valid(cfg->instance)) return SPI_ERR_PARAM;

    h->cfg = *cfg;
    h->busy = false;
    h->tx_buf = NULL;
    h->rx_buf = NULL;
    h->len = h->tx_count = h->rx_count = 0;
    h->on_complete = NULL;

    SPI_TypeDef *spi = cfg->instance;
    spi_clock_enable(spi, true);
    spi_reset(spi);

    uint32_t cr1 = 0;
    uint32_t cr2 = 0;

    if (cfg->mode == SPI_MODE_1 || cfg->mode == SPI_MODE_3) cr1 |= SPI_CR1_CPHA;
    if (cfg->mode == SPI_MODE_2 || cfg->mode == SPI_MODE_3) cr1 |= SPI_CR1_CPOL;
    cr1 |= ((uint32_t)cfg->baud << SPI_CR1_BR_Pos) & SPI_CR1_BR_Msk;
    if (cfg->datasize == SPI_DATA_16BIT) cr1 |= SPI_CR1_DFF;
    if (cfg->bitorder == SPI_LSB_FIRST)  cr1 |= SPI_CR1_LSBFIRST;

    if (cfg->role == SPI_ROLE_MASTER) {
        cr1 |= SPI_CR1_MSTR;
        if (cfg->nss == SPI_NSS_SOFT) {
            cr1 |= SPI_CR1_SSM | SPI_CR1_SSI;   /* internal NSS held high -> no MODF */
        } else if (cfg->nss == SPI_NSS_HARD_OUT) {
            cr2 |= SPI_CR2_SSOE;                /* NSS pin driven low while SPE=1 */
        }
    } else {
        if (cfg->nss == SPI_NSS_SOFT) {
            cr1 |= SPI_CR1_SSM;                 /* SSI=0 -> slave always selected */
        }
    }

    spi->CR1 = cr1;   /* configure with SPE=0 */
    spi->CR2 = cr2;
    spi->I2SCFGR = 0; /* make sure SPI (not I2S) mode is selected */
    spi->CR1 = cr1 | SPI_CR1_SPE;

    return SPI_OK;
}

void spi_deinit(spi_handle_t *h)
{
    SPI_TypeDef *spi = h->cfg.instance;
    int irqn = spi_irqn(spi);
    if (irqn >= 0) nvic_disable_irq((uint32_t)irqn);
    (void)spi_wait_idle(h);
    spi->CR1 &= ~SPI_CR1_SPE;
    spi_clock_enable(spi, false);
    h->busy = false;
}

spi_status_t spi_wait_idle(spi_handle_t *h)
{
    SPI_TypeDef *spi = h->cfg.instance;
    spi_status_t s = spi_wait_flag(spi, SPI_SR_TXE);
    if (s != SPI_OK) return s;
    uint32_t n = SPI_TIMEOUT_LOOPS;
    while (spi->SR & SPI_SR_BSY) {
        if (--n == 0U) return SPI_ERR_TIMEOUT;
    }
    return SPI_OK;
}

spi_status_t spi_transfer(spi_handle_t *h, const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (h == NULL || len == 0U) return SPI_ERR_PARAM;
    if (h->busy) return SPI_ERR_BUSY;

    SPI_TypeDef *spi = h->cfg.instance;

    /* Flush any stale RX data so the first read below belongs to this transfer */
    while (spi->SR & SPI_SR_RXNE) { (void)spi->DR; }
    (void)spi_check_errors(spi);

    for (size_t i = 0; i < len; i++) {
        spi_status_t s = spi_wait_flag(spi, SPI_SR_TXE);
        if (s != SPI_OK) return s;
        spi_put_frame(h, tx, i);

        s = spi_wait_flag(spi, SPI_SR_RXNE);
        if (s != SPI_OK) return s;
        spi_get_frame(h, rx, i);
    }
    return spi_wait_idle(h);
}

spi_status_t spi_write(spi_handle_t *h, const uint8_t *tx, size_t len)
{
    return spi_transfer(h, tx, NULL, len);
}

spi_status_t spi_read(spi_handle_t *h, uint8_t *rx, size_t len)
{
    return spi_transfer(h, NULL, rx, len);
}

spi_status_t spi_transfer_byte(spi_handle_t *h, uint8_t tx, uint8_t *rx)
{
    return spi_transfer(h, &tx, rx, 1);
}

/* ------------------------------------------------- interrupt-driven mode
 * One frame in flight at a time: write a frame, wait for RXNE in the ISR,
 * read it, write the next. This can never overrun, regardless of SCK speed
 * or interrupt latency, at the cost of a small gap between frames.
 */
spi_status_t spi_transfer_it(spi_handle_t *h, const uint8_t *tx, uint8_t *rx, size_t len, spi_callback_t cb)
{
    if (h == NULL || len == 0U) return SPI_ERR_PARAM;
    if (h->busy) return SPI_ERR_BUSY;

    SPI_TypeDef *spi = h->cfg.instance;
    int irqn = spi_irqn(spi);
    if (irqn < 0) return SPI_ERR_PARAM;

    while (spi->SR & SPI_SR_RXNE) { (void)spi->DR; }
    (void)spi_check_errors(spi);

    h->tx_buf = tx;
    h->rx_buf = rx;
    h->len = len;
    h->tx_count = 0;
    h->rx_count = 0;
    h->on_complete = cb;
    h->busy = true;

    nvic_enable_irq((uint32_t)irqn);
    spi->CR2 |= SPI_CR2_RXNEIE | SPI_CR2_ERRIE;

    /* Kick off the first frame; TXE is set when the peripheral is idle */
    spi_status_t s = spi_wait_flag(spi, SPI_SR_TXE);
    if (s != SPI_OK) {
        spi->CR2 &= ~(SPI_CR2_RXNEIE | SPI_CR2_ERRIE);
        h->busy = false;
        return s;
    }
    spi_put_frame(h, h->tx_buf, h->tx_count++);
    return SPI_OK;
}

static void spi_it_finish(spi_handle_t *h, spi_status_t status)
{
    h->cfg.instance->CR2 &= ~(SPI_CR2_RXNEIE | SPI_CR2_ERRIE | SPI_CR2_TXEIE);
    h->busy = false;
    if (h->on_complete) h->on_complete(h, status);
}

void spi_irq_handler(spi_handle_t *h)
{
    SPI_TypeDef *spi = h->cfg.instance;
    uint32_t sr = spi->SR;

    if (sr & (SPI_SR_OVR | SPI_SR_MODF)) {
        spi_it_finish(h, spi_check_errors(spi));
        return;
    }

    if ((sr & SPI_SR_RXNE) && h->busy) {
        spi_get_frame(h, h->rx_buf, h->rx_count++);
        if (h->tx_count < h->len) {
            spi_put_frame(h, h->tx_buf, h->tx_count++);
        } else {
            spi_it_finish(h, SPI_OK);
        }
    }
}
