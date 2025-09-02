// config.h – Laden/Speichern von Netzwerkkonfiguration

#pragma once
#include "pico/stdlib.h"
#include <stdbool.h>

typedef struct {
    bool dhcp;
    char ip[16];
    char netmask[16];
    char gateway[16];
    bool telnet_enabled;
} device_config_t;

bool config_load(device_config_t *cfg);
bool config_save(const device_config_t *cfg);
bool config_reset(void);

