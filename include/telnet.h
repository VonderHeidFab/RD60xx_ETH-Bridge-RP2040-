// telnet.h – optionaler Telnet Server

#pragma once
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "lwip/tcp.h"

void telnet_start(uint16_t port, uart_inst_t *uart_id);
void telnet_stop(void);
void telnet_task(void);
