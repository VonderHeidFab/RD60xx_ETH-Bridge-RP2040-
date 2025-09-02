// webserver.h – einfacher HTTP/Webserver für WebUI und API

#pragma once
#include "config.h"

void webserver_start(uint16_t port, device_config_t *cfg);
void webserver_stop(void);
void webserver_task(void);
