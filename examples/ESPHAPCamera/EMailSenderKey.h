#ifndef EMailSenderKey_h
#define EMailSenderKey_h

// Uncomment if you use esp8266 core <= 2.4.2
//#define ARDUINO_ESP8266_RELEASE_2_4_2

#define ENABLE_ATTACHMENTS

// Uncomment to enable printing out nice debug messages.
//#define EMAIL_SENDER_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

#define NETWORK_ESP8266_ASYNC (0)
#define NETWORK_ESP8266 (1)
#define NETWORK_ESP8266_242 (6)
#define NETWORK_W5100 (2)
#define NETWORK_ENC28J60 (3)
#define NETWORK_ESP32 (4)
#define NETWORK_ESP32_ETH (5)

#ifndef DEFAULT_EMAIL_NETWORK_TYPE_ESP8266
	#define DEFAULT_EMAIL_NETWORK_TYPE_ESP8266 	NETWORK_ESP8266
#endif
#ifndef DEFAULT_EMAIL_NETWORK_TYPE_ESP32
	#define DEFAULT_EMAIL_NETWORK_TYPE_ESP32 	NETWORK_ESP32
#endif
#ifndef DEFAULT_EMAIL_NETWORK_TYPE_ARDUINO
	#define DEFAULT_EMAIL_NETWORK_TYPE_ARDUINO 	NETWORK_ENC28J60
#endif

#endif
