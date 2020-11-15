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

const int motion_pin=17;  //GPIO pin where the motion sensor is attached to

#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager
#include <WS2812FX.h>  //https://github.com/kitesurfer1404/WS2812FX

#define RGB_LED_COUNT 8 //adopt for your project
#define RGB_LED_PIN 23 //adopt for your project

WS2812FX ws2812fx = WS2812FX(RGB_LED_COUNT, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

const char* HOSTNAME="ES";
const char* ssid     = "ssid";
const char* password = "pwd";

extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif

//color converters
#define REDVALUE(x) ((x >> 16) & 0xFF)
#define GREENVALUE(x)  ((x >> 8) & 0xFF)
#define BLUEVALUE(x) ((x >> 0) & 0xFF)
#define MOTION_CHECK_PERIOD 100
struct device_data_t{
  bool IsOn=true;
  uint8_t Brigthness=20;
  float Hue=0.0;
  float Saturation=0.0;
  bool IsMotion=false;
  unsigned long NextMotionCheck_ms=0;
  int get_br_100(){
    return map(Brigthness,0,0xFF,0,100);
  }
  void set_br_100(int val){
    Brigthness= map(val,0,100,0,0xFF);
  };
};

device_data_t DeviceData;

homekit_service_t* hapservice={0};
homekit_service_t* hapservice_motion={0};
homekit_characteristic_t * hap_on={0};
homekit_characteristic_t * hap_br={0};
homekit_characteristic_t * hap_hue={0};
homekit_characteristic_t * hap_saturation={0};

String pair_file_name="/pair.dat"; //store pairing information in this file

void lamp_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context);

//Web server section
#define ENABLE_OTA  //if OTA need

#include "spiffs_webserver.h"

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
 #ifdef ESP8266 
  disable_extra4k_at_link_time();
 #endif 
  Serial.begin(115200);
    delay(10);
    // We start by connecting to a WiFi network
    set_strip();
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
  
///setup identity gpio
   
    //this is for custom storage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    Serial.print("Free heap: ");
    Serial.println(system_get_free_heap_size());

/// Setting RGB WS2812
  ws2812fx.init();
  ws2812fx.setBrightness(DeviceData.Brigthness);
  ws2812fx.setMode(FX_MODE_STATIC);

//Setting Motion
    pinMode(motion_pin, INPUT);
    
    init_hap_storage();
  
    set_callback_storage_change(storage_changed);

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_lightbulb);

    /// init base properties
    hap_initbase_accessory_service(HOSTNAME,"Yurik72","0","EspHapLed","1.0");
   //we will add only one light bulb service and keep pointer for nest using
    
//adding rgb Accessory
  hapservice=hap_add_rgbstrip_service("RGB",lamp_callback,NULL);
  hap_on=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);;
  hap_br=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_BRIGHTNESS);;
  hap_hue=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_HUE);;
  hap_saturation=homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_SATURATION);
  if(hap_br)
    hap_br->value.int_value=DeviceData.get_br_100();   // initial value
//Adding Motion accessory
 hapservice_motion= hap_add_motion_service_as_accessory(homekit_accessory_category_security_system,"Motion",motion_callback,NULL);
   
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

#ifdef ESP8266
 if(hap_homekit_is_paired()){
#endif
  Serial.println(PSTR("Setting web server"));
  SETUP_FILEHANDLES
  server.on("/get", handleGetVal);
  server.on("/set", handleSetVal);   
  server.begin(); 
  isWebserver_started=true;
#ifdef ESP8266
}else
 Serial.println(PSTR("Web server is NOT SET, waiting for pairing"));
#endif
//notifyRGB();
}

void checkMotion(){
  bool motion = digitalRead(motion_pin)==HIGH;
 // Serial.println(String("Motion pin")+String(digitalRead(motion_pin)));
  if(DeviceData.IsMotion!=motion){
    Serial.println(String("Motion is: ")+String(motion?"ON":"OFF"));
    DeviceData.IsMotion=motion;
    notifyMotion();
  }
}

void notifyMotion(){
  homekit_characteristic_t * ch= homekit_service_characteristic_by_type(hapservice_motion, HOMEKIT_CHARACTERISTIC_MOTION_DETECTED);
    if(ch){
      if(ch->value.bool_value!=DeviceData.IsMotion){
        Serial.println("Notify Motion");
        ch->value.bool_value=DeviceData.IsMotion;
        homekit_characteristic_notify(ch,ch->value);
      }
    }
}

