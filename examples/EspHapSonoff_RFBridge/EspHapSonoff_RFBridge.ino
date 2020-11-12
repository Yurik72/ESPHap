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
#include "Button2.h"
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

#define MOTION_AUTORESET_PERIOD 5000

bool isWebserver_started=false;

const int led_gpio = 13;
const int button_gpio = 0;
const int relay_gpio = 12;

const char* HOSTNAME="EspLed";
homekit_service_t* hapservice={0};
homekit_service_t* hapservice_motion={0};

Button2 button_switch = Button2(button_gpio);
bool isMotionDetected=false;
unsigned long nextMotionResetCheck_ms=0;

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
    pinMode(led_gpio,OUTPUT);
  pinMode(relay_gpio,OUTPUT);
   digitalWrite(relay_gpio,HIGH);  //immediatelly  switch to be used as normal switch

  // We start by connecting to a WiFi network
#ifdef ESP32
  if (!SPIFFS.begin(true)) {
#else if ESP8266 
  if (!SPIFFS.begin()) {
#endif
       Serial.print("SPIFFS Mount failed");
  }
 
    


     startwifimanager();
   ///setup identity gpio
   hap_set_identity_gpio(led_gpio);    //identity_led=2 will blink on identity

/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    init_hap_storage("/pair.dat");


    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
    
    // Setup ID in format "XXXX" (where X is digit or latin capital letter)
    // Used for pairing using QR code
    hap_set_device_setupId((char*)"YK72");
    /// init base properties
    hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","SonoffMBr","1.0");

   //we will add only one light bulb service and keep pointer for nest using
    hapservice= hap_add_lightbulb_service("Led",relay_callback,(void*)&relay_gpio);

    hapservice_motion= hap_add_motion_service_as_accessory(homekit_accessory_category_security_system,"Motion",motion_callback,NULL);

   //and finally init HAP
    hap_init_homekit_server();
  
   
    set_indexhml(FPSTR(INDEX_HTML));
    hap_webserver_begin();
      
   server.on("/get", handleGetVal);
  server.on("/set", handleSetVal);

    isWebserver_started=true;
        button_switch.setClickHandler(click);
    button_switch.setLongClickHandler(longpress);
}

void loop() {
#ifdef ESP8266
  hap_homekit_loop();
#endif
 button_switch.loop();
  if(isWebserver_started){
     hap_webserver_loop();
  }
  if(isMotionDetected && nextMotionResetCheck_ms<=millis()){
    Serial.println("AutoReset Motion");
    notifyMotion(false);
  }
}
void click(Button2& btn) {
    Serial.println("button click (get RF signal)");
     if(hapservice_motion){
      notifyMotion(true);

    }

}
void longpress(Button2& btn) {
    unsigned int time = btn.wasPressedFor();
    Serial.print("You clicked ");
    if (time > 1500) {
        Serial.print("a really really long time.");
            //TO DO Reset
        //  SPIFFS.remove(pair_file_name);  //clean pairing data
         // WiFi.disconnect(true);   //lost saved wifi credentials
    } else {
        Serial.print("long.");        
    }
    Serial.print(" (");        
    Serial.print(time);        
    Serial.println(" ms)");
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
        set_relay(server.arg("val")=="true");
      }
    }
  }
}
//can be used for any logic, it will automatically inform Apple about state changes
void set_relay(bool val){
  Serial.println("set_led"); 
  digitalWrite(relay_gpio, val?HIGH:LOW);
  //we need notify apple about changes
  if(hapservice){
      Serial.println("notify hap"); 
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      HAP_NOTIFY_CHANGES(bool, ch, val, 0)
  }
}

void relay_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
  Serial.println("led_callback");
  set_relay(ch->value.bool_value);
}
void motion_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
//nothing to do
}
void notifyMotion(bool bval){
  homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice_motion, HOMEKIT_CHARACTERISTIC_MOTION_DETECTED);
    if(ch){
      if(ch->value.bool_value!=bval){
        Serial.println("Notify Motion");
        ch->value.bool_value=bval;
        homekit_characteristic_notify(ch,ch->value);
      }
    }
    isMotionDetected=bval;
    if(bval) {
      nextMotionResetCheck_ms=millis()+MOTION_AUTORESET_PERIOD;
      digitalWrite(led_gpio, HIGH);
    }
    else{
      nextMotionResetCheck_ms=0;
      digitalWrite(led_gpio, LOW);
    }
    
}
