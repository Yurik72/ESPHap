/// BASIC CONFIGURATION 

#define ENABLE_WIFI_MANAGER  // if we want to have built-in wifi configuration
                             // Otherwise direct connect ssid and pwd will be used
                             // for Wifi manager need extra library //https://github.com/tzapu/WiFiManager

#define ENABLE_WEB_SERVER    //if we want to have built in web server /site
#define ENABLE_OTA  //if Over the air update need  , ENABLE_WEB_SERVER must be defined first
#include <Arduino.h>


#ifdef ESP32
#include <SPIFFS.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif



#ifdef ENABLE_WEB_SERVER
#ifdef ESP8266
#include <ESP8266WebServer.h>
 ESP8266WebServer server(80);
#endif

#ifdef ESP32
#include <WebServer.h>
WebServer server(80);
#endif
#endif 

#if defined(ESP32) && defined(ENABLE_OTA)
#include <Update.h>
#endif

#ifdef ENABLE_WEB_SERVER
#include "spiffs_webserver.h"
bool isWebserver_started=false;
#endif 

const int button_gpio=4;


#ifdef ENABLE_WIFI_MANAGER
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#endif






const char* HOSTNAME="ES";
const char* ssid     = "ssid";
const char* password = "pwd";

extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage\hapfilestorage.hpp>
#include "CSimpleButton.h"


homekit_service_t* hapservice={0};

String pair_file_name="/pair.dat";



void button_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
  // nothing

}
void buttonhandler_callback(uint8_t gpio, button_event_t event) {

Serial.print("buttonhandler_callback");
Serial.println(gpio);
  switch(event){
    case button_event_single_press:
       notifyHap(0);
      break;
    case button_event_long_press:
      notifyHap(1);
      break;
  }
}
//Web server section
#define ENABLE_OTA  //if OTA need



CSimpleButton btn;

void setup() {
 #ifdef ESP8266 
  disable_extra4k_at_link_time();
 #endif 
  Serial.begin(115200);
    delay(10);

#ifdef ESP32
     if (!SPIFFS.begin(true)) {
      // Serial.print("SPIFFS Mount failed");
     }
#endif
#ifdef ESP8266
     if (!SPIFFS.begin()) {
      Serial.print("SPIFFS Mount failed");
     }
#endif


    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());


    

    
   // init_hap_storage();
   init_hap_storage("/pair.dat");


    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_programmable_switch);
    /// init base properties
    hap_initbase_accessory_service("host","Yurik72","0","EspHapButton","1.0");

   //we will add only one light bulb service and keep pointer for nest using
    hapservice= hap_add_button_service("Button");//,button_callback,(void*)&button_gpio);

#ifdef ENABLE_WIFI_MANAGER   
   startwifimanager();
#else

    WiFi.mode(WIFI_STA);
    WiFi.begin((char*)ssid, (char*)password);
     while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
#endif
    Serial.println(PSTR("WiFi connected"));
    Serial.println(PSTR("IP address: "));
    Serial.println(WiFi.localIP());
    
    hap_init_homekit_server();   

#ifdef ENABLE_WEB_SERVER
String strIp=String(WiFi.localIP()[0]) + String(".") + String(WiFi.localIP()[1]) + String(".") +  String(WiFi.localIP()[2]) + String(".") +  String(WiFi.localIP()[3]);    
#ifdef ESP8266
 if(hap_homekit_is_paired()){
#endif
  Serial.println(PSTR("Setting web server"));
    SETUP_FILEHANDLES
 
     server.begin(); 
     Serial.println(String("Web site http://")+strIp);  
     Serial.println(String("File system http://")+strIp+String("/browse")); 
      Serial.println(String("Update http://")+strIp+String("/update"));     
     isWebserver_started=true;
#ifdef ESP8266
}else
 Serial.println(PSTR("Web server is NOT SET, waiting for pairing"));
#endif

#endif

btn.Create(button_gpio, buttonhandler_callback,true);
}


void loop() {

#ifdef ESP8266
  hap_homekit_loop();
#endif
#ifdef ESP8266
btn.HandleLoop();
#endif

if(isWebserver_started)
    server.handleClient();


}

void notifyHap(uint8_t val){
  if(hapservice){
    Serial.println("notify hap"); 
    //getting PROGRAMMABLE_SWITCH_EVENT characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice,HOMEKIT_CHARACTERISTIC_PROGRAMMABLE_SWITCH_EVENT);
    if(ch){
        
        if(ch->value.int_value!=val){  //wil notify only if different
          ch->value.int_value=val;
          homekit_characteristic_notify(ch,ch->value);
        }
    }
  }
}



#ifdef ENABLE_WIFI_MANAGER
void startwifimanager() {
  WiFiManager wifiManager;



  if (!wifiManager.autoConnect(HOSTNAME, NULL)) {
      ESP.restart();
      delay(1000);
   }
}
#endif
