
#define ENABLE_OTA  //if OTA need
#define SENSOR_TYPE_DHT  
//#define SENSOR_TYPE_BME280
//#define SENSOR_TYPE_DALLAS 
//#define SEND_DATA_TO_THINGSPEAK  //To send data please specify your api_key to Thingspeak


#define SENSOR_READ_PERIOD_MS 5000
#define SEND_THINGSPEAK_PERIOD_MS 500000
#define PERIOD_HISTORY_MS 10000
#define MAX_HISTORY_RECORDS 20
#define JSON_BUFFER_SIZE_ELEMENT 500


#include <Arduino.h>
#include <ArduinoJson.h>
#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif


#if defined(ESP32) && defined(ENABLE_OTA)
#include <Update.h>
#endif

#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include "ESPTime.h"

bool isWebserver_started=false;

#include "CHistory.h"
#ifdef SENSOR_TYPE_DHT
#include "DHT.h"   //https://github.com/adafruit/DHT-sensor-library
#define DHT11_PIN 4
DHT DHT(DHT11_PIN,DHT11);
#endif

#ifdef SENSOR_TYPE_BME280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>  //https://github.com/adafruit/Adafruit_BME280_Library
#define BME_ADDR 0x76
Adafruit_BME280 BME;  //I2C
#endif 


#ifdef SENSOR_TYPE_DALLAS
#include <OneWire.h>
#include <DallasTemperature.h>  //https://github.com/milesburton/Arduino-Temperature-Control-Library
#define DALLAS_PIN  18
OneWire  OW(DALLAS_PIN);
DallasTemperature DALLAS(&OW);
#endif



const int identity_led=2;

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
#include <hapweb\hap_webserver.hpp>
#include "file_index_html.h"

#define _DEBUG


#define INFO(message, ...) Serial.printf(">>> Thermo: " message "\n", ##__VA_ARGS__)
#define ERROR(message, ...) Serial.printf("!!! Thermo: " message "\n", ##__VA_ARGS__)

#ifdef _DEBUG
#define DEBUG(message, ...) Serial.printf_P(PSTR(">>> %s: " message "\n"), __func__, ##__VA_ARGS__)
#else

#define DEBUG(message, ...)

#endif
homekit_service_t* temperature=NULL;
homekit_service_t* humidity=NULL;  
homekit_service_t* elgato_service=NULL;

struct device_data_t{
  float temp=20.0;
  float hum=50.0;
  float pressure=1000.0;
  unsigned long next_read_sensor_ms=0;
  unsigned long next_send_thingspeak_ms=0;
   unsigned long next_history_save_ms=0;
};
device_data_t DeviceData;

// History handling
struct device_history{
  device_history(){};
  device_history(device_data_t data){
    this->temp=data.temp;
    this->hum=data.hum;
    this->pressure=data.pressure;
  };
  float temp=20.0;
  float hum=50.0;
  float pressure=1000.0;
  long time_ms=0;
};
String historyFileName="/history.json";
CHistory<device_history> DeviceHistory;

String GetStateStringForHistory(device_history state) {
  DynamicJsonDocument jsonBuffer(JSON_BUFFER_SIZE_ELEMENT);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["t"] = state.temp;
  root["h"] = state.hum;
  root["p"] = state.pressure;
  root["tm"] = state.time_ms;
  String json;

  serializeJson(root, json);
  return json;
}
device_history ExtractElement (String json){
  INFO("Extract call");
  device_history rec;
  DynamicJsonDocument jsonBuffer(JSON_BUFFER_SIZE_ELEMENT);
  DeserializationError error = deserializeJson(jsonBuffer, json);
  
  if (error) {
  
    ERROR("Deserialize element: %s",error.c_str());
    return rec;
  }
  
  JsonObject root = jsonBuffer.as<JsonObject>();
  rec.temp = root["t"];
  rec.hum = root["h"];
  rec.pressure = root["p"];
  rec.time_ms = root["tm"];
  
  return rec;
}
void startwifimanager() {
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("Thermo", NULL)) {
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
     }
