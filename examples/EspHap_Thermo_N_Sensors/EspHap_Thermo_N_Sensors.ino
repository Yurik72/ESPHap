
#define ENABLE_OTA  //if OTA need
//#define SENSOR_TYPE_DHT  
//#define SENSOR_TYPE_BME280  // not supported a few BME in this version
#define SENSOR_TYPE_DALLAS 
//#define SEND_DATA_TO_THINGSPEAK  //To send data please specify your api_key to Thingspeak



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
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
bool isWebserver_started=false;
#define SENSRORS_COUNT 4

#ifdef SENSOR_TYPE_DHT
#include "DHT.h"   //https://github.com/adafruit/DHT-sensor-library
const size_t dht_pins[SENSRORS_COUNT]={2,5,8,9};
DHT* p_DHT[SENSRORS_COUNT];
#endif

#ifdef SENSOR_TYPE_BME280
#error "Currently few BME280 is not supported, there are a few solution how to manage few BME280 with the same ESP, out of scope this library"
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
//validate compiltion for issue #14
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

String pair_file_name="/pair.dat";


#define SENSOR_READ_PERIOD_MS 5000
#define SEND_THINGSPEAK_PERIOD_MS 500000

struct device_data_t{
  float temp=20.0;
 // float hum=50.0;
 // float pressure=1000.0;
  unsigned long next_read_sensor_ms=0;
  unsigned long next_send_thingspeak_ms=0;
};


device_data_t DeviceData[SENSRORS_COUNT];

const char* HOSTNAME="ESPThermo";
homekit_service_t* temperature[SENSRORS_COUNT]={0};

homekit_service_t* humidity=NULL;  

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
 startwifimanager();

    


    Serial.println("");
   // Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

#ifdef SENSOR_TYPE_DHT

#endif
#ifdef SENSOR_TYPE_BME280
 if(!BME.begin(BME_ADDR))
   Serial.print("Failed to Init BME280: ");
#endif
#ifdef SENSOR_TYPE_DHT
 for(int i=0;i<SENSRORS_COUNT;i++){
  p_DHT[i]=new DHT(DHT11,dht_pins[i]);
  p_DHT[i]->begin();
 }
 
#endif
#ifdef SENSOR_TYPE_DALLAS

#endif
/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

  
    init_hap_storage("/pair.dat");
  

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_thermostat);
    /// init base properties
    hap_initbase_accessory_service("THEMP","Yurik72","0","EspHapLed","1.0");
 
    
    // for base accessory registering temperature
    temperature[0] = hap_add_temperature_service("Temp-0");
    humidity=hap_add_hum_as_accessory(homekit_accessory_category_thermostat ,"Humidity"); 
    // Adding next sensors a
    for(int i=1;i<SENSRORS_COUNT;i++){
      String sensorname=String("Temp")+String(i);
     temperature[i]=hap_add_temp_as_accessory(homekit_accessory_category_sensor,sensorname.c_str());
      DeviceData[i].next_read_sensor_ms+=i*400; //offset for measure
    }
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
    server.send(200, FPSTR(TEXT_PLAIN),String(DeviceData[0].temp));
/*
  else if(server.arg("var") == "hum")
     server.send(200, FPSTR(TEXT_PLAIN),String(DeviceData.hum));
*/
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
  for(int i=0;i<SENSRORS_COUNT;i++){
     if(DeviceData[i].next_read_sensor_ms<=millis()){
        readSensor(i);
        yield();
        notify_hap(i);
        DeviceData[i].next_read_sensor_ms=millis()+SENSOR_READ_PERIOD_MS;
        yield();
     }
}

#ifdef SEND_DATA_TO_THINGSPEAK
if(DeviceData[0].next_send_thingspeak_ms<=millis()){
    sendToThingspeak();
    
    DeviceData[0].next_send_thingspeak_ms=millis()+SEND_THINGSPEAK_PERIOD_MS;
 }
#endif
 
#ifdef ESP8266
  hap_homekit_loop();
#endif
 if(isWebserver_started)
    server.handleClient();

}



void notify_hap(int sensor_index){

 if(temperature[sensor_index]){
  homekit_characteristic_t * ch_temp= homekit_service_characteristic_by_type(temperature[sensor_index], HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
  if(ch_temp ){
    // important if you send not valid values, iOS will reject including pairing
    HAP_NOTIFY_CHANGES_WITHCONSTRAIN(float, ch_temp, DeviceData[sensor_index].temp, 0)

  }
 }
 /*
if(humidity){
  homekit_characteristic_t * ch_hum= homekit_service_characteristic_by_type(humidity, HOMEKIT_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY);
  if(ch_hum && !isnan(DeviceData.hum) && ch_hum->value.float_value!=DeviceData.hum){
    ch_hum->value.float_value=DeviceData.hum;
    homekit_characteristic_notify(ch_hum,ch_hum->value);
  }
}
*/
}


void readSensor(int sensor_index){

#ifdef SENSOR_TYPE_DHT
  DeviceData[sensor_index].temp= p_DHT[sensor_index]->readTemperature();
//  DeviceData[sensor_index].hum = DHT.readHumidity();
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
  DeviceData[sensor_index].temp = DALLAS.getTempCByIndex(sensor_index);  // very slow , direct address should be used
  if(isnan(DeviceData[sensor_index].temp)){
        Serial.println("Set default temp 20");
         DeviceData[sensor_index].temp=20.0;
  }
#endif

}


#ifdef SEND_DATA_TO_THINGSPEAK
const char* thing_api_key="YOUR KEY";
void sendToThingspeak(){
   Serial.println("sendToThingspeak start");
    String url="http://api.thingspeak.com";
    url=" https://api.thingspeak.com/update?api_key="+String(thing_api_key);
    HTTPSimpleClient http;
  
 for(int i=0;i<SENSRORS_COUNT;i++){
     if(i>8)
      break;
     url+="&field"+String(i)+"="+String(DeviceData[i].temp);
}
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
