// config.c – Konfigurationsspeicherung (Flash + USB)
#include "config.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ffconf.h"
#include "ff.h"

// Speicheradresse im Flash (letzte 4kB Page)
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static device_config_t current_cfg;

// ---- Hilfsfunktionen JSON <-> struct ----
static bool parse_json(const char *json, device_config_t *cfg) {
    if (!json || !cfg) return false;
    cfg->dhcp = strstr(json, "\"dhcp\": true") ? true : false;

    const char *p;
    p = strstr(json, "\"ip\"");
    if (p) sscanf(p, "\"ip\" : \"%15[^\"]\"", cfg->ip); else strcpy(cfg->ip, "");
    p = strstr(json, "\"netmask\"");
    if (p) sscanf(p, "\"netmask\" : \"%15[^\"]\"", cfg->netmask); else strcpy(cfg->netmask, "");
    p = strstr(json, "\"gateway\"");
    if (p) sscanf(p, "\"gateway\" : \"%15[^\"]\"", cfg->gateway); else strcpy(cfg->gateway, "");
    p = strstr(json, "\"telnet_enabled\"");
    if (p) {
        cfg->telnet_enabled = strstr(p, "true") ? true : false;
    } else {
        cfg->telnet_enabled = false;
    }
    return true;
}

static void to_json(const device_config_t *cfg, char *buf, size_t buflen) {
    if (!cfg || !buf || buflen == 0) return;
    snprintf(buf, buflen,
        "{\n"
        "  \"dhcp\": %s,\n"
        "  \"ip\": \"%s\",\n"
        "  \"netmask\": \"%s\",\n"
        "  \"gateway\": \"%s\",\n"
        "  \"telnet_enabled\": %s\n"
        "}\n",
        cfg->dhcp ? "true" : "false",
        cfg->ip, cfg->netmask, cfg->gateway,
        cfg->telnet_enabled ? "true" : "false"
    );
}

// ---- Flash Zugriff ----
static bool flash_load(device_config_t *cfg) {
    if (!cfg) return false;
    const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    char buf[512];
    memcpy(buf, flash_target_contents, sizeof(buf));
    buf[sizeof(buf)-1] = 0; // Buffer sicher terminieren
    return parse_json(buf, cfg);
}

static bool flash_save(const device_config_t *cfg) {
    if (!cfg) return false;
    char buf[512];
    memset(buf, 0, sizeof(buf)); // Buffer mit Nullen füllen
    to_json(cfg, buf, sizeof(buf));
    size_t len = strlen(buf);
    size_t write_len = ((len + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE; // Aufrunden auf Page-Größe

    // Buffer auf write_len auffüllen, damit keine alten Datenreste im Flash stehen bleiben
    if (write_len > sizeof(buf)) write_len = sizeof(buf);
    memset(buf + len, 0, write_len - len);

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)buf, write_len);
    restore_interrupts(ints);
    return true;
}

// ---- USB File Zugriff ----
static bool usb_save(const char *fname, const device_config_t *cfg) {
    if (!fname || !cfg) return false;
    FIL file;
    if (f_open(&file, fname, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return false;
    char buf[512];
    to_json(cfg, buf, sizeof(buf));
    UINT bw = 0;
    FRESULT res = f_write(&file, buf, strlen(buf), &bw);
    f_close(&file);
    return (res == FR_OK && bw > 0);
}

static bool usb_load(const char *fname, device_config_t *cfg) {
    if (!fname || !cfg) return false;
    FIL file;
    if (f_open(&file, fname, FA_READ) != FR_OK) return false;
    char buf[512];
    UINT br = 0;
    FRESULT res = f_read(&file, buf, sizeof(buf)-1, &br);
    f_close(&file);
    if (res != FR_OK || br == 0) return false;
    buf[br] = 0;
    return parse_json(buf, cfg);
}

// ---- Public API ----
bool config_load(device_config_t *cfg) {
    if (!cfg) return false;
    // Erst von USB versuchen
    if (usb_load("config.json", cfg)) {
        memcpy(&current_cfg, cfg, sizeof(device_config_t));
        return true;
    }
    // Dann aus Flash
    if (flash_load(cfg)) {
        memcpy(&current_cfg, cfg, sizeof(device_config_t));
        return true;
    }
    return false;
}

bool config_save(const device_config_t *cfg) {
    if (!cfg) return false;
    flash_save(cfg);
    usb_save("config.json", cfg);
    memcpy(&current_cfg, cfg, sizeof(device_config_t));
    return true;
}

bool config_reset(void) {
    // Backup auf USB
    usb_save("config_backup.json", &current_cfg);

    // USB config löschen
    f_unlink("config.json");

    // Flash löschen
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);

    // current_cfg auf Default setzen (optional, falls gewünscht)
    memset(&current_cfg, 0, sizeof(current_cfg));

    return true;
}

