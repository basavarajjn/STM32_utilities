/**
 * @file  main.c
 * @brief SPI1 (master) <-> SPI2 (slave) on ONE NUCLEO-F446RE board.
 *
 * Wiring (4 jumper wires):
 *     SPI1 master            SPI2 slave
 *     PA5  SCK  (D13)  --->  PB13 SCK  (CN10-30)
 *     PA7  MOSI (D11)  --->  PB15 MOSI (CN10-26)
 *     PA6  MISO (D12)  <---  PB14 MISO (CN10-28)
 *     PA4  CS   (A2)   --->  PB12 NSS  (CN10-16)
 *
 * The slave uses the interrupt-driven API with hardware NSS; it pre-loads a
 * reply buffer, then the master clocks a transfer and both sides check the
 * data they received. Output on USART2 @ 115200 (ST-LINK VCP).
 */
#include <stdio.h>
#include <string.h>
#include "stm32f446xx.h"
#include "gpio.h"
#include "spi.h"
#include "uart.h"
#include "systick.h"

#define LEN 16

static spi_handle_t g_master, g_slave;
static volatile int g_slave_done;
static volatile spi_status_t g_slave_status;

void SPI2_IRQHandler(void) { spi_irq_handler(&g_slave); }

static void slave_done(spi_handle_t *h, spi_status_t st) { (void)h; g_slave_status = st; g_slave_done = 1; }

static void pins_init(void)
{
    const uint8_t m_pins[] = { 5, 6, 7 };           /* SPI1 AF5 */
    for (unsigned i = 0; i < 3; i++) {
        gpio_config_t c = { GPIOA, m_pins[i], GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_NONE, 5 };
        gpio_init(&c);
    }
    gpio_config_t cs = { GPIOA, 4, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_NONE, 0 };
    gpio_init(&cs);
    gpio_set(GPIOA, 4);

    const uint8_t s_pins[] = { 12, 13, 14, 15 };     /* SPI2 NSS/SCK/MISO/MOSI AF5 */
    for (unsigned i = 0; i < 4; i++) {
        gpio_config_t c = { GPIOB, s_pins[i], GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH,
                            s_pins[i] == 12 ? GPIO_PULL_UP : GPIO_PULL_NONE, 5 };
        gpio_init(&c);
    }
}

static int run(spi_mode_t mode, spi_baud_t baud)
{
    /* Slave on APB1 (SPI2) and master on APB2 (SPI1) both run at 16 MHz here.
     * A slave needs SCK <= f_PCLK/4 in full duplex, and the IT slave needs
     * time per frame, so keep the master at DIV16 or slower. */
    spi_config_t mc = { SPI1, SPI_ROLE_MASTER, mode, baud, SPI_DATA_8BIT, SPI_MSB_FIRST, SPI_NSS_SOFT };
    spi_config_t sc = { SPI2, SPI_ROLE_SLAVE,  mode, SPI_BAUD_DIV2, SPI_DATA_8BIT, SPI_MSB_FIRST, SPI_NSS_HARD_IN };
    spi_init(&g_slave, &sc);
    spi_init(&g_master, &mc);

    uint8_t m_tx[LEN], m_rx[LEN], s_tx[LEN], s_rx[LEN];
    for (int i = 0; i < LEN; i++) { m_tx[i] = (uint8_t)(0x10 + i); s_tx[i] = (uint8_t)(0xE0 - i); }
    memset(m_rx, 0, LEN);
    memset(s_rx, 0, LEN);

    g_slave_done = 0;
    spi_transfer_it(&g_slave, s_tx, s_rx, LEN, slave_done);

    /* Master sends byte-by-byte with a short gap so the slave ISR can refill DR */
    gpio_reset(GPIOA, 4);
    spi_status_t mst = SPI_OK;
    for (int i = 0; i < LEN && mst == SPI_OK; i++) {
        mst = spi_transfer_byte(&g_master, m_tx[i], &m_rx[i]);
        for (volatile int d = 0; d < 50; d++) { }
    }
    gpio_set(GPIOA, 4);

    uint32_t t0 = millis();
    while (!g_slave_done && millis() - t0 < 50U) { }

    int ok = mst == SPI_OK && g_slave_done && g_slave_status == SPI_OK &&
             memcmp(m_tx, s_rx, LEN) == 0 && memcmp(s_tx, m_rx, LEN) == 0;
    printf("  mode%d div%-3u : %s", (int)mode, 2u << baud, ok ? "PASS" : "FAIL");
    if (!ok) printf("  (m->s first 0x%02X/0x%02X, s->m first 0x%02X/0x%02X, slave_done=%d)",
                    m_tx[0], s_rx[0], s_tx[0], m_rx[0], g_slave_done);
    printf("\n");

    spi_deinit(&g_master);
    spi_deinit(&g_slave);
    return ok;
}

int main(void)
{
    systick_init();
    uart2_init(115200);
    pins_init();

    printf("\n=== STM32F446RE SPI1 master <-> SPI2 slave test ===\n");
    int pass = 0, total = 0;
    const spi_baud_t bauds[] = { SPI_BAUD_DIV16, SPI_BAUD_DIV64, SPI_BAUD_DIV256 };
    for (int m = 0; m < 4; m++)
        for (unsigned b = 0; b < 3; b++) { pass += run((spi_mode_t)m, bauds[b]); total++; }

    printf("\nRESULT: %d/%d passed -> %s\n", pass, total, pass == total ? "ALL PASS" : "FAILURES");
    for (;;) { }
}