void notifyRGB(){
  if(hap_on && hap_on->value.bool_value!=DeviceData.IsOn){
      hap_on->value.bool_value=DeviceData.IsOn;
      homekit_characteristic_notify(hap_on,hap_on->value);
    }
    if(hap_br && hap_br->value.int_value !=DeviceData.get_br_100()){
      hap_br->value.int_value=DeviceData.get_br_100();
      homekit_characteristic_notify(hap_br,hap_br->value);
    }
    if(hap_hue && hap_hue->value.float_value !=DeviceData.Hue){
      hap_hue->value.float_value=DeviceData.Hue;
      homekit_characteristic_notify(hap_hue,hap_hue->value);
    }
    if(hap_saturation && hap_saturation->value.float_value !=DeviceData.Saturation){
      hap_saturation->value.float_value=DeviceData.Saturation;
      homekit_characteristic_notify(hap_saturation,hap_saturation->value);
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
  //to do analyze
  bool isSucess=false;
  if(server.arg("var") == "ch1"){
 
		  DeviceData.IsOn=(server.arg("val")=="true");
      Serial.println("Web SetOn");
		  set_strip();
      notifyRGB();
      isSucess=true;
    }
     else if(server.arg("var") == "br"){
      //Serial.println("Web Set Brigthness");
      DeviceData.set_br_100(server.arg("val").toInt());
      
      notifyRGB();
      set_strip();
      isSucess=true;
      }
    else if(server.arg("var") == "col"){
      // Serial.println("Web Set Color");
      uint32_t color=server.arg("val").toInt();
      double Hue;
      double Saturation;
      double Intensity;
      ColorToHSI(color,(float) DeviceData.get_br_100(),Hue,Saturation,Intensity);
      DeviceData.Saturation=Saturation;
      DeviceData.Hue=Hue;
      notifyRGB();
      set_strip();
      isSucess=true;
      }
  if(isSucess)
        server.send(200, FPSTR(TEXT_PLAIN), "OK");
   else
        server.send(505, FPSTR(TEXT_PLAIN), "Bad args");
}

void loop() { //main program loop

#ifdef ESP8266
  hap_homekit_loop();
#endif
  
if(isWebserver_started)
    server.handleClient();

ws2812fx.service();

if(DeviceData.NextMotionCheck_ms<=millis()){
    checkMotion();
    DeviceData.NextMotionCheck_ms=millis()+MOTION_CHECK_PERIOD;
 }
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
void motion_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context){
//nothing to do
}

void lamp_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    Serial.println("lamp_callback");
     bool isSet=false;
    if(ch==hap_on && ch->value.bool_value!=DeviceData.IsOn){
      DeviceData.IsOn=ch->value.bool_value;
	    //DeviceData.Brigthness = 0;
      isSet=true;
    }
    if(ch==hap_br && ch->value.int_value!=DeviceData.get_br_100() && DeviceData.IsOn){
      DeviceData.set_br_100(ch->value.int_value);
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
    if(isSet)
      set_strip();
}

void startwifimanager() {
  WiFiManager wifiManager;
//  WiFi.onStationModeDisconnected(onWifiDisconnect);
//  WiFi.onStationModeConnected(onWifiConnect);
  if (!wifiManager.autoConnect(HOSTNAME, NULL)) {
      ESP.restart();
      delay(1000);
   }
}

void set_strip(){
  if(DeviceData.IsOn){
    uint32_t color = HSVColor(DeviceData.Hue, DeviceData.Saturation/100.0, DeviceData.get_br_100()/100.0);
    ws2812fx.setColor(color);
    ws2812fx.setBrightness(DeviceData.Brigthness);
  }
  else{
     ws2812fx.setBrightness(0);
  }
   ws2812fx.trigger();
}
uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Convert Hue/Saturation/Brightness values to a packed 32-bit RBG color.
// hue must be a float value between 0 and 360
// saturation must be a float value between 0 and 1
// brightness must be a float value between 0 and 1
uint32_t HSVColor(float h, float s, float v) {
  h = constrain(h, 0.0, 360.0);
  s = constrain(s, 0.0, 1.0);
  v = constrain(v, 0.0, 1.0);

  int i, b, p, q, t;
  float f;

  h /= 60.0;  // sector 0 to 5
  i = floor(h);
  f = h - i;  // factorial part of h

  b = v * 255;
  p = v * (1 - s) * 255;
  q = v * (1 - s * f) * 255;
  t = v * (1 - s * (1 - f)) * 255;

  switch (i) {
  case 0:
    return Color(b, t, p);
  case 1:
    return Color(q, b, p);
  case 2:
    return Color(p, b, t);
  case 3:
    return Color(p, q, b);
  case 4:
    return Color(t, p, b);
  default:
    return Color(b, p, q);
  }
}

void ColorToHSI(uint32_t rgbcolor, uint32_t brightness,  double &Hue, double &Saturation, double &Intensity) {
  uint32_t r = REDVALUE(rgbcolor);
  uint32_t g = GREENVALUE(rgbcolor);
  uint32_t b = BLUEVALUE(rgbcolor);

  if ((r < 0 && g < 0 && b < 0) || (r > 255 || g > 255 || b > 255))
  {
    Hue = Saturation = Intensity = 0;
    return;
  }

  if (g == b)
  {
    if (b < 255)
    {
      b = b + 1;
    }
    else
    {
      b = b - 1;
    }
  }
  uint32_t nImax, nImin, nSum, nDifference;
  nImax = max(r, b);
  nImax = max(nImax, g);
  nImin = min(r, b);
  nImin = min(nImin, g);
  nSum = nImin + nImax;
  nDifference = nImax - nImin;

  Intensity = (float)nSum / 2;

  if (Intensity < 128)
  {
    Saturation = (255 * ((float)nDifference / nSum));
  }
  else
  {
    Saturation = (float)(255 * ((float)nDifference / (510 - nSum)));
  }

  if (Saturation != 0)
  {
    if (nImax == r)
    {
      Hue = (60 * ((float)g - (float)b) / nDifference);
    }
    else if (nImax == g)
    {
      Hue = (60 * ((float)b - (float)r) / nDifference + 120);
    }
    else if (nImax == b)
    {
      Hue = (60 * ((float)r - (float)g) / nDifference + 240);
    }

    if (Hue < 0)
    {
      Hue = (60 * ((float)b - (float)r) / nDifference + 120);
    }
  }
  else
  {
    Hue = -1;
  }
  return;
}
