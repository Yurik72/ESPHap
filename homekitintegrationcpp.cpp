#include "port_x.h"
#ifdef ARDUINO8266_SERVER_CPP
extern "C" {
#include "homeintegration.h"
}
#include "arduino_homekit_server.h"
void hap_init_homekit_server()
{
	hap_setup_final_step();
	homekit_server_config_t* cfg = hap_get_server_config();
	arduino_homekit_setup(cfg);
}
void hap_homekit_loop() {
	arduino_homekit_loop();
}
bool hap_homekit_is_paired() {
	return arduino_homekit_is_paired();
}
#endif