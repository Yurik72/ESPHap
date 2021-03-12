#pragma once

#include "port_x.h"



#include <stdlib.h>
#include <stdio.h>
#include "elgato.h"
#include "homekit.h"
#include "characteristics.h"
#include "storage.h"
#include "storage_ex.h"
#ifdef ARDUINO8266_SERVER_CPP
//#include "arduino_homekit_server.h"
#endif


#define INFO(message, ...) printf(">>> Home Integration: " message "\n", ##__VA_ARGS__)

#define hap_constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#ifndef HAP_NOTIFY_CHANGES
#define HAP_NOTIFY_CHANGES(name,home_characteristic,val,tollerance) \
	if (home_characteristic && abs(home_characteristic->value.name ##_value - val)>tollerance){ \
		home_characteristic->value.name ##_value = val ;\
		homekit_characteristic_notify(home_characteristic, home_characteristic->value); \
	};
#endif
#ifndef HAP_NOTIFY_CHANGES_WITHCONSTRAIN
#define HAP_NOTIFY_CHANGES_WITHCONSTRAIN(name,home_characteristic,val,tollerance) \
    name cval=hap_constrain(val,*home_characteristic->min_value,*home_characteristic->max_value); \
	if (home_characteristic && abs(home_characteristic->value.name ##_value - cval)>tollerance){ \
		home_characteristic->value.name ##_value = cval ;\
		homekit_characteristic_notify(home_characteristic, home_characteristic->value); \
	};
#endif


#define HAP_IMPLEMENT_GETVAL(name,type,service,characteristic)\
type name() { \
	if (service) { \
		homekit_characteristic_t * ch = homekit_service_characteristic_by_type(service, characteristic);\
		if (ch) {\
			return ch->value.type ##_value;\
		}\
	}\
};


	void init_accessory();
	void init_homekit_server();

	typedef void(*callback_storagechanged)(char * szstorage, int size);
	void set_callback_storage_change(callback_storagechanged fn);
	callback_storagechanged get_callback_storage_change();
	void hap_set_max_pairing(byte val);
	int hap_init_storage_ex(char* szdata, int size);
	int hap_get_storage_size_ex();
	void hap_set_identity_gpio(int gpio);
	void hap_set_device_password(char* szpwd);

	void hap_set_device_setupId(char* szpwd);


	bool hap_set_charachteristic_validrange(homekit_characteristic_t * ch, float min, float max);
	bool hap_set_charachteristic_validrange_by_type(homekit_service_t *service,const char *type, float min, float max);

	//esp controller usage
	int hap_initbase_accessory_service(const char* szname_value, const char* szmanufacturer, const char* szserialnumber, const char* szmodels, const char* szfirmware);
	homekit_service_t* hap_new_homekit_accessory_service(const char *szname, const char *szserialnumber);
	typedef void(*hap_callback)(homekit_characteristic_t *ch, homekit_value_t value, void *context);
	homekit_service_t* hap_add_lightbulb_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_lightbulb_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_lightbulb_dim_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t*  hap_add_relaydim_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_relaydim_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t*  hap_add_lightbulb_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);


	//RGB 
	homekit_service_t* hap_add_rgbstrip_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t*  hap_add_rgbstrip_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_rgbstrip_service(const char* szname, hap_callback cb, void* context);

	homekit_service_t* hap_add_rgbstrip_service_ex(const char* szname, hap_callback cb, void* context);
	homekit_service_t*  hap_add_rgbstrip_service_as_accessory_ex(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_rgbstrip_service_ex(const char* szname, hap_callback cb, void* context);

	//input source
	homekit_service_t* hap_new_inputsource_service(const char* szname, const char* szconfigname, int value, hap_callback cb, void* context);

	//Thermo
	
	homekit_service_t* hap_add_temperature_service(const char* szname);

	homekit_service_t* hap_add_thermostat_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_thermostat_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_thermostat_service(const char* szname, hap_callback cb, void* context);
	// heater cooler
#define HEATER_COOLER_STATE_AUTO 0
#define HEATER_COOLER_STATE_HEAT 1
#define HEATER_COOLER_STATE_COOL 2

	homekit_service_t* hap_add_heater_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_heater_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_heater_service(const char* szname, hap_callback cb, void* context);


	homekit_service_t* hap_add_humidity_service(const char* szname);
	homekit_service_t*  hap_add_temp_hum_as_accessory(int acctype, const char* szname, homekit_service_t** pp_temp, homekit_service_t** pp_hum);
	homekit_service_t*  hap_add_hum_as_accessory(int acctype, const char* szname);
	homekit_service_t*  hap_add_temp_as_accessory(int acctype, const char* szname);
	homekit_service_t* hap_add_pressure_service(const char* szname);

	homekit_service_t* hap_add_light_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_light_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t*  hap_add_light_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);

	homekit_service_t* hap_add_battery_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_battery_service(const char* szname, hap_callback cb, void* context);
	
	//switch
	homekit_service_t* hap_add_switch_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_switch_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_switch_service_as_accessory(const char* szname, hap_callback cb, void* context);
	
	//button
	homekit_service_t* hap_add_button_service(const char* szname);// , hap_callback cb, void* context);
	homekit_service_t* hap_new_button_service(const char* szname);// , hap_callback cb, void* context);

	//motion
	homekit_service_t* hap_new_motion_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_motion_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_motion_service_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);

	//fan
	homekit_service_t* hap_new_fan_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_fan_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_new_fan2_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_fan2_service(const char* szname, hap_callback cb, void* context);
	//air quality
	homekit_service_t* hap_new_air_quality_service(const char* szname/*, hap_callback cb, void* context*/);
	homekit_service_t* hap_add_air_quality_service(const char* szname/*, hap_callback cb, void* context*/);
	homekit_service_t*  hap_add_air_quality_service_as_accessory(int acctype, const char* szname/*, hap_callback cb, void* context*/);
	//garage door
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN 0
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED 1
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING 2
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING 3
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_STOPPED 4
	#define HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN 255

	#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN 0
	#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED 1
	#define HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN 255
	homekit_service_t* hap_new_garagedoor_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_garagedoor_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_garagedoor_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);


	//Window Covering
