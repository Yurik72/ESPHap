#pragma once
#include <Arduino.h>

extern "C" {
#include "../homeintegration.h"
}

#define SETUP_BUFSIZE 22
String get_setup_json() {

	char buf[SETUP_BUFSIZE];
	hap_get_setup_uri(buf, SETUP_BUFSIZE);

	String res = "";
	res += "{\"setup_uri\":\"";
	
	res +=String(buf);
	res += "\",\"pin\":\"";
	res += hap_get_server_config()->password;
	res += "\"}";
	return res;
}