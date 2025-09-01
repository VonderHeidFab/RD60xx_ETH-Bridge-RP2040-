/* =============================
 * PATCH: Telnet-Option + Control-Tab im Webserver
 * =============================
 * Änderungen:
 *  1) device_config_t um telnet_enabled erweitern
 *  2) Web-UI Tabs (Netzwerk, Bedienen, Service) + Telnet-Schalter
 *  3) Endpunkte: /control, /service, /api/set, /api/cmd, /api/va, /api/st
 *  4) UART Helper für synchrone Kurz-Kommandos (VV/VI/PO/VA/ST …)
 *  5) Telnet-Server (optional) zusätzlich zur immer aktiven Bridge (8080)
 *
 * Integration: Siehe unten stehende Header/Implementierungen und ersetze/ergänze
 * die vorhandenen Module entsprechend. Bridge (Port 8080) bleibt unverändert aktiv.
 */

/* --- webserver.h (Ergänzung) --- */
#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <stdbool.h>

typedef struct {
    bool dhcp;
    char ip[16];
    char netmask[16];
    char gateway[16];
    bool telnet_enabled; // NEU: optionaler Telnet-Server (Port 23)
} device_config_t;

void webserver_start(device_config_t *cfg);
void webserver_task(void);
#endif