#define WINDOWCOVERING_POSITION_OPEN 100
#define WINDOWCOVERING_POSITION_CLOSED 0
#define WINDOWCOVERING_POSITION_STATE_CLOSING 0
#define WINDOWCOVERING_POSITION_STATE_OPENING 1
#define WINDOWCOVERING_POSITION_STATE_STOPPED 2

	homekit_service_t* hap_new_windowcovering_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_windowcovering_service(const char* szname, hap_callback cb, void* context);
	homekit_service_t* hap_add_windowcovering_as_accessory(int acctype, const char* szname, hap_callback cb, void* context);
	homekit_characteristic_t* hap_add_hold_characteristik_to_windowcovering(homekit_service_t* s, hap_callback cb, void* context);


	homekit_service_t* hap_add_service(homekit_service_t* service);

    homekit_accessory_t *hap_add_accessory(
            int acctype, homekit_service_t *services[]);

	void hap_setbase_accessorytype(int val);

	//initial value
#define INIT_CHARACHTERISTIC_VAL(type,ch,val) \
		hap_set_initial_characteristic_##type##_value(ch,val);
#define INIT_CHARACHTERISTIC_VAL_BY_TYPE(type,service,ch,val) \
		hap_setinitial_characteristic_##type##_value(service,ch,val);

	void hap_setinitial_characteristic_int_value(homekit_service_t* s, const char *type, int val);
	void hap_setinitial_characteristic_bool_value(homekit_service_t* s, const char *type, bool val);

	void hap_set_initial_characteristic_int_value(homekit_characteristic_t* ch, int val);
	void hap_set_initial_characteristic_bool_value(homekit_characteristic_t* ch, bool bval);
	//elgato
	homekit_service_t* hap_add_elgatosupport_service(const char* szname, hap_callback cb, void* context);

#ifndef ARDUINO8266_SERVER_CPP
	void hap_init_homekit_server();
	bool hap_homekit_is_paired();
	void hap_restart_server();

	
#endif
	void hap_init_homekit_base_accessory();

	homekit_server_config_t* hap_get_server_config();
	bool hap_setup_final_step();
#ifndef ARDUINO8266_SERVER_CPP
	int hap_get_setup_uri(char *buffer, size_t buffer_size);
	void hap_setstopflag();
#endif

	homekit_value_t HOMEKIT_UINT8_VALUE(uint8_t value);
	int set_wifi_max_power();
	int set_wifi_save_power_middle(void);
	int set_wifi_save_power(int8_t level);
	