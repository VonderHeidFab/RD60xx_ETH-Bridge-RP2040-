# RP2040 Netzteil Ethernet-Bridge & WebUI

Dieses Projekt implementiert eine Firmware für den Raspberry Pi RP2040 (z. B. Raspberry Pi Pico),
die ein **RD60062 (oder kompatibles)** Labornetzteil über UART anbindet und via Ethernet (CH9120)
sowohl **kompatibel zur Original-WLAN-Bridge (ESP12F, Port 8080)** als auch über ein modernes
**Webinterface (Port 8443)** steuerbar macht.

## Features

- **UART↔TCP-Bridge** (Port 8080) – 100% kompatibel zur Original-Firmware (ESP-12F)
- **Optionale Telnet-Schnittstelle** (Port 23) – separat über WebUI aktivierbar
- **WebUI (Port 8443)** mit drei Reitern:
  - **Netzwerk** – DHCP/Static, IP, Subnet, Gateway, Telnet aktivieren
  - **Bedienen** – Spannungs-/Stromvorgabe, Ausgang EIN/AUS, Messwerte, manuelle Befehle
  - **Service** – Infos, Werkseinstellungen, Backup
- **Konfiguration** wird im W25Q32-Flash und zusätzlich als `config.json` auf USB-Massenspeicher gespeichert
- **Factory Reset**: legt Backup auf USB ab, löscht Flash & USB-Config, rebootet
- **WS2812-Status-LEDs** (GPIO25): LED0 = Betriebsstatus, LED1 = RX/TX-Pulse

## Hardware

- **RP2040 (Pico oder Custom Board)**
- **W25Q32JVSSIQ (4 MB SPI-NOR)** für persistente Config
- **CH9120 Ethernet-UART-Bridge IC**
- **WS2812B LEDs (2 Stück in Reihe) an GPIO25**

## Ports

- `8080` – UART-Bridge (immer aktiv)
- `8443` – WebUI (TLS optional)
- `23` – Telnet (nur wenn per WebUI aktiviert)

## Projektstruktur

src/ – Quellcode (.c)
include/ – Header (.h)
docs/ – Dokumentation
webui/ – Web-UI (HTML/CSS/JS)
CMakeLists.txt
README.md


## Build

Projekt basiert auf dem **Pico SDK** und **lwIP**.

```bash
export PICO_SDK_PATH=/pfad/zum/pico-sdk
mkdir build && cd build
cmake ..
make

Das resultierende .uf2-File kann auf den RP2040 geflasht werden.

Lizenz

MIT License – frei verwendbar, bitte bei Forks/Anpassungen die Ursprungsquelle nennen.

WebUI Design anpassen

Alle HTML/CSS/JS-Dateien liegen im Ordner webui/.
Diese können einfach editiert oder ersetzt werden, um das Design der Weboberfläche anzupassen.
Der Webserver lädt die Dateien dynamisch von dort.

