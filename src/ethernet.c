// ethernet.c – CH9120 Steuerung
#include "ethernet.h"
#include "hardware/uart.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdio.h>

// Annahme: CH9120 ist am zweiten UART (uart1)
#define ETH_UART_ID uart1
#define ETH_TX 4
#define ETH_RX 5
#define ETH_BAUD 9600

static void ch9120_send_cmd(const uint8_t *cmd, size_t len) {
    uart_write_blocking(ETH_UART_ID, cmd, len);
    sleep_ms(50);
}

static void ch9120_set_ip(const char *ip, const char *mask, const char *gw) {
    // CH9120 erwartet Binärdaten (IP als 4 Bytes)
    uint8_t buf[20];
    uint8_t ipb[4], maskb[4], gwb[4];
    sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &ipb[0], &ipb[1], &ipb[2], &ipb[3]);
    sscanf(mask, "%hhu.%hhu.%hhu.%hhu", &maskb[0], &maskb[1], &maskb[2], &maskb[3]);
    sscanf(gw, "%hhu.%hhu.%hhu.%hhu", &gwb[0], &gwb[1], &gwb[2], &gwb[3]);

    // Beispielbefehl: Setze lokale IP
    buf[0] = 0xC0; buf[1] = 0x01; // Set IP
    memcpy(&buf[2], ipb, 4);
    ch9120_send_cmd(buf, 6);

    buf[0] = 0xC0; buf[1] = 0x02; // Set Mask
    memcpy(&buf[2], maskb, 4);
    ch9120_send_cmd(buf, 6);

    buf[0] = 0xC0; buf[1] = 0x03; // Set Gateway
    memcpy(&buf[2], gwb, 4);
    ch9120_send_cmd(buf, 6);
}

static void ch9120_set_dhcp(bool enable) {
    uint8_t cmd[3];
    cmd[0] = 0xC0; cmd[1] = 0x06; cmd[2] = enable ? 1 : 0;
    ch9120_send_cmd(cmd, 3);
}

void ethernet_init(const device_config_t *cfg) {
    // Init UART1 für CH9120
    uart_init(ETH_UART_ID, ETH_BAUD);
    gpio_set_function(ETH_TX, GPIO_FUNC_UART);
    gpio_set_function(ETH_RX, GPIO_FUNC_UART);

    // Konfiguration senden
    if (cfg->dhcp) {
        ch9120_set_dhcp(true);
    } else {
        ch9120_set_dhcp(false);
        ch9120_set_ip(cfg->ip, cfg->netmask, cfg->gateway);
    }

    // Ports konfigurieren:
    // UART Bridge auf 8080
    uint8_t cmd[5];
    cmd[0] = 0xC0; cmd[1] = 0x10; cmd[2] = 0x1F; cmd[3] = 0x90; // Port 8080
    ch9120_send_cmd(cmd, 4);

    // Telnet (Port 23) wird nicht hier gesetzt, nur wenn aktiviert
}

void ethernet_task(void) {
    // Placeholder für zukünftige Tasks (DHCP lease check etc.)
}

