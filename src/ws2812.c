// ws2812.c – WS2812B LED Steuerung
#include "ws2812.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico/time.h"
#include <string.h>
#include <stdlib.h>

#include "ws2812.pio.h"   // PIO-Programm für WS2812

static PIO ws2812_pio;
static int ws2812_sm;
static uint ws2812_pin;
static uint ws2812_count;

static uint32_t *leds = NULL; // Dynamisch

// Puls-Verwaltung
static absolute_time_t rx_timeout, tx_timeout;
static bool rx_active = false, tx_active = false;

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(ws2812_pio, ws2812_sm, pixel_grb << 8u);
}

static uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
}

static void ws2812_show() {
    for (uint i = 0; i < ws2812_count; i++) {
        put_pixel(leds[i]);
    }
}

void ws2812_init(uint pin, uint led_count) {
    ws2812_pio = pio0;
    ws2812_sm = 0;
    ws2812_pin = pin;
    ws2812_count = led_count;

    if (leds) free(leds);
    leds = calloc(ws2812_count, sizeof(uint32_t));

    uint offset = pio_add_program(ws2812_pio, &ws2812_program);
    ws2812_program_init(ws2812_pio, ws2812_sm, offset, ws2812_pin, 800000, true);

    ws2812_show();
}

void ws2812_set_status(ws2812_status_t st) {
    if (ws2812_count < 1) return;
    switch (st) {
        case WS2812_STATUS_BOOT:   leds[0] = urgb_u32(50, 50, 50); break;
        case WS2812_STATUS_STATIC: leds[0] = urgb_u32(0, 100, 0);  break;
        case WS2812_STATUS_DHCP:   leds[0] = urgb_u32(0, 0, 100);  break;
    }
    ws2812_show();
}

void ws2812_pulse_rx(void) {
    if (ws2812_count < 2) return;
    leds[1] = urgb_u32(100, 0, 0);
    ws2812_show();
    rx_active = true;
    rx_timeout = make_timeout_time_ms(50);
}

void ws2812_pulse_tx(void) {
    if (ws2812_count < 2) return;
    leds[1] = urgb_u32(0, 0, 100);
    ws2812_show();
    tx_active = true;
    tx_timeout = make_timeout_time_ms(50);
}

void ws2812_task(void) {
    bool update = false;
    if (rx_active && time_reached(rx_timeout)) {
        rx_active = false;
        leds[1] = 0;
        update = true;
    }
    if (tx_active && time_reached(tx_timeout)) {
        tx_active = false;
        leds[1] = 0;
        update = true;
    }
    if (update) ws2812_show();
}
