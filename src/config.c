// config.c – Konfigurationsspeicherung (Flash + USB)
#include "config.h"

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"   // FatFS

// Speicheradresse im Flash (letzte 4kB Page)
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static device_config_t current_cfg;

// ---- Hilfsfunktionen JSON <-> struct ----
static bool parse_json(const char *json, device_config_t *cfg) {
    if (!json) return false;
    if (strstr(json, "\"dhcp\": true")) cfg->dhcp = true; else cfg->dhcp = false;

    const char *p;
    p = strstr(json, "\"ip\"");
    if (p) sscanf(p, "\"ip\" : \"%15[^\"]\"", cfg->ip);
    p = strstr(json, "\"netmask\"");
    if (p) sscanf(p, "\"netmask\" : \"%15[^\"]\"", cfg->netmask);
    p = strstr(json, "\"gateway\"");
    if (p) sscanf(p, "\"gateway\" : \"%15[^\"]\"", cfg->gateway);
    p = strstr(json, "\"telnet_enabled\"");
    if (p) {
        if (strstr(p, "true")) cfg->telnet_enabled = true;
        else cfg->telnet_enabled = false;
    }
    return true;
}

static void to_json(const device_config_t *cfg, char *buf, size_t buflen) {
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
    const uint8_t *flash_target_contents = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);
    char buf[512];
    memcpy(buf, flash_target_contents, sizeof(buf));
    return parse_json(buf, cfg);
}

static bool flash_save(const device_config_t *cfg) {
    char buf[512];
    to_json(cfg, buf, sizeof(buf));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, (const uint8_t*)buf, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    return true;
}

// ---- USB File Zugriff ----
static bool usb_save(const char *fname, const device_config_t *cfg) {
    FIL file;
    if (f_open(&file, fname, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return false;
    char buf[512];
    to_json(cfg, buf, sizeof(buf));
    UINT bw;
    f_write(&file, buf, strlen(buf), &bw);
    f_close(&file);
    return (bw > 0);
}

static bool usb_load(const char *fname, device_config_t *cfg) {
    FIL file;
    if (f_open(&file, fname, FA_READ) != FR_OK) return false;
    char buf[512];
    UINT br;
    f_read(&file, buf, sizeof(buf)-1, &br);
    f_close(&file);
    buf[br] = 0;
    return parse_json(buf, cfg);
}

// ---- Public API ----
bool config_load(device_config_t *cfg) {
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

    return true;
}

