/* --- webserver.c (kompakter Auszug mit neuen Tabs & APIs) --- */
// Füge am Kopf hinzu:
//   #include "telnet.h"
//   #include "hardware/uart.h"
//   #define UART_ID uart1
//   extern void ethernet_reconfigure(device_config_t*);
//   extern void led_tx_pulse(void); extern void led_rx_pulse(void);

// HTML: Tab-Navigation + dunkles Layout
//  - GET /            → Netzwerk-Seite (mit Telnet-Schalter)
//  - GET /control     → Bedienen-Seite (Manuelles Setzen/Lesen)
//  - GET /service     → Informationsseite
//  - POST /save       → Netzwerk+Telnet speichern
//  - POST /factory_reset
//  - POST /api/set    → Felder vv, vi, po (z.B. vv=12000&vi=2500&po=1)
//  - POST /api/cmd    → freier ASCII-Befehl (z.B. cmd=VR)
//  - GET  /api/va     → JSON { va: "12.000,1.250,15.00" }
//  - GET  /api/st     → JSON { st: "ON,CC,OVP_OK,OCP_OK" }

/* Hilfsfunktion: eine kurze UART-Transaktion (optional Antwort) */
static bool uart_send_cmd(const char *cmd, char *reply, size_t rlen, uint32_t timeout_ms) {
    for (const char *p=cmd; *p; ++p) { uart_putc_raw(UART_ID, *p); led_tx_pulse(); }
    if (!reply || rlen==0) return true;
    absolute_time_t until = make_timeout_time_ms(timeout_ms?timeout_ms:200);
    size_t i=0; while (absolute_time_diff_us(get_absolute_time(), until) > 0) {
        if (uart_is_readable(UART_ID)) { char ch = uart_getc(UART_ID); led_rx_pulse(); if (i+1<rlen) reply[i++]=ch; if (ch=='
') break; }
        tight_loop_contents();
    }
    if (i<rlen) reply[i]=' ';
    return i>0;
}

/* GET /control  → Bedienoberfläche */
static void respond_control(struct tcp_pcb *tpcb) {
    const char *page =
        "HTTP/1.1 200 OK
Content-Type: text/html; charset=UTF-8
Cache-Control: no-store

"
        "<!doctype html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<title>Bedienen</title><style>body{font-family:system-ui;background:#111;color:#eee;margin:0}"
        ".wrap{max-width:920px;margin:0 auto;padding:20px} .panel{background:#161616;border:1px solid #2a2a2a;padding:16px;border-radius:12px;margin-top:16px}"
        ".row{display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px} input,button{background:#181818;color:#eee;border:1px solid #333;padding:.6rem;border-radius:10px}"
        ".tabs{display:flex;border-bottom:1px solid #333;background:#1b1b1b} .tabs a{padding:12px 16px;color:#9ad;text-decoration:none} .tabs a.active{background:#222;border-bottom:2px solid #39f;color:#def}"
        ".big{font-size:1.25rem} .kbd{font-family:monospace;background:#222;border:1px solid #333;padding:2px 6px;border-radius:6px}"
        "</style></head><body><div class='wrap'>"
        "<div class='tabs'><a href='/' >Netzwerk</a><a class='active' href='/control'>Bedienen</a><a href='/service'>Service</a></div>"
        "<h2>Bedienen</h2><div class='panel'>"
        "<div class='row'>"
        "<div><label>Spannung (mV)</label><input id='vv' value='12000'></div>"
        "<div><label>Strom (mA)</label><input id='vi' value='1000'></div>"
        "<div><label>Status</label><span id='status' class='big'>–</span></div>"
        "</div>"
        "<div style='display:flex;gap:12px;margin-top:12px'>"
        "<button onclick=\"apply()\">Übernehmen</button>"
        "<button onclick=\"po(1)\">Ausgang EIN</button>"
        "<button onclick=\"po(0)\">Ausgang AUS</button>"
        "</div>"
        "</div><div class='panel'>"
        "<div class='row'><div><label>Messwerte</label><div id='va' class='big'>–</div></div>"
        "<div style='grid-column: span 2'><label>Manueller Befehl <span class='kbd'>(z.B. VR)</span></label>"
        "<input id='cmd' placeholder='VR, VA, ST, VV12000 ...'><button onclick=\"sendCmd()\">Senden</button></div></div>"
        "</div>"
        "<script>async function j(u,o){return fetch(u,o).then(r=>r.text())}"
        "async function refresh(){const st=await j('/api/st'); const va=await j('/api/va'); try{document.getElementById('status').innerText=JSON.parse(st).st||st}catch{} try{document.getElementById('va').innerText=JSON.parse(va).va||va}catch{}}"
        "async function apply(){const vv=document.getElementById('vv').value; const vi=document.getElementById('vi').value; await fetch('/api/set',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`vv=${'${'}vv}${'&'}vi=${'${'}vi}`}); refresh()}"
        "async function po(x){await fetch('/api/set',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`po=${'${'}x}`}); refresh()}"
        "async function sendCmd(){const c=document.getElementById('cmd').value; const t=await j('/api/cmd',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:`cmd=${'${'}encodeURIComponent(c)}`}); alert(t)}"
        "setInterval(refresh, 1000); refresh();"
        "</script>"
        "</div></body></html>";
    tcp_write(tpcb, page, (u16_t)strlen(page), TCP_WRITE_FLAG_COPY); tcp_output(tpcb); ws_close(tpcb);
}

/* In /save zusätzlich Telnet-Schalter auswerten und telnet_start/stop aufrufen */
// if (s_cfg->telnet_enabled) telnet_start(23, 115200); else telnet_stop();

/* API-Handler (POST /api/set) */
static void handle_post_set(struct tcp_pcb *tpcb, const char *body) {
    char sv[16]={0}, si[16]={0}, po[8]={0};
    kv_get(body, "vv", sv, sizeof sv);
    kv_get(body, "vi", si, sizeof si);
    kv_get(body, "po", po, sizeof po);
    char cmd[64];
    if (*sv) { snprintf(cmd, sizeof cmd, "VV%s
", sv); uart_send_cmd(cmd, NULL, 0, 0); }
    if (*si) { snprintf(cmd, sizeof cmd, "VI%s
", si); uart_send_cmd(cmd, NULL, 0, 0); }
    if (*po) { snprintf(cmd, sizeof cmd, "PO%s
", po); uart_send_cmd(cmd, NULL, 0, 0); }
    respond_303(tpcb, "/control");
}

/* API-Handler (POST /api/cmd) */
static void handle_post_cmd(struct tcp_pcb *tpcb, const char *body) {
    char c[64]={0}; kv_get(body, "cmd", c, sizeof c);
    char rep[160]={0}; bool ok=false; if (*c){ size_t n=strlen(c); if (n<2 || c[n-1]!='
') strcat(c, "
"); ok=uart_send_cmd(c, rep, sizeof rep, 250);} 
    char out[256]; snprintf(out, sizeof out, "HTTP/1.1 200 OK
Content-Type: text/plain

%s", ok?rep:"ERR");
    tcp_write(tpcb, out, (u16_t)strlen(out), TCP_WRITE_FLAG_COPY); tcp_output(tpcb); ws_close(tpcb);
}

/* API-Handler (GET /api/va & /api/st) */
static void respond_json_va(struct tcp_pcb *tpcb) { char rep[128]={0}; if(!uart_send_cmd("VA
", rep, sizeof rep, 200)) strcpy(rep,"null"); char out[192]; snprintf(out,sizeof out,"HTTP/1.1 200 OK
Content-Type: application/json

{\"va\":\"%s\"}",rep); tcp_write(tpcb,out,(u16_t)strlen(out),TCP_WRITE_FLAG_COPY); tcp_output(tpcb); ws_close(tpcb);} 
static void respond_json_st(struct tcp_pcb *tpcb) { char rep[128]={0}; if(!uart_send_cmd("ST
", rep, sizeof rep, 200)) strcpy(rep,"null"); char out[192]; snprintf(out,sizeof out,"HTTP/1.1 200 OK
Content-Type: application/json

{\"st\":\"%s\"}",rep); tcp_write(tpcb,out,(u16_t)strlen(out),TCP_WRITE_FLAG_COPY); tcp_output(tpcb); ws_close(tpcb);}