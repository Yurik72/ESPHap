/*******************************************************************
 * This sketch will allow homekit control of an ESP8266/8285 
 * RGBW LED controller. This example is based on the ARILUX AL_LC06.
 * You will need to determin the proper pins for your controller and
 * set the pin definitions accordingly. 
 * For ARILUX AL_LC06 set board to generic ESP8285 Module
********************************************************************/
#define REDPIN 14
#define GREENPIN 12
#define BLUEPIN 13
#define WHITEPIN 15 //ARILUX AL_LC06 WarmWhite is 15, CoolWhite is 5

#define ENABLE_OTA  // remove comments if OTA need
#define ENABLE_WIFI_MANAGER //add comments to manually enter Wifi SSIS/PASSWORD Below. Remove comment to have device create access point for configutation.

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "coredecls.h"

#ifdef ENABLE_OTA
#include <ArduinoOTA.h>
#endif

//Webserver

#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

#include "spiffs_webserver.h"
bool isWebserver_started=false;

const char* device_name="LEDStrip"; //Assign unique device name if you have more than one device

#ifdef ENABLE_WIFI_MANAGER
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#else
const char* ssid     = "yourssid";          //Enter your WiFi credentials if you are not using WiFiManager
const char* password = "yourpassword";      //Enter your WiFi credentials if you are not using WiFiManager
#endif

extern "C"{
#include "homeintegration.h"
}

#include "homekitintegrationcpp.h"

//This code is not used at this time as there is no on device color change ability
////color converters
//#define REDVALUE(x) ((x >> 16) & 0xFF)
//#define GREENVALUE(x)  ((x >> 8) & 0xFF)
//#define BLUEVALUE(x) ((x >> 0) & 0xFF)

struct device_data_t{
  bool IsOn=true;
  uint8_t Brightness=100;
  float Hue=0.0;
  float Saturation=0.0;
};

device_data_t DeviceData;

homekit_service_t* hapservice=NULL;
homekit_service_t* hapservice_motion=NULL;
homekit_characteristic_t * hap_on=NULL;
homekit_characteristic_t * hap_br=NULL;
homekit_characteristic_t * hap_hue=NULL;
homekit_characteristic_t * hap_saturation=NULL;

String pair_file_name="/pair.dat"; //store pairing information in this file

#include "spiffs_webserver.h"

int rgbw[4];

void setup() {
    disable_extra4k_at_link_time();

    Serial.begin(115200);
    delay(10);

    if (!SPIFFS.begin()) {
      //Serial.print("SPIFFS Mount failed");
     }
 
    pinMode(REDPIN, OUTPUT);
    pinMode(GREENPIN, OUTPUT);
    pinMode(BLUEPIN, OUTPUT);
    pinMode(WHITEPIN, OUTPUT);

#ifdef ENABLE_WIFI_MANAGER
    startwifimanager();
    WiFi.mode(WIFI_STA);
#else
    //Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin((char*)ssid, (char*)password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        //Serial.print(".");
        }
    //Serial.println("");
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
#endif
 
    //Serial.print("Free heap: ");
    //Serial.println(system_get_free_heap_size());

#ifdef ENABLE_OTA  //OTA setup
    ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });
    ArduinoOTA.setPassword("ESPHap");
    ArduinoOTA.setHostname(device_name);
    ArduinoOTA.begin(); 
#endif

    // setup homekit device 
   
    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);

    /// init base properties
    hap_initbase_accessory_service(device_name,"Yurik72","0","EspHapLedStrip","1.0");
    //we will add only one light bulb service and keep pointer for nest using
    
    //adding rgbw Accessory
    hapservice=hap_add_rgbstrip_service("RGBW",led_callback,NULL);
    hap_on=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);;
    hap_br=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_BRIGHTNESS);;
    hap_hue=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_HUE);;
    hap_saturation=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_SATURATION);

    hap_init_homekit_server();

  //Web server section
    if(hap_homekit_is_paired()){
    //Serial.println(PSTR("Setting web server"));
	SETUP_FILEHANDLES
	server.on("/get", handleGetVal);
	server.on("/set", handleSetVal);   
	server.begin(); 
	isWebserver_started=true;
    }else
	//Serial.println(PSTR("Web server is NOT SET, waiting for pairing"));
	//On restart set LEDs to off
	analogWrite(REDPIN, 0);
	analogWrite(GREENPIN, 0);
	analogWrite(BLUEPIN, 0);
	analogWrite(WHITEPIN, 0);
	//notify homekit of inital value LED is off
	INIT_CHARACHTERISTIC_VAL(bool,hap_on,false);
}

