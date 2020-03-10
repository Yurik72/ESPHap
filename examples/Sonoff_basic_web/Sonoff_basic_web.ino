
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

const int led_gpio = 13;
const int relay_gpio = 12;
const int button_gpio = 0;
const char* HOSTNAME="ES";
const char* ssid     = "ssid";
const char* password = "pwd";

extern "C"{
#include "homeintegration.h"
}
#include "homekitintegrationcpp.h"

homekit_service_t* hapservice={0};
String pair_file_name="/pair.dat";


Button2 button_switch = Button2(button_gpio);

//Web server section
#include <ESP8266WebServer.h>
 ESP8266WebServer server(80);
#include "spiffs_webserver.h"
bool isWebserver_started=false;
void click(Button2& btn) {
    Serial.println("click");
     if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        set_relay(!ch->value.bool_value);
      }
    }
}
bool getRelayVal(){
  if(hapservice){

      homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
      if(ch){
        return ch->value.bool_value;
      }
    }
    return false;
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
void setup() {
  disable_extra4k_at_link_time();  //let's maximize as much as possible stack size for crypto libraries
  Serial.begin(115200);
  
 // pinMode(led_gpio,OUTPUT);
  pinMode(relay_gpio,OUTPUT);
  digitalWrite(relay_gpio,HIGH);  //immediatelly  switch to be used as normal switch
    // We start by connecting to a WiFi network

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
    hapservice= hap_add_lightbulb_service("Son",relay_callback,(void*)&relay_gpio);

   
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
    button_switch.setClickHandler(click);
    button_switch.setLongClickHandler(longpress);
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
    server.send(200, FPSTR(TEXT_PLAIN), getRelayVal()?"1":"0");
}
void handleSetVal(){
  if (server.args() !=2){
    server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
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
void loop() {

  hap_homekit_loop();
  button_switch.loop();
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

void relay_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("led_callback");
 set_relay(ch->value.bool_value);
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