#endif
#ifdef ESP8266
     if (!SPIFFS.begin()) {
      ERROR("SPIFFS Mount failed");
     }
#endif


    startwifimanager();
    INFO("WiFi connected");
    INFO("IP address: %s",WiFi.localIP().toString().c_str());

    //Configure & start time from udp
    startTime();
#ifdef SENSOR_TYPE_DHT
DHT.begin();
#endif
#ifdef SENSOR_TYPE_BME280
 if(!BME.begin(BME_ADDR))
   ERROR("Failed to Init BME280: ");
#endif
#ifdef SENSOR_TYPE_DALLAS

#endif
/// setup history
    
    DeviceHistory.SetGetStringFn(GetStateStringForHistory);
    DeviceHistory.SetExtractFn(ExtractElement);
    DeviceHistory.LoadFromFile(historyFileName);
/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    INFO("Free heap: %d",system_get_free_heap_size());
    
  
    init_hap_storage("/pair.dat");
  
    
    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_thermostat);
    /// init base properties
    hap_initbase_accessory_service("Thermo","Yurik72","0","EspThermo","1.0");
 
    
    // for base accessory registering temperature
    temperature = hap_add_temperature_service("Temperature");
    // Adding second accessory for humidity
    humidity=hap_add_hum_as_accessory(homekit_accessory_category_thermostat ,"Humidity");

   //elgato_service=hap_add_elgatosupport_service("TestElgato",hap_callback_process,0);
   
   //and finally init HAP
   
   
hap_init_homekit_server();
String strIp=String(WiFi.localIP()[0]) + String(".") + String(WiFi.localIP()[1]) + String(".") +  String(WiFi.localIP()[2]) + String(".") +  String(WiFi.localIP()[3]); 
 //setup web server
#ifdef ESP8266      
   if(hap_homekit_is_paired()){
#endif
     delay(500);
     INFO("Setting web server");      
     SETUP_FILEHANDLES
     set_indexhml(FPSTR(INDEX_HTML));
     hap_webserver_begin();


     server.on("/get", handleGetVal);
     server.on("/set", handleSetVal);   
     server.begin(); 
     Serial.println(String("Web site http://")+strIp);  
     Serial.println(String("File system http://")+strIp+String("/browse")); 
     Serial.println(String("Update http://")+strIp+String("/update"));     
     isWebserver_started=true;
#ifdef ESP8266     
  }else{
      INFO("Web server is NOT SET, waiting for pairing");
  }
#endif   
}

void hap_callback_process(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    INFO("hap_callback -%s",ch->type);
    
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
 if(DeviceData.next_read_sensor_ms<=millis()){
    readSensor();
    notify_hap();
    DeviceData.next_read_sensor_ms=millis()+SENSOR_READ_PERIOD_MS;
 }
 if(DeviceData.next_history_save_ms<=millis()){
    saveHistory();
    
    DeviceData.next_history_save_ms=millis()+PERIOD_HISTORY_MS;
 }

#ifdef SEND_DATA_TO_THINGSPEAK
if(DeviceData.next_send_thingspeak_ms<=millis()){
    sendToThingspeak();
    
    DeviceData.next_send_thingspeak_ms=millis()+SEND_THINGSPEAK_PERIOD_MS;
 }
#endif
 
#ifdef ESP8266
  hap_homekit_loop();
#endif
 if(isWebserver_started)
    server.handleClient();

}
#define ELGATO_SIGNATURE_LENGTH    3     // number of 16 bits word of the following 
void getSignature(uint8_t* signature){

    // 0102 0202 0302
  //  |   |    +-> Pressure 
  //  |   +-> Humidity
  //  +-> Temp
  // 
  // bitmask 0x07 => all      = 111
  // bitmask 0x01 => temp     = 001
  // bitmask 0x02 => hum      = 010
  // bitmask 0x04 => pressure   = 100


}

