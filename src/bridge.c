// bridge.c – UART <-> TCP Bridge auf Port 8080
#include "bridge.h"
#include "ws2812.h"

#include <string.h>

static struct tcp_pcb *bridge_pcb = NULL;
static struct tcp_pcb *bridge_client = NULL;
static uart_inst_t *bridge_uart = NULL;

#define BRIDGE_RX_BUF 256
#define BRIDGE_TX_BUF 256

static uint8_t rx_buf[BRIDGE_RX_BUF];
static uint8_t tx_buf[BRIDGE_TX_BUF];

static err_t bridge_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t bridge_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void bridge_err(void *arg, err_t err);

void bridge_start(uint16_t port, uart_inst_t *uart_id) {
    bridge_uart = uart_id;

    bridge_pcb = tcp_new();
    if (!bridge_pcb) return;
    tcp_bind(bridge_pcb, IP_ADDR_ANY, port);
    bridge_pcb = tcp_listen(bridge_pcb);
    tcp_accept(bridge_pcb, bridge_accept);
}

void bridge_stop(void) {
    if (bridge_client) {
        tcp_close(bridge_client);
        bridge_client = NULL;
    }
    if (bridge_pcb) {
        tcp_close(bridge_pcb);
        bridge_pcb = NULL;
    }
}

static err_t bridge_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (bridge_client) {
        tcp_abort(newpcb); // nur 1 Client erlaubt
        return ERR_ABRT;
    }
    bridge_client = newpcb;
    tcp_recv(bridge_client, bridge_recv);
    tcp_err(bridge_client, bridge_err);
    return ERR_OK;
}

static err_t bridge_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        bridge_client = NULL;
        return ERR_OK;
    }
    if (err != ERR_OK) {
        pbuf_free(p);
        return err;
    }

    // Daten aus TCP -> UART
    for (struct pbuf *q = p; q != NULL; q = q->next) {
        uart_write_blocking(bridge_uart, (const uint8_t*)q->payload, q->len);
        ws2812_pulse_tx(); // LED-Blink für TX
    }
    pbuf_free(p);
    return ERR_OK;
}

static void bridge_err(void *arg, err_t err) {
    bridge_client = NULL;
}

void bridge_task(void) {
    if (!bridge_client) return;

    // UART -> TCP
    while (uart_is_readable(bridge_uart)) {
        uint8_t c;
        uart_read_blocking(bridge_uart, &c, 1);
        ws2812_pulse_rx(); // LED-Blink für RX
        if (tcp_sndbuf(bridge_client) > 0) {
            char tmp[1] = {c};
            tcp_write(bridge_client, tmp, 1, TCP_WRITE_FLAG_COPY);
            tcp_output(bridge_client);
        }
    }
}
