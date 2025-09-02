// webserver.c – Minimal-Webserver auf Port 8443
#include "webserver.h"
#include "config.h"
#include "ethernet.h"
#include "ws2812.h"

#include "lwip/tcp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

static struct tcp_pcb *ws_pcb = NULL;
static device_config_t *ws_cfg;

static const char *http_ok_hdr =
    "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";

static const char *http_json_hdr =
    "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n";

static const char *http_404 =
    "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";

// --- Hilfsfunktion: JSON Status ---
static void send_status(struct tcp_pcb *tpcb) {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "{ \"dhcp\": %s, \"ip\": \"%s\", \"netmask\": \"%s\", \"gateway\": \"%s\", \"telnet\": %s }",
        ws_cfg->dhcp ? "true" : "false",
        ws_cfg->ip, ws_cfg->netmask, ws_cfg->gateway,
        ws_cfg->telnet_enabled ? "true" : "false"
    );
    tcp_write(tpcb, http_json_hdr, strlen(http_json_hdr), TCP_WRITE_FLAG_COPY);
    tcp_write(tpcb, buf, strlen(buf), TCP_WRITE_FLAG_COPY);
}

// --- Datei-Serving (aus webui/) ---
static void send_file(struct tcp_pcb *tpcb, const char *path) {
    char full[128];
    snprintf(full, sizeof(full), "webui/%s", path);

    FILE *f = fopen(full, "rb");
    if (!f) {
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
        return;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    if (len <= 0) {
        fclose(f);
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
        return;
    }

    char *buf = malloc(len);
    if (!buf) {
        fclose(f);
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
        return;
    }
    size_t read_bytes = fread(buf, 1, len, f);
    fclose(f);

    // Content-Type rudimentär
    const char *hdr = http_ok_hdr;
    if (strstr(path, ".css")) hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\n\r\n";
    else if (strstr(path, ".js")) hdr = "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\n\r\n";

    tcp_write(tpcb, hdr, strlen(hdr), TCP_WRITE_FLAG_COPY);
    tcp_write(tpcb, buf, read_bytes, TCP_WRITE_FLAG_COPY);
    free(buf);
}

// --- API Handler ---
static void handle_api(struct tcp_pcb *tpcb, const char *req) {
    if (strstr(req, "GET /api/status")) {
        send_status(tpcb);
    }
    else if (strstr(req, "POST /api/set")) {
        // Werte extrahieren (sehr simpel)
        if (strstr(req, "dhcp=true")) ws_cfg->dhcp = true;
        else ws_cfg->dhcp = false;

        const char *p;
        p = strstr(req, "ip=");     if (p) sscanf(p+3, "%15s", ws_cfg->ip);
        p = strstr(req, "netmask=");if (p) sscanf(p+8, "%15s", ws_cfg->netmask);
        p = strstr(req, "gateway=");if (p) sscanf(p+8, "%15s", ws_cfg->gateway);

        config_save(ws_cfg);
        ethernet_init(ws_cfg);

        tcp_write(tpcb, http_json_hdr, strlen(http_json_hdr), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, "{ \"ok\": true }", strlen("{ \"ok\": true }"), TCP_WRITE_FLAG_COPY);
    }
    else if (strstr(req, "POST /api/telnet")) {
        if (strstr(req, "enable=true")) ws_cfg->telnet_enabled = true;
        else ws_cfg->telnet_enabled = false;
        config_save(ws_cfg);

        tcp_write(tpcb, http_json_hdr, strlen(http_json_hdr), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, "{ \"ok\": true }", strlen("{ \"ok\": true }"), TCP_WRITE_FLAG_COPY);
    }
    else if (strstr(req, "POST /api/reset")) {
        config_reset();
        tcp_write(tpcb, http_json_hdr, strlen(http_json_hdr), TCP_WRITE_FLAG_COPY);
        tcp_write(tpcb, "{ \"reset\": true }", strlen("{ \"reset\": true }"), TCP_WRITE_FLAG_COPY);
        // reset_usb_boot(0, 0); // Entfernt, da Funktion nicht deklariert/importiert
    }
    else {
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
    }
}

// --- Request Handler ---
static err_t ws_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = malloc(p->tot_len + 1);
    if (!req) {
        pbuf_free(p);
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        return ERR_MEM;
    }
    pbuf_copy_partial(p, req, p->tot_len, 0);
    req[p->tot_len] = 0;
    pbuf_free(p);

    if (strstr(req, "GET /api/") || strstr(req, "POST /api/")) {
        handle_api(tpcb, req);
    } else if (strstr(req, "GET / ") || strstr(req, "GET /index.html")) {
        send_file(tpcb, "index.html");
    } else if (strstr(req, "GET /style.css")) {
        send_file(tpcb, "style.css");
    } else if (strstr(req, "GET /script.js")) {
        send_file(tpcb, "script.js");
    } else {
        tcp_write(tpcb, http_404, strlen(http_404), TCP_WRITE_FLAG_COPY);
    }

    free(req);
    tcp_output(tpcb);
    return ERR_OK;
}

static err_t ws_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, ws_recv);
    return ERR_OK;
}

void webserver_start(uint16_t port, device_config_t *cfg) {
    ws_cfg = cfg;
    ws_pcb = tcp_new();
    if (!ws_pcb) return;
    tcp_bind(ws_pcb, IP_ADDR_ANY, port);
    ws_pcb = tcp_listen(ws_pcb);
    tcp_accept(ws_pcb, ws_accept);
}

void webserver_stop(void) {
    if (ws_pcb) {
        tcp_close(ws_pcb);
        ws_pcb = NULL;
    }
}

void webserver_task(void) {
    // nichts nötig – lwIP callbacks erledigen Arbeit
}