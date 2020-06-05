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

const int FAN_gpio=4;
const int FAN_speed_gpio=5;
const int FAN_direction_gpio=13;
int channel =0;  // for esp32, we will use channel 0 for PWM

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
#define HOMEKIT_SHORT_APPLE_UUIDS
#endif




homekit_service_t* hapservice={0};

String pair_file_name="/pair.dat";

#define DIM_MIN_VAL 0
#define DIM_MAX_VAL 0xFF
#define DIM_FREQ 5000
#define DIM_RESOLUTION 8

#define DIMCALC_VAL(val,invert) constrain(((!invert)?val:(DIM_MAX_VAL-val)),DIM_MIN_VAL,DIM_MAX_VAL)
#define MAP_100_2_255(val) map(val,0,100,DIM_MIN_VAL,DIM_MAX_VAL)

void FAN_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);
//Web server section
#define ENABLE_OTA  //if OTA need

#include "spiffs_webserver.h"


bool getFANState(){
  if(hapservice){
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        return ch->value.bool_value;
      }
    }
    return false;
}
bool getFANDirection(){
  if(hapservice){
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_DIRECTION);
      if(ch){
        return ch->value.int_value;
      }
    }
    return 0;
}
float getFANSpeed(){
  if(hapservice){
      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_SPEED);
      if(ch){
        return ch->value.float_value;
      }
    }
    return 0.0;
}

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


    pinMode(FAN_gpio, OUTPUT);
    pinMode(FAN_direction_gpio, OUTPUT);
#ifdef ESP32    
    ledcSetup(channel, DIM_FREQ, DIM_RESOLUTION);
    ledcAttachPin(FAN_speed_gpio, channel);
#endif
    pinMode(FAN_speed_gpio, OUTPUT);

    
    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is FAN , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_fan);
    /// init base properties
    hap_initbase_accessory_service("host","Yurik72","0","EspHapFAN","1.0");

   //we will add only one FAN service and keep pointer for nest using
    hapservice= hap_add_fan_service("FAN",FAN_callback,(void*)&FAN_gpio);

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
     server.on("/get", handleGetVal);
      server.on("/set", handleSetVal);   
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
}

void handleGetVal(){
   
  if(server.arg("var") == "ch1")
    server.send(200, FPSTR(TEXT_PLAIN),String(getFANState()));
  else if(server.arg("var") == "ch2")
     server.send(200, FPSTR(TEXT_PLAIN),String(getFANDirection()));
  else if(server.arg("var") == "speed")
     server.send(200, FPSTR(TEXT_PLAIN),String(getFANSpeed()));
  else
    server.send(505, FPSTR(TEXT_PLAIN),"Bad args");  
}
void handleSetVal(){
  if (server.args() !=2){
    server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
    return;
  }
  bool isOk=false;
  //to do analyze
  if(server.arg("var") == "ch1"){
    if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        set_FANState(server.arg("val")=="true");
        isOk=true;
      }
    }
  }
   if(server.arg("var") == "ch2"){
    if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_DIRECTION);
      if(ch){
        set_FANDirection(server.arg("val")=="true"?1:0);
         isOk=true;
      }
    }
  }
  if(server.arg("var") == "speed"){
    if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_SPEED);
      if(ch){
        set_FANSpeed(String(server.arg("val")).toFloat());
         isOk=true;
      }
    }
  }
if( isOk)
    server.send(200, FPSTR(TEXT_PLAIN), "OK");
  else
    server.send(505, FPSTR(TEXT_PLAIN), "WRONG");
}
void loop() {

#ifdef ESP8266
  hap_homekit_loop();
#endif
  
if(isWebserver_started)
    server.handleClient();


}

void init_hap_storage(){
  Serial.print("init_hap_storage");
 
    
  File fsDAT=SPIFFS.open(pair_file_name, "r");
  
 if(!fsDAT){
   Serial.println("Failed to read pair.dat");
   SPIFFS.format(); 
   
 }
  int size=hap_get_storage_size_ex();
  char* buf=new char[size];
  memset(buf,0xff,size);
  if(fsDAT)
  fsDAT.readBytes(buf,size);
 
  hap_init_storage_ex(buf,size);
  if(fsDAT)
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

void set_FANState(bool val){
  Serial.println(String("set_FANState:")+String(val?"True":"False")); 
  digitalWrite(FAN_gpio, val?HIGH:LOW);
  //we need notify apple about changes
  if(hapservice){
    Serial.println("notify fan state"); 
    //getting on/off characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
    HAP_NOTIFY_CHANGES(bool, ch, val, 0)
  }
}
void set_FANDirection(uint8_t val){
  // to do adjust your code to device to handle direction
  digitalWrite(FAN_direction_gpio, val?1:0);
  if(hapservice){
    Serial.println("notify fan direction:"+String(val)); 
    //getting on/off characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_DIRECTION);
    HAP_NOTIFY_CHANGES(int, ch, val, 0)
  }
}

void set_FANSpeed(float val){
  #ifdef ESP8266
    analogWrite(FAN_speed_gpio, DIMCALC_VAL(MAP_100_2_255(val), false));
  #endif
  #ifdef ESP32
  ledcWrite(channel, DIMCALC_VAL(MAP_100_2_255(val), false));
  #endif
  if(hapservice){
    Serial.println("notify fan speed:"+String(val)); 
    //getting on/off characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ROTATION_SPEED);
    HAP_NOTIFY_CHANGES(float, ch, val, 0)
  }  
}

void FAN_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("FAN_callback");
    if(!ch)
      return;
    //Serial.println("got callback charachteristic "+String(ch->type));
    
    if(strcmp(ch->type,HOMEKIT_CHARACTERISTIC_ON)==0)
      set_FANState(ch->value.bool_value);
    else if(strcmp(ch->type,HOMEKIT_CHARACTERISTIC_ROTATION_DIRECTION)==0)
      set_FANDirection(ch->value.int_value);
    else if(strcmp(ch->type,HOMEKIT_CHARACTERISTIC_ROTATION_SPEED)==0)
      set_FANSpeed(ch->value.float_value);
    else
      Serial.println(" unknown charactheristic");
      

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
