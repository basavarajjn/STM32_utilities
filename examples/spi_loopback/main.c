/**
 * @file  main.c
 * @brief SPI1 loopback self-test for NUCLEO-F446RE (bare metal).
 *
 * Wiring: put ONE jumper wire between PA7 (MOSI, Arduino D11) and PA6 (MISO, D12).
 *         PA5 = SCK (D13, also drives the green LED LD2, so it flickers - normal).
 *         PA4 = software CS (A2) - scope it if you like, no connection needed.
 *
 * Output: USART2 @ 115200 8N1 on the ST-LINK virtual COM port.
 *
 * Tests: all 4 SPI modes, MSB/LSB first, 8-bit and 16-bit frames,
 *        several baud dividers, and the interrupt-driven API.
 */
#include <stdio.h>
#include <string.h>
#include "stm32f446xx.h"
#include "gpio.h"
#include "spi.h"
#include "uart.h"
#include "systick.h"

#define CS_PORT GPIOA
#define CS_PIN  4

static spi_handle_t g_spi1;
static volatile int g_it_done;
static volatile spi_status_t g_it_status;

void SPI1_IRQHandler(void) { spi_irq_handler(&g_spi1); }

static void on_it_done(spi_handle_t *h, spi_status_t st)
{
    (void)h;
    g_it_status = st;
    g_it_done = 1;
}

static void spi1_pins_init(void)
{
    /* SCK PA5, MISO PA6, MOSI PA7 -> AF5 */
    const uint8_t pins[] = { 5, 6, 7 };
    for (unsigned i = 0; i < sizeof pins; i++) {
        gpio_config_t c = { GPIOA, pins[i], GPIO_MODE_AF, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_NONE, 5 };
        gpio_init(&c);
    }
    /* CS PA4 as push-pull output, idle high */
    gpio_config_t cs = { CS_PORT, CS_PIN, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_SPEED_HIGH, GPIO_PULL_NONE, 0 };
    gpio_init(&cs);
    spi_cs_high(CS_PORT, CS_PIN);
}

static const char *st_str(spi_status_t s)
{
    switch (s) {
    case SPI_OK:          return "OK";
    case SPI_ERR_TIMEOUT: return "TIMEOUT";
    case SPI_ERR_MODF:    return "MODF";
    case SPI_ERR_OVR:     return "OVR";
    case SPI_ERR_BUSY:    return "BUSY";
    default:              return "PARAM";
    }
}

static int run_case(spi_mode_t mode, spi_baud_t baud, spi_datasize_t ds, spi_bitorder_t bo, int use_it)
{
    spi_config_t cfg = {
        .instance = SPI1, .role = SPI_ROLE_MASTER, .mode = mode, .baud = baud,
        .datasize = ds, .bitorder = bo, .nss = SPI_NSS_SOFT,
    };
    spi_init(&g_spi1, &cfg);

    uint16_t tx[32], rx[32];
    size_t n = 32;
    for (size_t i = 0; i < n; i++) tx[i] = (uint16_t)(0xA500u + i * 37u);
    memset(rx, 0, sizeof rx);

    /* 8-bit mode uses the buffers as bytes */
    uint8_t *txp = (uint8_t *)tx, *rxp = (uint8_t *)rx;

    spi_status_t st;
    spi_cs_low(CS_PORT, CS_PIN);
    if (use_it) {
        g_it_done = 0;
        st = spi_transfer_it(&g_spi1, txp, rxp, n, on_it_done);
        if (st == SPI_OK) {
            uint32_t t0 = millis();
            while (!g_it_done && (millis() - t0) < 100U) { }
            st = g_it_done ? g_it_status : SPI_ERR_TIMEOUT;
            if (st == SPI_OK) st = spi_wait_idle(&g_spi1);
        }
    } else {
        st = spi_transfer(&g_spi1, txp, rxp, n);
    }
    spi_cs_high(CS_PORT, CS_PIN);

    size_t bytes = (ds == SPI_DATA_16BIT) ? n * 2 : n;
    int ok = (st == SPI_OK) && memcmp(txp, rxp, bytes) == 0;

    printf("  mode%d div%-3u %2s-bit %s %-4s : %s",
           (int)mode, 2u << baud, ds == SPI_DATA_16BIT ? "16" : "8",
           bo == SPI_LSB_FIRST ? "LSB" : "MSB", use_it ? "IT" : "POLL",
           ok ? "PASS" : "FAIL");
    if (!ok) {
        printf(" (status=%s, tx[0]=0x%02X rx[0]=0x%02X)", st_str(st), txp[0], rxp[0]);
    }
    printf("\n");

    spi_deinit(&g_spi1);
    return ok;
}

int main(void)
{
    systick_init();
    uart2_init(115200);
    spi1_pins_init();

    printf("\n=== STM32F446RE bare-metal SPI1 loopback test ===\n");
    printf("SYSCLK = %lu Hz. Jumper PA7 (D11) <-> PA6 (D12) required.\n\n", (unsigned long)SystemCoreClock);

    int pass = 0, total = 0;
    const spi_baud_t bauds[] = { SPI_BAUD_DIV2, SPI_BAUD_DIV16, SPI_BAUD_DIV256 };

    for (int it = 0; it <= 1; it++)
        for (int m = 0; m < 4; m++)
            for (unsigned b = 0; b < sizeof bauds / sizeof bauds[0]; b++)
                for (int ds = 0; ds <= 1; ds++)
                    for (int bo = 0; bo <= 1; bo++) {
                        pass += run_case((spi_mode_t)m, bauds[b], (spi_datasize_t)ds, (spi_bitorder_t)bo, it);
                        total++;
                    }

    printf("\nRESULT: %d/%d passed -> %s\n", pass, total, pass == total ? "ALL PASS" : "FAILURES");
    if (pass == 0) printf("Hint: every case failed - check the PA7<->PA6 jumper.\n");

    /* Keep clocking a known pattern so you can probe SCK/MOSI/CS with a scope / logic analyzer */
    spi_config_t cfg = { SPI1, SPI_ROLE_MASTER, SPI_MODE_0, SPI_BAUD_DIV16, SPI_DATA_8BIT, SPI_MSB_FIRST, SPI_NSS_SOFT };
    spi_init(&g_spi1, &cfg);
    const uint8_t pattern[] = { 0x55, 0xAA, 0x0F, 0xF0 };
    for (;;) {
        spi_cs_low(CS_PORT, CS_PIN);
        spi_write(&g_spi1, pattern, sizeof pattern);
        spi_cs_high(CS_PORT, CS_PIN);
        delay_ms(10);
    }
}
