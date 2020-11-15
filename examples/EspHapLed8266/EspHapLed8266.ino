/*
  ESPHap example EspHapLed for ESP8266

  This example for ESPHap shows how to control a LED
  with Apple Home app. It implements accessory type "light bulb".

  This example code is part of the public domain
*/

#include <Arduino.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include "coredecls.h"
const char* ssid     = "ssid";
const char* password = "pwd";

const int identity_led=2;
const int led_gpio = 4;

extern "C"{
#include "homeintegration.h"
}
#include "homekitintegrationcpp.h"

homekit_service_t* hapservice={0};
String pair_file_name="/pair.dat";
 //esp8266::MDNSImplementation::MDNSResponder* MDNS1;   

LOCAL os_timer_t callbacktimer;

void setup() {
  disable_extra4k_at_link_time();
  Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
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

    Serial.println(ssid);
#ifdef ESP8266
  WiFi.mode(WIFI_STA);
    WiFi.begin((char*)ssid, (char*)password);
#else
    WiFi.begin(ssid, password);
#endif
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
   // Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


   pinMode(led_gpio,OUTPUT);
///setup identity gpio
   // hap_set_identity_gpio(identity_led);    //identity_led=2 will blink on identity

/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb, Apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);

    /// init base properties
    hap_initbase_accessory_service("ES","Yurik72","0","EspHapLed","1.0");

   //we will add only one light bulb service and keep pointer for nest using
    hapservice= hap_add_lightbulb_service("Led",led_callback,(void*)&led_gpio);
   
   //and finally init HAP
   
    hap_init_homekit_server();
}

void loop() {
 
  hap_homekit_loop();
  //delay(2);
  return;
}

void init_hap_storage(){
  Serial.print("init_hap_storage");
 
    
  File fsDAT=SPIFFS.open(pair_file_name, "r");
 if(!fsDAT){
   Serial.println("Failed to read pair.dat");
   return;
 }
  int size=hap_get_storage_size_ex();
  char* buf=new char[size];
  memset(buf,0xff,size);
 int readed=fsDAT.readBytes(buf,size);
 // Serial.print("Readed bytes ->");
//  Serial.println(readed);
  hap_init_storage_ex(buf,size);
  fsDAT.close();
  delete []buf;
}

void storage_changed(char * szstorage,int bufsize){

  SPIFFS.remove(pair_file_name);
  File fsDAT=SPIFFS.open(pair_file_name, "w+");
  if(!fsDAT){
    Serial.println("Failed to open pair.dat");
    return;
  }
  fsDAT.write((uint8_t*)szstorage, bufsize);

  fsDAT.close();
}
//can be used for any logic, it will automatically inform Apple about state changes
void set_led(bool val){
   Serial.println("set_led"); 
  digitalWrite(led_gpio, val?HIGH:LOW);
  //we need notify apple about changes
  
  if(hapservice){
    Serial.println("notify hap"); 
    //getting on/off characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
    if(ch){
        Serial.println("found characteristic"); 
        if(ch->value.bool_value!=val){  //wil notify only if different
          ch->value.bool_value=val;
          homekit_characteristic_notify(ch,ch->value);
        }
    }
  }
}

void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("led_callback");
 set_led(ch->value.bool_value);
}