void notifyHAP(){
//  Serial.print("Notify HAP - ");
  if(hap_on && hap_on->value.bool_value!=DeviceData.IsOn){
//      Serial.print(" ON/OFF state: ");
//      Serial.println(DeviceData.IsOn);
      HAP_NOTIFY_CHANGES(int,hap_on,DeviceData.IsOn, 0)
    }
    if(hap_br && hap_br->value.int_value !=DeviceData.Brightness){
 //     Serial.print(" Brightness: ");
 //     Serial.println(DeviceData.Brightness);
      HAP_NOTIFY_CHANGES(int,hap_br,DeviceData.Brightness, 0)
    }
    if(hap_hue && hap_hue->value.float_value !=DeviceData.Hue){ //note this code is not used at this time. Device will not change hue or saturation
//      Serial.print(" Hue: ");
//      Serial.println(DeviceData.Hue);
      HAP_NOTIFY_CHANGES(int,hap_hue,DeviceData.Hue, 0)
    }
    if(hap_saturation && hap_saturation->value.float_value !=DeviceData.Saturation){
//      Serial.print(" Saturation: ");
//      Serial.println(DeviceData.Saturation);
      HAP_NOTIFY_CHANGES(int,hap_saturation,DeviceData.Saturation, 0)
    }
}

void handleGetVal(){
    server.send(200, FPSTR(TEXT_PLAIN), DeviceData.IsOn ?"1":"0");
}

void handleSetVal(){
  if (server.args() !=2){
    server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
    return;
  }
  bool isSucess=false;
  if(server.arg("var") == "ch1"){
		  if (server.arg("val")=="true"){
		      DeviceData.IsOn=true;
          HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,DeviceData.Brightness, rgbw);
		  } else {
          DeviceData.IsOn=false;
          HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,0, rgbw);
		  }
//      Serial.print("Web SetOn: ");
//      Serial.println(DeviceData.IsOn);
      notifyHAP();
      set_strip(rgbw);
      isSucess=true;
    }
     else if(server.arg("var") == "br"){
//      Serial.print("Web Set Brightness: ");
      DeviceData.Brightness = server.arg("val").toInt();
//      Serial.println(DeviceData.Brightness);
      notifyHAP();
      HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,DeviceData.Brightness, rgbw);
      set_strip(rgbw);
      isSucess=true;
      }
//    else if(server.arg("var") == "col"){ //Note: this code is not used Color info not sent from web server
//      Serial.println("Web Set Color");
//      uint32_t color=server.arg("val").toInt();
//      double Hue;
//      double Saturation;
//      double Intensity;
//      RGBtoHSI(color,(float) DeviceData.Brightness,Hue,Saturation,Intensity);
//      DeviceData.Saturation=Saturation;
//      DeviceData.Hue=Hue;
//      DeviceData.Brightness=Intensity;
//      notifyHAP();
//      HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,DeviceData.Brightness, rgbw);
//      set_strip(rgbw);
//      isSucess=true;
//      }
  if(isSucess)
        server.send(200, FPSTR(TEXT_PLAIN), "OK");
   else
        server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
}

void init_hap_storage(){
  //Serial.print("init_hap_storage");
  File fsDAT=SPIFFS.open(pair_file_name, "r");
  
 if(!fsDAT){
   //Serial.println("Failed to read pair.dat");
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
    //Serial.println("Failed to open pair.dat");
    return;
  }
  fsDAT.write((uint8_t*)szstorage, bufsize);
  fsDAT.close();
}


void led_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    //Serial.println("led_callback");
    bool isSet=false;
    bool isOff=false;
    if(ch==hap_on && ch->value.bool_value!=DeviceData.IsOn){
        DeviceData.IsOn=ch->value.bool_value;
        if (DeviceData.IsOn == false){
          isOff=true;
        }
        isSet=true;
    }
    if(ch==hap_br && ch->value.int_value!=DeviceData.Brightness && DeviceData.IsOn){
        DeviceData.Brightness=ch->value.int_value;
        isSet=true;
    }
    if(ch==hap_hue && ch->value.float_value!=DeviceData.Hue){
        DeviceData.Hue=ch->value.float_value;
        isSet=true;
    }
    if(ch==hap_saturation && ch->value.float_value!=DeviceData.Saturation){
        DeviceData.Saturation=ch->value.float_value;
        isSet=true;
    }
    if(isSet){
        if(isOff){
          HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,0, rgbw);
        }else{
          HSItoRGBW(DeviceData.Hue,DeviceData.Saturation,DeviceData.Brightness, rgbw);
        }
        set_strip(rgbw);
    }
}

