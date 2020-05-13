
#define ENABLE_OTA  //if OTA need
#define ENABLE_HISTORY
//#define SEND_DATA_TO_THINGSPEAK  //To send data please specify your api_key to Thingspeak

const char* ssid     = "ssid";
const char* password = "pwd";

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



const int identity_led=2;
const int sensorpin=36;
const float factor=14.0;  //to be calibrated with your MQ135

#ifdef SEND_DATA_TO_THINGSPEAK

#include "HTTPSimpleClient.h"
#endif
///HTTPSimpleClient http;
extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage\hapfilestorage.hpp>



homekit_service_t* airqualitylevelservice=NULL;
homekit_characteristic_t*  airqualitylevelcharacteristic=NULL;


#define MEASSURE_NUMBER 5 //number of reads
#define OPERATING_VOLTAGE 5.0
#define RLOAD 10.0
#ifdef ESP32
#define ADC_VALUE_MAX 4095
#endif
#ifdef ESP8266
#define ADC_VALUE_MAX 1024
#endif
#define PAR_1 -0.42
#define PAR_2 1.92
#define PPM_CO2_IN_CLEAR_AIR    397.13


#define MQ_READ_PERIOD_MS 5000
#define SEND_THINGSPEAK_PERIOD_MS 500000

#define HISTORY_TIME_MS 36000
#define  HISTORYCOUNT 30
#ifdef ENABLE_HISTORY
#include "Array.h"
#endif

struct device_data_t{
 // float temp=20.0;
  //float hum=50.0;
 // float pressure=1000.0;
 float ppm=0.0;
  //unsigned long next_read_sensor_ms=0;
  //unsigned long next_send_thingspeak_ms=0;
  unsigned long next_read_mq135_ms=0;
#ifdef ENABLE_HISTORY
  unsigned long next_pushhistory_ms=0;
   CSimpleArray<float> ppmhistory;
#endif
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


/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

  
    init_hap_storage("/pair.dat");
  
    
    pinMode(sensorpin,INPUT);
    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_sensor);
    /// init base properties
    hap_initbase_accessory_service("ES","Yurik72","0","EspHapAir","1.0");
 
    
    // for base accessory registering temperature
    airqualitylevelservice= hap_add_air_quality_service( "AirQuality");
    airqualitylevelcharacteristic= homekit_service_characteristic_by_type(airqualitylevelservice, HOMEKIT_CHARACTERISTIC_CARBON_DIOXIDE_LEVEL);
   
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
  if(server.arg("var") == "ppm")
    server.send(200, FPSTR(TEXT_PLAIN),String(DeviceData.ppm));
 #ifdef ENABLE_HISTORY
  else if(server.arg("var") == "ppmhist")
    server.send(200, FPSTR(TEXT_PLAIN),DeviceData.ppmhistory.toJsonArray(200.0,6000.0));
#endif 
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
 if(DeviceData.next_read_mq135_ms<=millis()){
    readMQ135();
    notify_hap();
    DeviceData.next_read_mq135_ms=millis()+MQ_READ_PERIOD_MS;
 }

#ifdef SEND_DATA_TO_THINGSPEAK
if(DeviceData.next_send_thingspeak_ms<=millis()){
    sendToThingspeak();
    
    DeviceData.next_send_thingspeak_ms=millis()+SEND_THINGSPEAK_PERIOD_MS;
 }
#endif
 #ifdef ENABLE_HISTORY
  if(DeviceData.next_pushhistory_ms<=millis()){
    pushhistory();
   
    DeviceData.next_pushhistory_ms=millis()+HISTORY_TIME_MS;
 }
#endif
#ifdef ESP8266
  hap_homekit_loop();
#endif
 if(isWebserver_started)
    server.handleClient();

}


void notify_hap(){
if(airqualitylevelservice){
    HAP_NOTIFY_CHANGES(float, airqualitylevelcharacteristic, DeviceData.ppm, 0.0)
    homekit_characteristic_t* hc_quality = homekit_service_characteristic_by_type(airqualitylevelservice, HOMEKIT_CHARACTERISTIC_AIR_QUALITY);
    Serial.println("Noify PPM:"+String(DeviceData.ppm));
    if (hc_quality){
      uint8_t quality = air_quality_level(DeviceData.ppm, (uint8_t)(*hc_quality->min_value)+1, (uint8_t)(*hc_quality->max_value));
       Serial.println("Noify level:"+String(quality));

      HAP_NOTIFY_CHANGES(int, hc_quality, quality, 0)
    }
}
}

uint16_t meassure() {
  int accres=0;
  for (int i = 0; i < MEASSURE_NUMBER; i++) {
    accres+= analogRead(sensorpin);
    delay(2);
  }
  return accres / MEASSURE_NUMBER;
}
float getRoInCleanAir() {
  return exp((log(PPM_CO2_IN_CLEAR_AIR) * PAR_1) + PAR_2);
}

float readScaled(float val, float a, float b) {
  float ratio = val / factor;
  return exp((log(ratio) - b) / a);
}

float calculateResistance(int sensorADC) {
  float sensorVoltage = sensorADC * (OPERATING_VOLTAGE / ADC_VALUE_MAX);
  float sensorResistance = (OPERATING_VOLTAGE - sensorVoltage) / sensorVoltage * RLOAD;
  return sensorResistance;
}

float calc_PPM(int val) {
  float res = calculateResistance(val);
  return readScaled(res, PAR_1, PAR_2);
}
#define RANGE_EXCELLENT_LEVEL 500.0
#define RANGE_POOR_LEVEL 2500.0
uint8_t air_quality_level(float ppm,uint8_t min, uint8_t max) {
  if (ppm < RANGE_EXCELLENT_LEVEL)
    return min;
  if (ppm > RANGE_POOR_LEVEL)
    return max;
  return ((int)ppm) / ((RANGE_POOR_LEVEL-RANGE_EXCELLENT_LEVEL)/(float)(max - min));
}

void readMQ135(){

DeviceData.ppm = calc_PPM(meassure());
  Serial.println("PPM:"+String(DeviceData.ppm));
}

#ifdef ENABLE_HISTORY
void pushhistory(){
   if (DeviceData.ppmhistory.GetSize() >= HISTORYCOUNT) {
    DeviceData.ppmhistory.AddWithShiftLeft(DeviceData.ppm);
  }
  else {
    DeviceData.ppmhistory.Add(DeviceData.ppm);
  }
 // Serial.println(DeviceData.ppmhistory.toJsonArray(200.0,6000.0));
}
#endif 
#ifdef SEND_DATA_TO_THINGSPEAK
const char* thing_api_key="YOUR KEY";
void sendToThingspeak(){
   Serial.println("sendToThingspeak start");
    String url="http://api.thingspeak.com";
    url=" https://api.thingspeak.com/update?api_key="+String(thing_api_key);
    HTTPSimpleClient http;
  
    
     url+="&field1="+String(DeviceData.ppm);

     Serial.println(url);
     //nt httpcode=http.POST(poststr);
     if(!http.begin(url)){
       Serial.println("Failed to connect to"+url );
    }
    int httpCode = http.GET();
   Serial.println("http code returns"+String(httpCode) );
    Serial.println("http returns"+http.getString() );
     
}

#endif
