# Dokumentation

## Bedienung über WebUI
- Netzwerk konfigurieren (DHCP/Static, IP, Subnet, Gateway)
- Telnet optional aktivieren
- Spannungs-/Stromvorgabe setzen, Ausgang EIN/AUS
- Live-Messwerte und Status einsehen
- Manuelle Befehle senden (ASCII-Protokoll RD60062, siehe Protokoll-Handbuch)

## Werkseinstellungen
- Über WebUI → Service → Werkseinstellungen laden
- Alte Config wird als `config_backup.json` auf USB gespeichert

## LED-Anzeigen
- LED0 (WS2812): Systemstatus (weiß=Boot, blau=DHCP, grün=Static)
- LED1 (WS2812): RX/TX Blinken (Rot=RX, Blau=TX)
