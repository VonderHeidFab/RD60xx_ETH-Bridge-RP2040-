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

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "ws2812.h"
#include "config.h"
#include "webserver.h"
#include "ethernet.h"
#include "bridge.h"

#define UART_BAUDRATE 115200

int main() {
    stdio_init_all();
    sleep_ms(500);

    // LEDs
    led_init();
    struct repeating_timer led_timer;
    add_repeating_timer_ms(10, led_timer_cb, NULL, &led_timer);
    led_set(0, 255, 255, 255); led_show(); // Boot: weiß

    // Config laden
    device_config_t cfg;
    if (!config_load(&cfg)) {
        strcpy(cfg.ip, "192.168.0.1");
        strcpy(cfg.netmask, "255.255.255.0");
        strcpy(cfg.gateway, "192.168.0.1");
        cfg.dhcp = false;
        config_save(&cfg);
    }

    if (cfg.dhcp) led_set(0, 0, 0, 255); else led_set(0, 0, 255, 0);
    led_show();

    // Ethernet Init (CH9120) mit Config
    ethernet_init(&cfg);

    // Webserver Port 8443
    webserver_start(&cfg);

    // UART Bridge auf Port 8080
    bridge_start(8080, UART_BAUDRATE);

    while (true) {
        ethernet_task();
        webserver_task();
        bridge_task();
    }
}
/* --- main.c (Ausschnitt) --- */
// Nach dem Laden der Config:
//   if (cfg.telnet_enabled) telnet_start(23, UART_BAUDRATE);
// Bridge (Port 8080) bleibt wie gehabt dauerhaft aktiv.

// main.c – Einstiegspunkt
#include "pico/stdlib.h"
int main(){while(1){tight_loop_contents();}}
