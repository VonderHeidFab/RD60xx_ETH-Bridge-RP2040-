// ws2812.h – Steuerung für Status- und RX/TX-LEDs

#pragma once
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include <stdint.h>
#include <stdbool.h>

// Zustände für LED0 (Status)
typedef enum {
    WS2812_STATUS_BOOT,
    WS2812_STATUS_STATIC,
    WS2812_STATUS_DHCP
} ws2812_status_t;

void ws2812_init(uint pin, uint led_count);

void ws2812_set_status(ws2812_status_t st);
void ws2812_pulse_rx(void);
void ws2812_pulse_tx(void);

void ws2812_task(void);
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin, uint freq, bool rgbw);