void update_elgqato_history(device_history& rec ){
  if(!elgato_service){
    ERROR("elgato_service not defined");
    return;
  }
  homekit_characteristic_t * elgato_history= homekit_service_characteristic_by_type(elgato_service, HOMEKIT_CHARACTERISTIC_ELGATO_HISTORY_STATUS);
  if(!elgato_history){
    ERROR("elgato_history not defined");
    return;
  }
  INFO("Update elgato history");
  int sigLength = ELGATO_SIGNATURE_LENGTH * 2;    
  uint8_t signature[sigLength];        
  getSignature(signature);
  uint8_t data[33];        
  memset(&data,0,33);
  //Weather (to be checked): 3 16bits word, "0102 0202 0302", example of full data "01010000 FF000000 3C0F0000 03 0102 0202 0302 1D00 F50F 00000000 00000000 01FF"
  //"01010000 FF000000 3C0F0000 03010202 0203021D 00F50F00 00000000 00000001 FF"
  data[0]=0x01;data[1]=0x01;data[2]=0x00;data[3]=0x00;
  data[4]=0xFF;data[5]=0x00;data[6]=0x00;data[7]=0x00;
  data[8]=0x3C;data[9]=0x0F;data[10]=0x00;data[11]=0x00;
  data[12]=0x03;data[13]=0x01;data[14]=0x02;data[15]=0x02;
  data[16]=0x02;data[17]=0x03;data[18]=0x02;data[19]=0x1D;
  data[20]=0x00;data[21]=0xF5;data[22]=0x0F;data[23]=0x00;
  data[24]=0x00;data[25]=0x00;data[26]=0x00;data[27]=0x00;
  data[28]=0x00;data[29]=0x00;data[30]=0x00;data[31]=0x01;
  data[32]=0xFF;
  elgato_history->value.data_size=33;
  elgato_history->value.data_value=data;
   homekit_characteristic_notify(elgato_history,elgato_history->value);
}

void saveHistory(){
  INFO("SAVEHISTORY");
  device_history rec(DeviceData);
  rec.time_ms= getLocalTime_ms();
  if (DeviceHistory.GetSize() >= MAX_HISTORY_RECORDS) {
    DeviceHistory.AddWithShiftLeft(rec);
  }
  else {
    DeviceHistory.Add(rec);
  }
  update_elgqato_history(rec);
  DeviceHistory.WriteToFile(historyFileName);
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
void readSensor(){

#ifdef SENSOR_TYPE_DHT
  DeviceData.temp= DHT.readTemperature();
  DeviceData.hum = DHT.readHumidity();
#endif
#ifdef  SENSOR_TYPE_BME280
 DeviceData.temp=BME.readTemperature();
 DeviceData.hum=BME.readHumidity();
 DeviceData.pressure=BME.readPressure();
#endif 
#ifdef SENSOR_TYPE_DALLAS
DALLAS.requestTemperatures(); // Send the command to get temperatures

  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  DeviceData.temp = DALLAS.getTempCByIndex(0);
#endif
  Serial.println(String("Temp")+String(DeviceData.temp)+String("  Hum:")+String(DeviceData.hum));
  if(isnan(DeviceData.temp)){
    INFO("Set default temp 20");
    DeviceData.temp=20.0;
  }
    if(isnan(DeviceData.hum)){
      INFO("Set default hum 50");
     DeviceData.hum=50.0;
  }
   if(isnan(DeviceData.pressure)){
    INFO("Set default pressure 1000");
    DeviceData.pressure=1000.0;
  }
  
}


#ifdef SEND_DATA_TO_THINGSPEAK
const char* thing_api_key="YOUR KEY";
void sendToThingspeak(){
   INFO("sendToThingspeak start");
    String url="http://api.thingspeak.com";
    url=" https://api.thingspeak.com/update?api_key="+String(thing_api_key);
    HTTPSimpleClient http;
  
    
     url+="&field1="+String(DeviceData.temp);
     url+="&field2="+String(DeviceData.hum);
     url+="&field3="+String(DeviceData.pressure);
     INFO(url);
     //nt httpcode=http.POST(poststr);
     if(!http.begin(url)){
       ERROR("Failed to connect to"+url );
    }
    int httpCode = http.GET();
   Serial.println("http code returns"+String(httpCode) );
    Serial.println("http returns"+http.getString() );
     
}

#endif
