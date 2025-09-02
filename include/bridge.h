/* =================================================
 * bridge.h / bridge.c : TCP <-> UART Bridge Modul
 * ================================================= */

// bridge.h – UART <-> TCP Bridge

#pragma once
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "lwip/tcp.h"

void bridge_start(uint16_t port, uart_inst_t *uart_id);
void bridge_stop(void);
void bridge_task(void);