#ifdef ENABLE_WIFI_MANAGER
void startwifimanager() {
    WiFiManager wifiManager;
    wifiManager.setCountry("US");
    delay(100);
    if (!wifiManager.autoConnect(device_name, NULL)) {
        ESP.restart();
        delay(1000);
     }
}
#endif

void set_strip(int RGBW[4]){
    analogWrite(REDPIN, RGBW[0]);
    analogWrite(GREENPIN, RGBW[1]);
    analogWrite(BLUEPIN, RGBW[2]);
    analogWrite(WHITEPIN, RGBW[3]);
//    Serial.print("Setting Strip: R=[");
//    Serial.print(RGBW[0]);
//    Serial.print("] - G=[");
//    Serial.print(RGBW[1]);
//    Serial.print("] - B=[");
//    Serial.print(RGBW[2]);
//    Serial.print("] - W=[");
//    Serial.print(RGBW[3]);
//    Serial.println("]");
}

void HSItoRGBW(float H, float S, float I, int* rgbw) {//https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white
    int r, g, b, w;
    float cos_h, cos_1047_h;
    //H = fmod(H,360); // cycle H around to 0-360 degrees
    H = 3.14159*H/(float)180; // Convert to radians.
    S /=(float)100; I/=(float)100; //from percentage to ratio
    S = S>0?(S<1?S:1):0; // clamp S and I to interval [0,1]
    I = I>0?(I<1?I:1):0;
    I = I*sqrt(I); //shape intensity to have finer granularity near 0
    
    if(H < 2.09439) {
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667-H);
        r = S*1023*I/3*(1+cos_h/cos_1047_h);
        g = S*1023*I/3*(1+(1-cos_h/cos_1047_h));
        b = 0;
        w = 1023*(1-S)*I;
    } else if(H < 4.188787) {
        H = H - 2.09439;
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667-H);
        g = S*1023*I/3*(1+cos_h/cos_1047_h);
        b = S*1023*I/3*(1+(1-cos_h/cos_1047_h));
        r = 0;
        w = 1023*(1-S)*I;
    } else {
        H = H - 4.188787;
        cos_h = cos(H);
        cos_1047_h = cos(1.047196667-H);
        b = S*1023*I/3*(1+cos_h/cos_1047_h);
        r = S*1023*I/3*(1+(1-cos_h/cos_1047_h));
        g = 0;
        w = 1023*(1-S)*I;
    }
    
    rgbw[0]=r;
    rgbw[1]=g;
    rgbw[2]=b;
    rgbw[3]=w;
}

//the code is not used at this time. No on device color setting is done
//void RGBtoHSI(uint32_t rgbcolor, uint32_t brightness,  double &Hue, double &Saturation, double &Intensity) {
//    uint32_t r = REDVALUE(rgbcolor);
//    uint32_t g = GREENVALUE(rgbcolor);
//    uint32_t b = BLUEVALUE(rgbcolor);
//
//    if ((r < 0 && g < 0 && b < 0) || (r > 255 || g > 255 || b > 255)){
//        Hue = Saturation = Intensity = 0;
//        return;
//    }
//  
//    if (g == b){
//        if (b < 255){
//            b = b + 1;
//        } else {
//            b = b - 1;
//        }
//    }
//    uint32_t nImax, nImin, nSum, nDifference;
//    nImax = max(r, b);
//    nImax = max(nImax, g);
//    nImin = min(r, b);
//    nImin = min(nImin, g);
//    nSum = nImin + nImax;
//    nDifference = nImax - nImin;
//  
//    Intensity = (float)nSum / 2;
//  
//    if (Intensity < 128){
//        Saturation = (255 * ((float)nDifference / nSum));
//    } else {
//        Saturation = (float)(255 * ((float)nDifference / (510 - nSum)));
//    }
//  
//    if (Saturation != 0){
//        if (nImax == r){
//            Hue = (60 * ((float)g - (float)b) / nDifference);
//        } else if (nImax == g){
//            Hue = (60 * ((float)b - (float)r) / nDifference + 120);
//        }
//        else if (nImax == b){
//            Hue = (60 * ((float)r - (float)g) / nDifference + 240);
//        }
//    
//        if (Hue < 0){
//            Hue = (60 * ((float)b - (float)r) / nDifference + 120);
//        }
//    } else {
//        Hue = -1;
//    }
//    return;
//}

void loop() { //main program loop
#ifdef ENABLE_OTA
    ArduinoOTA.handle(); //check for OTA updates
#endif

  hap_homekit_loop();
  
  if(isWebserver_started)
    server.handleClient();
}
