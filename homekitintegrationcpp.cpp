#include "port_x.h"
#include "esphap_debug.h"
#ifdef ARDUINO8266_SERVER_CPP
extern "C" {
#include "homeintegration.h"
}
#include "arduino_homekit_server.h"
void hap_init_homekit_server()
{
	hap_setup_final_step();
	
	INFO("homekit_is_paired %d", arduino_homekit_is_paired());
	homekit_server_config_t* cfg = hap_get_server_config();
	arduino_homekit_setup(cfg);
}
void hap_homekit_loop() {
	arduino_homekit_loop();
}
bool hap_homekit_is_paired() {
	return arduino_homekit_is_paired();
}
void hap_restart_server() {
	homekit_server_restart();
}
int hap_get_setup_uri(char *buffer, size_t buffer_size) {
	int res = homekit_get_setup_uri(hap_get_server_config(), buffer, buffer_size);
	//INFO("hap_get_setup_uri returned %d", res);
	return res;
}

#endif