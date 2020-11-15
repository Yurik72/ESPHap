#include <Arduino.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif

//Webserver
#define ENABLE_OTA  //if OTA need
#ifdef ESP8266
#include <ESP8266WebServer.h>
 ESP8266WebServer server(80);

#endif
#ifdef ESP32
#include <WebServer.h>
WebServer server(80);
#endif
#if defined(ESP32) && defined(ENABLE_OTA)
#include <Update.h>
#endif

#include "spiffs_webserver.h"
bool isWebserver_started=false;

#include "DHT.h"   //https://github.com/adafruit/DHT-sensor-library
#define DHT11_PIN 4

DHT DHT(DHT11_PIN,DHT11);

const char* ssid     = "ssid";
const char* password = "pwd";

const int identity_led=2;

extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif

String pair_file_name="/pair.dat";

homekit_service_t* temperature=NULL;
homekit_service_t* humidity=NULL;  

#define DHT_READ_PERIOD_MS 5000

struct device_data_t{
  float temp=20.0;
  float hum=50.0;
  unsigned long next_read_dht_ms=0;
};

device_data_t DeviceData;

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

DHT.begin();
 
/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_thermostat);
    /// init base properties
    hap_initbase_accessory_service("ES","Yurik72","0","EspHapLed","1.0");
    // for base accessory registering temperature
    temperature = hap_add_temperature_service("Temperature");
    // Adding second accessory for humidity
    humidity = hap_add_hum_as_accessory(homekit_accessory_category_thermostat ,"Humidity");
   
   //and finally init HAP
   
hap_init_homekit_server();

String strIp=String(WiFi.localIP()[0]) + String(".") + String(WiFi.localIP()[1]) + String(".") +  String(WiFi.localIP()[2]) + String(".") +  String(WiFi.localIP()[3]); 

//setup web server
#ifdef ESP8266      
   if(hap_homekit_is_paired()){
#endif
     delay(500);
      Serial.println("Setting web server");      
      SETUP_FILEHANDLES
      server.on("/get", handleGetVal);
      server.on("/set", handleSetVal);   
     server.begin(); 
     Serial.println(String("Web site http://")+strIp);  
     Serial.println(String("File system http://")+strIp+String("/browse")); 
      Serial.println(String("Update http://")+strIp+String("/update"));     
     isWebserver_started=true;
#ifdef ESP8266     
  }else{
      Serial.println("Web server is NOT SET, waiting for pairing");
  }
#endif   
}

void handleGetVal(){
  if(server.arg("var") == "temp")
    server.send(200, FPSTR(TEXT_PLAIN),String(DeviceData.temp));
  else if(server.arg("var") == "hum")
     server.send(200, FPSTR(TEXT_PLAIN),String(DeviceData.hum));
  else
    server.send(505, FPSTR(TEXT_PLAIN),"Bad args");      
}

void handleSetVal(){
  if (server.args() !=2){
    server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
    return;
  }
  //to do analyze
  if(server.arg("var") == "ch1"){
  }     
}

void loop() {
 if(DeviceData.next_read_dht_ms<=millis()){
    readDHT();
    notify_hap();
    DeviceData.next_read_dht_ms=millis()+DHT_READ_PERIOD_MS;
 }

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

void notify_hap(){
 if(temperature){
  homekit_characteristic_t * ch_temp= homekit_service_characteristic_by_type(temperature, HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
  if(ch_temp && !isnan(DeviceData.temp) &&  ch_temp->value.float_value!=DeviceData.temp ){
    ch_temp->value.float_value=DeviceData.temp;
    homekit_characteristic_notify(ch_temp,ch_temp->value);
  }
 }
if(humidity){
  homekit_characteristic_t * ch_hum= homekit_service_characteristic_by_type(humidity, HOMEKIT_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY);
  if(ch_hum && !isnan(DeviceData.hum) && ch_hum->value.float_value!=DeviceData.hum){
    ch_hum->value.float_value=DeviceData.hum;
    homekit_characteristic_notify(ch_hum,ch_hum->value);
  }
}
}

void readDHT(){
  DeviceData.temp= DHT.readTemperature();
  DeviceData.hum = DHT.readHumidity();
  Serial.println(String("Temp")+String(DeviceData.temp)+String("  Hum:")+String(DeviceData.hum));
  if(isnan(DeviceData.temp)){
    Serial.println("Set default temp 20");
    DeviceData.temp=20.0;
  }
    if(isnan(DeviceData.hum)){
      Serial.println("Set default hum 50");
     DeviceData.hum=50.0;
  }
}
