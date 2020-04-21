
//#include <ESP8266WiFi.h>
//#define LWIP_TCP 1
#include <Arduino.h>



//#include <FS.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>

#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

#include "coredecls.h"
#include "Button2.h"
#include "Clight.h"
Clight lamp;
const int led_gpio = 13;
const int relay_gpio = 12;

const char* HOSTNAME="ES";
const char* ssid     = "ssid";
const char* password = "pwd";

extern "C"{
#include "homeintegration.h"
}
#include "homekitintegrationcpp.h"

homekit_service_t* hapservice={0};
homekit_characteristic_t * hap_on={0};
homekit_characteristic_t * hap_br={0};
homekit_characteristic_t * hap_hue={0};
homekit_characteristic_t * hap_saturation={0};
String pair_file_name="/pair.dat";



void lamp_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);
//Web server section
#define ENABLE_OTA  //if OTA need
#include <ESP8266WebServer.h>
 ESP8266WebServer server(80);
#include "spiffs_webserver.h"
bool isWebserver_started=false;

bool getRelayVal(){
  if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        return ch->value.bool_value;
      }
    }
    return false;
}

void setup() {
  disable_extra4k_at_link_time();  //let's maximize as much as possible stack size for crypto libraries
  Serial.begin(115200);
  

    // We start by connecting to a WiFi network
  lamp.show();
     if (!SPIFFS.begin()) {
      Serial.print("SPIFFS Mount failed");
      
     }






  
///setup identity gpio
   


    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());


    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);
    /// init base properties
    hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");
   //we will add only one light bulb service and keep pointer for nest using
    

  hapservice=hap_add_rgbstrip_service("Lamp",lamp_callback,NULL);
  hap_on=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);;
  hap_br=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_BRIGHTNESS);;
  hap_hue=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_HUE);;
  hap_saturation=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_SATURATION);;
   
   startwifimanager();
   /*
    WiFi.mode(WIFI_STA);
    WiFi.begin((char*)ssid, (char*)password);
     while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    */
    Serial.println(PSTR("WiFi connected"));
    Serial.println(PSTR("IP address: "));
    Serial.println(WiFi.localIP());
    
    hap_init_homekit_server();   

 if(hap_homekit_is_paired()){
  Serial.println(PSTR("Setting web server"));
    SETUP_FILEHANDLES
     server.on("/get", handleGetVal);
      server.on("/set", handleSetVal);   
     server.begin(); 
     isWebserver_started=true;
}else
 Serial.println(PSTR("Web server is NOT SET, waiting for pairing"));
}
void handleGetVal(){
    server.send(200, FPSTR(TEXT_PLAIN), lamp.IsOn ?"1":"0");
}
void handleSetVal(){
  if (server.args() !=2){
    server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
    return;
  }
  //to do analyze
  if(server.arg("var") == "ch1"){
 
		  lamp.IsOn=(server.arg("val")=="true");
		  lamp.show();
    }
     else  if(server.arg("var") == "br"){

      lamp.Brigthness =server.arg("val").toInt();
      lamp.show();
      }
    else  if(server.arg("var") == "col"){
      uint32_t color=server.arg("val").toInt();
      double Hue;
      double Saturation;
      double Intensity;
      Clight::ColorToHSI(color, lamp.Brigthness*255/100,Hue,Saturation,Intensity);
      lamp.Saturation=Saturation;
      lamp.Hue=Hue;
      lamp.show();
      }
     
}
void loop() {

  hap_homekit_loop();
  
  if(isWebserver_started)
    server.handleClient();
return;
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
void set_relay(bool val){
   Serial.println("set_led"); 
  digitalWrite(relay_gpio, val?HIGH:LOW);
  //we need notify apple about changes
  
  if(hapservice){
    Serial.println("notify hap"); 
    //getting on/off characteristic
    homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
    if(ch){
       
        if(ch->value.bool_value!=val){  //wil notify only if different
          ch->value.bool_value=val;
          homekit_characteristic_notify(ch,ch->value);
        }
    }
  }
}

void lamp_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("lamp_callback");
     bool isSet=false;
    if(ch==hap_on && ch->value.bool_value!=lamp.IsOn){
      lamp.IsOn=ch->value.bool_value;
	  lamp.Brigthness = 0.0;
      isSet=true;
    }
    if(ch==hap_br && ch->value.int_value!=lamp.Brigthness && lamp.IsOn){
      lamp.Brigthness=ch->value.int_value;
      isSet=true;
    }
    if(ch==hap_hue && ch->value.float_value!=lamp.Hue){
      lamp.Hue=ch->value.float_value;

      isSet=true;
    }
    if(ch==hap_saturation && ch->value.float_value!=lamp.Saturation){
      lamp.Saturation=ch->value.float_value;
      isSet=true;
    }
    if(isSet)
      lamp.show();
}
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("WiFi On Disconnect.");
  
}

void onWifiConnect(const WiFiEventStationModeConnected& event) {
  Serial.println("WiFi On Connect.");
  
}
void startwifimanager() {
  WiFiManager wifiManager;

  WiFi.onStationModeDisconnected(onWifiDisconnect);
  WiFi.onStationModeConnected(onWifiConnect);

  if (!wifiManager.autoConnect(HOSTNAME, NULL)) {
      ESP.restart();
      delay(1000);
   }
}
