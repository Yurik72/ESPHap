
////  !!!! This sketch is not finished yet
///   It just shows how to add/integrate garage door charachteristic with apple
///   garage door functionality should be reviewed and properly implemented




#include <Arduino.h>
#include "simplesensor.h"

#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager




const char* HOSTNAME="EspGarageDoor";
const int identity_led=2;


extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage\hapfilestorage.hpp>




homekit_service_t* service_garagedoor=NULL;
const int relay_gpio = 4;
const int sensor_gpio = 5;
SimpleSensor Sensor(sensor_gpio);
void sensor_callback(uint8_t gpio_num, uint8_t state)
{
  current_door_state_update_from_sensor();
}

uint8_t current_door_state = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN;
uint8_t target_door_state = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN;

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
      // Serial.print("SPIFFS Mount failed");
     }
#endif
#ifdef ESP8266
     if (!SPIFFS.begin()) {
      Serial.print("SPIFFS Mount failed");
     }
#endif

 pinMode(relay_gpio,OUTPUT);
 Sensor.start(sensor_callback);
 startwifimanager();




/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

  
    init_hap_storage("/pair.dat");
  
  

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_door);
    /// init base properties
    hap_initbase_accessory_service("ES","Yurik72","0","EspHapLed","1.0");
 
    
    // for base accessory registering temperature
    service_garagedoor = hap_add_garagedoor_service("garagedoor",hap_callback_process,0);
   
   
   
  hap_init_homekit_server();
  
     
}
void loop() {
 
#ifdef ESP8266
  hap_homekit_loop();
#endif
}

void notify_hap(){

 if(service_garagedoor){
    homekit_characteristic_t * ch_currentstate= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE);
     homekit_characteristic_t * ch_targetstate= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE);
     homekit_characteristic_t * ch_obstruction= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_OBSTRUCTION_DETECTED);

 }

}
void relay_write(bool on) {
    digitalWrite(relay_gpio, on?HIGH:LOW);
}

void current_state_set(uint8_t new_state) {
    homekit_characteristic_t * ch_currentstate= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE);
    if (ch_currentstate && current_door_state != new_state) {
        current_door_state = new_state;
        HAP_NOTIFY_CHANGES(int, ch_currentstate, new_state, 0)
    }
}

void current_door_state_update_from_sensor() {
    uint8_t state=Sensor.getstate();

    switch (state) {
        case HIGH:
      //target_door_state = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN;
      if (current_door_state != HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN) {
              current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING); // NOP, if already set
              current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN);
      }
            break;
      case LOW:
            //target_door_state = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED;
      if (current_door_state != HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED) {
              current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING); // NOP, if already set
              current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED);
      }
            break;
        default:
            Serial.println("Unknown contact sensor event");
    }
}
void hap_callback_process(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("hap_callback");
    if(!service_garagedoor){
       Serial.println("service not defined");
      return;
   
    }
     homekit_characteristic_t * ch_currentstate= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE);
     homekit_characteristic_t * ch_targetstate= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE);
     homekit_characteristic_t * ch_obstruction= homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_OBSTRUCTION_DETECTED);

     if(ch==ch_targetstate){
        Serial.println("processing target state");
         if (current_door_state != HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN &&
             current_door_state != HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED) {
            Serial.println("target_state_set  ignored: current state not open or closed.");
            return;
        }
            //target_door_state = new_value.int_value;
        if (current_door_state == target_door_state) {
            Serial.println("target_state_set ignored: target state == current state");
            return;
        }
         relay_write(true);
        // Wait for some time:
        delay(1000); ///????
        // Turn OFF GPIO:
        relay_write(false);
        if (current_door_state == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN) {
            current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING);
        } else {
            current_state_set(HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING);
        }
    }
     

}
