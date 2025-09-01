/* =================================================
 * bridge.h / bridge.c : TCP <-> UART Bridge Modul
 * ================================================= */

// bridge.h
#ifndef BRIDGE_H
#define BRIDGE_H
void bridge_start(int port, int baud);
void bridge_task(void);
#endif

// bridge.c
#include "bridge.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "lwip/tcp.h"
#include "ws2812.h"

#define UART_ID uart1
#define UART_TX_PIN 8
#define UART_RX_PIN 9

static struct tcp_pcb *bridge_listener = NULL;
static struct tcp_pcb *bridge_client = NULL;

static err_t bridge_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t bridge_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t bridge_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void  bridge_err(void *arg, err_t err);

void bridge_start(int port, int baud) {
    uart_init(UART_ID, baud);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    bridge_listener = tcp_new();
    if (!bridge_listener) return;
    tcp_bind(bridge_listener, IP_ANY_TYPE, port);
    bridge_listener = tcp_listen(bridge_listener);
    tcp_accept(bridge_listener, bridge_accept);
    printf("[BRIDGE] TCP<->UART Bridge gestartet auf Port %d, %d Baud\n", port, baud);
}

void bridge_task(void) {
    // UART -> TCP weiterleiten
    if (bridge_client && uart_is_readable(UART_ID)) {
        uint8_t c = uart_getc(UART_ID);
        tcp_write(bridge_client, &c, 1, TCP_WRITE_FLAG_COPY);
        tcp_output(bridge_client);
        led_rx_pulse();
    }
}

static err_t bridge_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    LWIP_UNUSED_ARG(arg); LWIP_UNUSED_ARG(err);
    bridge_client = newpcb;
    tcp_recv(newpcb, bridge_recv);
    tcp_sent(newpcb, bridge_sent);
    tcp_err(newpcb, bridge_err);
    printf("[BRIDGE] Client verbunden\n");
    return ERR_OK;
}

static err_t bridge_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) { bridge_client = NULL; return ERR_OK; }
    if (err != ERR_OK) { pbuf_free(p); return err; }
    struct pbuf *q = p;
    while (q) {
        uint8_t *d = (uint8_t*)q->payload;
        for (u16_t i = 0; i < q->len; i++) {
            uart_putc_raw(UART_ID, d[i]);
            led_tx_pulse();
        }
        q = q->next;
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t bridge_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    return ERR_OK;
}

static void bridge_err(void *arg, err_t err) {
    bridge_client = NULL;
}