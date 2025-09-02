// RP2040 + W25Q32JVSSIQ (4 MB SPI-NOR) + CH9120 (UART⇄Ethernet)
// Integriertes Projekt mit Webserver (8443) und UART-Bridge (8080)
// ---------------------------------------------------------------
// Features:
//  • Initialisiert SPI-Flash (liest JEDEC-ID, simple R/W/Erase Helfer)
//  • Webserver (Port 8443) zur Netzwerkkonfiguration (DHCP/static, IP, Subnet, Gateway, Factory Reset)
//  • Speichert Config in W25Q32 und als config.json auf USB-Massenspeicher
//  • Factory Reset legt Backup-Datei auf USB an
//  • WS2812-Kette (GPIO25, 2 LEDs): LED[0] Systemstatus, LED[1] RX/TX-Blink
//  • UART-Bridge auf Port 8080: TCP <-> UART (115200 Baud) für RD60062 ASCII-Protokoll
// ---------------------------------------------------------------

// main.c – Einstiegspunkt für RP2040 Netzteil-Ethernet-Bridge & WebUI

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"

#include "config.h"
#include "ethernet.h"
#include "bridge.h"
#include "telnet.h"
#include "webserver.h"
#include "ws2812.h"
#include <string.h>

// UART Settings for RD60062
#define PSU_UART_ID uart0
#define PSU_UART_TX 0
#define PSU_UART_RX 1
#define PSU_UART_BAUD 115200

int main() {
    stdio_init_all();

    // WS2812 LEDs init (2 LEDs, GPIO25)
    ws2812_init(25, 2);
    ws2812_set_status(WS2812_STATUS_BOOT);

    // UART init
    uart_init(PSU_UART_ID, PSU_UART_BAUD);
    gpio_set_function(PSU_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PSU_UART_RX, GPIO_FUNC_UART);

    // Load config
    device_config_t cfg;
    if (!config_load(&cfg)) {
        // Default config
        cfg.dhcp = false;
        strcpy(cfg.ip, "192.168.0.1");
        strcpy(cfg.netmask, "255.255.255.0");
        strcpy(cfg.gateway, "192.168.0.1");
        cfg.telnet_enabled = false;
        config_save(&cfg);
    }

    // Set LED according to network mode
    if (cfg.dhcp) {
        ws2812_set_status(WS2812_STATUS_DHCP);
    } else {
        ws2812_set_status(WS2812_STATUS_STATIC);
    }

    // Init Ethernet (CH9120)
    ethernet_init(&cfg);

    // Start Bridge (TCP 8080 <-> UART)
    bridge_start(8080, PSU_UART_ID);

    // Start Webserver (Port 8443)
    webserver_start(8443, &cfg);

    // Optional Telnet (Port 23)
    if (cfg.telnet_enabled) {
        telnet_start(23, PSU_UART_ID);
    }

    // Main loop
    while (true) {
        ethernet_task();
        bridge_task();
        webserver_task();
        if (cfg.telnet_enabled) {
            telnet_task();
        }
        ws2812_task();
        tight_loop_contents();
    }

    return 0;
}
