#pragma once

//#define WIFI_MANAGER  // will support captive portal to establish wi fi connection
#define ESP_WIFI_MANAGER  // will support captive portal to establish wi fi connection


#define SUPPORT_RF  /// this option will enable RF transmitter
#define SUPPORT_GPIO   ///This will enable option LOW/HIGH when face recognized on specified GPIO
#define INTERNAL_LOOP   // this will enable internal loop without http request via web scoket
#define SUPPORT_SERVO

#define HOSTNAME "ESPCAM"

#define ENABLE_HAP   /// Define if device should support Apple Home Kit connection

/// define e-mail settings
//#define EMAIL_NOT_SSL // define if e-mail is not required security

#define EMAIL_SERVER "mail.xxx.xxx"
#define EMAIL_SERVER_PORT 465
#define EMAIL_SERVER_USER "admin@xxx.xxx"
#define EMAIL_SERVER_PWD "PWD"
#define EMAIL_FROM  "admin@xxx.xxx"
#define EMAIL_TO  "admin@xxx.xxx"


//#define DEBUG_FRAMES
