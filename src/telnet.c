/* --- telnet.c (minimal) --- */
#include "telnet.h"
#include "lwip/tcp.h"
#include "hardware/uart.h"
#include "ws2812.h"
#define UART_ID uart1
static struct tcp_pcb *tn_listener=NULL,*tn_client=NULL; static err_t tn_accept(void*,struct tcp_pcb*,err_t); static err_t tn_recv(void*,struct tcp_pcb*,struct pbuf*,err_t); static void tn_err(void*,err_t);
void telnet_start(int port, int baud){(void)baud; if(tn_listener) return; tn_listener=tcp_new(); if(!tn_listener) return; tcp_bind(tn_listener,IP_ANY_TYPE,port); tn_listener=tcp_listen(tn_listener); tcp_accept(tn_listener,tn_accept);} 
void telnet_stop(void){ if(tn_client){tcp_close(tn_client); tn_client=NULL;} if(tn_listener){tcp_close(tn_listener); tn_listener=NULL;} }
static err_t tn_accept(void*arg,struct tcp_pcb*pcb,err_t e){(void)arg;(void)e; tn_client=pcb; tcp_recv(pcb,tn_recv); tcp_err(pcb,tn_err); return ERR_OK;}
static err_t tn_recv(void*arg,struct tcp_pcb*tpcb,struct pbuf*p,err_t e){ if(!p){tn_client=NULL; return ERR_OK;} if(e!=ERR_OK){pbuf_free(p); return e;} for(struct pbuf*q=p;q;q=q->next){uint8_t *d=(uint8_t*)q->payload; for(u16_t i=0;i<q->len;i++){uint8_t c=d[i]; if(c==255){ if(i+2<q->len){ i+=2; continue; } else break; } uart_putc_raw(UART_ID,c); led_tx_pulse(); }} pbuf_free(p); return ERR_OK;}
static void tn_err(void*arg,err_t e){(void)arg;(void)e; tn_client=NULL;}