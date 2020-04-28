#pragma once

//#define BTN_DEBUG

 #if defined(ESP32) || defined(ESP8266)
#define INFO(message, ...) Serial.printf(">>> Sketch: " message "\n", ##__VA_ARGS__)
#define ERROR(message, ...) Serial.printf("!!! Sketch: " message "\n", ##__VA_ARGS__)
#else
void format_print(char* msg, size_t val=0) {
	char buf[512];
	sprintf(buf, msg, val);
	Serial.print(buf);
};
#define INFO(message, val)  format_print(">>> button: " message "\n", val) // Serial.print(">>> button: " message "\n");
#define ERROR(message, val) format_print("!!! button: " message "\n");
#endif
#ifdef BTN_DEBUG
 #if defined(ESP32) || defined(ESP8266)
#define DEBUG(message, ...) Serial.printf_P(PSTR(">>> %s: " message "\n"), __func__, ##__VA_ARGS__)
#else
#define DEBUG(message, ...) format_print("--- button: " message "\n");
#endif
#else

#define DEBUG(message, ...)

#endif
