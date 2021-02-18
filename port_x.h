#pragma once
//#define HOMEKIT_DEBUG   // should be defined if we  want to debug Homekit
#define CJSON_USE_PREALLOCATED_BUFFER   // instruct to use static (on the stack ) buffer for cJSON parsing
#define TLV_USE_PREALLOCATED_BUFFER     //instruct to  to use static (on the stack )  buffer for TLV values parsing
#define PAIRVERIFY_USE_MEMORYBUFFER   
#define USE_STACK_BUFFER               //instruct to use stack buffer on some operation, like sending responce etc
#define USE_STATIC_HTTP_BODY           //instruct to use static buffer when HTTP parser parsing a body
#define CLOSE_DUPLICATED_CONNECTION  1  //instruct that previous connection with the same ip should be closed
#define USE_STATIC_BUFFER_WRITE_JSON_CHARACHTERISTIC    //instruct use static buffer when base64 encoded for write charachteristik

#define DEBUG_MEMORY
#if defined(ARDUINO) && defined(ESP8266)
#define ARDUINO8266_SERVER

#define ARDUINO8266_SERVER_CPP

//#define HOMEKIT_DEBUG
#define HOMEKIT_SHORT_APPLE_UUIDS
#define HOMEKIT_OVERCLOCK_PAIR_SETUP
#endif 