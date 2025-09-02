// ethernet.h – CH9120 Ethernet Initialisierung

#pragma once
#include "pico/stdlib.h"
#include "config.h"

void ethernet_init(const device_config_t *cfg);
void ethernet_task(void);

