#pragma once
#include "port_x.h"
#ifdef ARDUINO8266_SERVER_CPP
#include "homeintegration.h"

void hap_init_homekit_server();
void hap_homekit_loop();
bool hap_homekit_is_paired();
void hap_restart_server();
int hap_get_setup_uri(char *buffer, size_t buffer_size);
#endif
