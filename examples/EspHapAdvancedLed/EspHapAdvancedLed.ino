#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif
extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage/hapfilestorage.hpp>
#include <hapweb/hap_webserver.hpp>

#include "file_index_html.h"

#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager



bool isWebserver_started=false;
const int identity_led=2;
const int led_gpio = 4;

const char* HOSTNAME="EspLed";
homekit_service_t* hapservice={0};

void startwifimanager() {
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(HOSTNAME, NULL)) {
      ESP.restart();
      delay(1000);
   }
}

void setup() {
#ifdef ESP8266 
  disable_extra4k_at_link_time();
#endif 
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
#ifdef ESP32
  if (!SPIFFS.begin(true)) {
#else if ESP8266 
  if (!SPIFFS.begin()) {
#endif
       Serial.print("SPIFFS Mount failed");
  }
 
   pinMode(led_gpio,OUTPUT);

     startwifimanager();
   ///setup identity gpio
   hap_set_identity_gpio(identity_led);    //identity_led=2 will blink on identity

/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    init_hap_storage("/pair.dat");


    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
    
    // Setup ID in format "XXXX" (where X is digit or latin capital letter)
    // Used for pairing using QR code
    hap_set_device_setupId("YK72");
    /// init base properties
    hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");

   //we will add only one light bulb service and keep pointer for nest using
    hapservice= hap_add_lightbulb_service("Led",led_callback,(void*)&led_gpio);

   //and finally init HAP
    hap_init_homekit_server();
  
   
    set_indexhml(FPSTR(INDEX_HTML));
    hap_webserver_begin();
      
   server.on("/get", handleGetVal);
  server.on("/set", handleSetVal);

    isWebserver_started=true;
}

void loop() {
#ifdef ESP8266
  hap_homekit_loop();
#endif
  if(isWebserver_started){
     hap_webserver_loop();
  }

}

bool getVal(){
  if(hapservice){
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        return ch->value.bool_value;
      }
    }
    return false;
}
void handleGetVal(){
    server.send(200, "text/plain", getVal()?"1":"0");
}
void handleSetVal(){
  if (server.args() !=2){
    server.send(505,"text/plain", "Bad args");
    return;
  }
  //to do analyze
  if(server.arg("var") == "ch1"){
    if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        set_led(server.arg("val")=="true");
      }
    }
  }
}
//can be used for any logic, it will automatically inform Apple about state changes
void set_led(bool val){
  Serial.println("set_led"); 
  digitalWrite(led_gpio, val?HIGH:LOW);
  //we need notify apple about changes
  if(hapservice){
      Serial.println("notify hap"); 
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      HAP_NOTIFY_CHANGES(bool, ch, val, 0)
  }
}

void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
  Serial.println("led_callback");
  set_led(ch->value.bool_value);
}
