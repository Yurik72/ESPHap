
#define ENABLE_OTA  //if OTA need
#define SENSOR_TYPE_DHT  
//#define SENSOR_TYPE_BME280
//#define SENSOR_TYPE_DALLAS 
#define SEND_DATA_TO_THINGSPEAK  //To send data please specify your api_key to Thingspeak

const char* ssid     = "";
const char* password = "";

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


String pair_file_name="/pair.dat";

homekit_service_t* thermostat=NULL;



#define SENSOR_READ_PERIOD_MS 5000
#define SEND_THINGSPEAK_PERIOD_MS 500000

struct device_data_t{
  float temp=20.0;
  float hum=50.0;
  float pressure=1000.0;
  unsigned long next_read_sensor_ms=0;
  unsigned long next_send_thingspeak_ms=0;
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

#ifdef SENSOR_TYPE_DHT
DHT.begin();
#endif
#ifdef SENSOR_TYPE_BME280
 if(!BME.begin(BME_ADDR))
   Serial.print("Failed to Init BME280: ");
#endif

#ifdef SENSOR_TYPE_DALLAS

#endif
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
    thermostat = hap_add_thermostat_service("Thermostat",hap_update,0);
   
   
   
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
 if(DeviceData.next_read_sensor_ms<=millis()){
    readSensor();
    notify_hap();
    DeviceData.next_read_sensor_ms=millis()+SENSOR_READ_PERIOD_MS;
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

 if(thermostat){
  homekit_characteristic_t * ch_temp= homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
  if(ch_temp && !isnan(DeviceData.temp) &&  ch_temp->value.float_value!=DeviceData.temp ){
    ch_temp->value.float_value=DeviceData.temp;
    homekit_characteristic_notify(ch_temp,ch_temp->value);
  }
 }

}
void hap_update(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("hap_update");
    if(!thermostat){
       Serial.println("service not defined");
      return;
   
    }
     homekit_characteristic_t * ch_temp= homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
     homekit_characteristic_t * ch_targetstate= homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_TARGET_HEATING_COOLING_STATE);
     homekit_characteristic_t * ch_currentstate= homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_CURRENT_HEATING_COOLING_STATE);

     homekit_characteristic_t*  ch_target_temperature=homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_TARGET_TEMPERATURE);
     homekit_characteristic_t* ch_heating_threshold=homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_HEATING_THRESHOLD_TEMPERATURE);
     homekit_characteristic_t* ch_cooling_threshold=homekit_service_characteristic_by_type(thermostat, HOMEKIT_CHARACTERISTIC_COOLING_THRESHOLD_TEMPERATURE);

     
if(!ch_temp || !ch_targetstate || !ch_target_temperature){
    Serial.println("characteristic wrong defined");
  return;
}
 uint8_t state = ch_targetstate->value.int_value;
     if ((state == 1 && ch_temp->value.float_value < ch_target_temperature->value.float_value) ||
            (state == 3 && ch_temp->value.float_value < ch_heating_threshold->value.float_value)) {
        if (ch_currentstate->value.int_value != 1) {
            ch_currentstate->value = HOMEKIT_UINT8_VALUE(1);
            homekit_characteristic_notify(ch_currentstate, ch_currentstate->value);

            heaterOn();
            coolerOff();
            fanOff();
            fanOn(5000);
        }
    } else if ((state == 2 && ch_temp->value.float_value > ch_target_temperature->value.float_value) ||
            (state == 3 && ch_temp->value.float_value > ch_cooling_threshold->value.float_value)) {
        if (ch_currentstate->value.int_value != 2) {
            ch_currentstate->value = HOMEKIT_UINT8_VALUE(2);
            homekit_characteristic_notify(ch_currentstate, ch_currentstate->value);

            coolerOn();
            heaterOff();
            fanOff();
            fanOn(5000);
        }
    } else {
        if (ch_currentstate->value.int_value != 0) {
            ch_currentstate->value = HOMEKIT_UINT8_VALUE(0);
            homekit_characteristic_notify(ch_currentstate, ch_currentstate->value);

            coolerOff();
            heaterOff();
            fanOff();
        }
    }
}

void heaterOn() {
   Serial.println("heaterOn");
}


void heaterOff() {
     Serial.println("heaterOff");
}


void coolerOn() {
    Serial.println("coolerOn");
}


void coolerOff() {
    Serial.println("coolerOff");
}


void fan_alarm(void *arg) {
     Serial.println("fan_alarm");
}

void fanOn(uint16_t delay) {
Serial.println("fanOn");
}


void fanOff() {
Serial.println("fanOff");
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
    Serial.println("Set default temp 20");
    DeviceData.temp=20.0;
  }
    if(isnan(DeviceData.hum)){
      Serial.println("Set default hum 50");
     DeviceData.hum=50.0;
  }
   if(isnan(DeviceData.pressure)){
    Serial.println("Set default pressure 1000");
    DeviceData.pressure=1000.0;
  }
  
}


#ifdef SEND_DATA_TO_THINGSPEAK
const char* thing_api_key="YOUR KEY";
void sendToThingspeak_old(){
   Serial.println("sendToThingspeak start");
    String url="http://api.thingspeak.com";
    url=" https://api.thingspeak.com/update?api_key="+String(thing_api_key);
    HTTPSimpleClient http;
  
    
     url+="&field1="+String(DeviceData.temp);
     url+="&field2="+String(DeviceData.hum);
     url+="&field3="+String(DeviceData.pressure);
     Serial.println(url);
     //nt httpcode=http.POST(poststr);
     if(!http.begin(url)){
       Serial.println("Failed to connect to"+url );
    }
    int httpCode = http.GET();
   Serial.println("http code returns"+String(httpCode) );
    Serial.println("http returns"+http.getString() );
     
}
void sendToThingspeak(){
   Serial.println("sendToThingspeak start");
    String url="https://www.borneland.com/sensor/post/";
    
    HTTPSimpleClient http;
     String apiKeyValue="tPmAT5Ab3j7F9";
     String sensorName="14661474";
     String postString="api_key="+String(apiKeyValue);
     postString+="&chip_id="+String(sensorName);
     postString+="&temp="+String(DeviceData.temp);
     postString+="&hum="+String(DeviceData.hum);
     Serial.println(url);
     
     if(!http.begin(url)){
       Serial.println("Failed to connect to "+url );
    }
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpcode = http.POST(postString);
    String response=http.getString();
    Serial.println("postString "+String(postString) );
   Serial.println("http code returns "+String(httpcode) );
   Serial.println("http code response "+String(response) );   
}
#endif
