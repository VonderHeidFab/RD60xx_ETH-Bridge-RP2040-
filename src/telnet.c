// telnet.c – Einfacher Telnet-Server für UART
#include "telnet.h"
#include "ws2812.h"

#include <string.h>

static struct tcp_pcb *telnet_pcb = NULL;
static struct tcp_pcb *telnet_client = NULL;
static uart_inst_t *telnet_uart = NULL;

static err_t telnet_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t telnet_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void telnet_err(void *arg, err_t err);

void telnet_start(uint16_t port, uart_inst_t *uart_id) {
    telnet_uart = uart_id;

    telnet_pcb = tcp_new();
    if (!telnet_pcb) return;
    tcp_bind(telnet_pcb, IP_ADDR_ANY, port);
    telnet_pcb = tcp_listen(telnet_pcb);
    tcp_accept(telnet_pcb, telnet_accept);
}

void telnet_stop(void) {
    if (telnet_client) {
        tcp_close(telnet_client);
        telnet_client = NULL;
    }
    if (telnet_pcb) {
        tcp_close(telnet_pcb);
        telnet_pcb = NULL;
    }
}

static err_t telnet_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    if (telnet_client) {
        tcp_abort(newpcb); // nur 1 Client erlaubt
        return ERR_ABRT;
    }
    telnet_client = newpcb;
    tcp_recv(telnet_client, telnet_recv);
    tcp_err(telnet_client, telnet_err);
    return ERR_OK;
}

static err_t telnet_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        telnet_client = NULL;
        return ERR_OK;
    }
    if (err != ERR_OK) {
        pbuf_free(p);
        return err;
    }

    // Daten aus TCP -> UART
    for (struct pbuf *q = p; q != NULL; q = q->next) {
        uint8_t *data = (uint8_t*)q->payload;
        for (u16_t i = 0; i < q->len; i++) {
            if (data[i] == 0xFF) {
                // Telnet IAC überspringen
                continue;
            }
            uart_write_blocking(telnet_uart, &data[i], 1);
            ws2812_pulse_tx(); // TX-LED
        }
    }
    pbuf_free(p);
    return ERR_OK;
}

static void telnet_err(void *arg, err_t err) {
    telnet_client = NULL;
}

void telnet_task(void) {
    if (!telnet_client) return;

    // UART -> TCP
    while (uart_is_readable(telnet_uart)) {
        uint8_t c;
        uart_read_blocking(telnet_uart, &c, 1);
        ws2812_pulse_rx(); // RX-LED
        if (tcp_sndbuf(telnet_client) > 0) {
            char tmp[1] = {c};
            tcp_write(telnet_client, tmp, 1, TCP_WRITE_FLAG_COPY);
            tcp_output(telnet_client);
        }
    }
}
