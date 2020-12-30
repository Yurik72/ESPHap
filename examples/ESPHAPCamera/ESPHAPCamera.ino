
//  CONFIGURATION
#include "config.h"  //OPEN AND CHANGE





#include "esp_camera.h"
#include <WiFi.h>


#ifdef ENABLE_HAP
extern "C"{
#include "homeintegration.h"
}

#define ERPROM_START 100 // give first bytes for any usage
//#include <SPIFFS.h>
//#include <hapfilestorage\hapfilestorage.hpp>
#include "EEPROM.h"
homekit_service_t* hapservice={0};
#endif

//
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled
//
// YK   Please read  https://robotzero.one/esp32-face-door-entry/   this sketch used that

/// YK Configuration



// Select camera model
#define CAMERA_MODEL_WROVER_KIT_YK
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
//#define CAMERA_MODEL_AI_THINKER


#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "camera_pins.h"


#include "app_httpd.h"
#include "users_http.h" 
#include "Motion.h"
//#include "looptask_int.h"


#ifdef WIFI_MANAGER
#include "IotWebConf.h"
IotWebConf* piotWebConf=NULL;
DNSServer* pdnsServer=NULL;

WebServer* pserver=NULL;
#endif

#ifdef ESP_WIFI_MANAGER

#include "ESP32WifiManager.h"
ESP32WiFiManager wifiman;
#endif

const char* ssid = "LIS007";
const char* password = "Andru2000!";

void startCameraServer();
#ifdef WIFI_MANAGER
static bool isConfigPortal=false;
void startConfigPortal();
#endif
void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
	  config.frame_size =  FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif


#ifdef ESP_WIFI_MANAGER
  if (!wifiman.autoConnect(HOSTNAME, NULL, true)) {
    ESP.restart();
    delay(1000);
  }
#else
  WiFi.begin(ssid, password);
  int num_attempts=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
#ifdef WIFI_MANAGER
    num_attempts++;
    
    if(num_attempts>10){
      startConfigPortal();
      return;
    }
#endif
  }
#endif
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
#ifdef ENABLE_HAP

   hap_set_max_pairing(2);
	init_hap_storage();
	set_callback_storage_change(storage_changed);
    hap_setbase_accessorytype(homekit_accessory_category_security_system);
    /// init base properties
     hap_set_device_setupId((char*)"YK72");

    hap_initbase_accessory_service("host","Yurik72","0","EspHapCamera","1.0");

   //we will add only one light bulb service and keep pointer for nest using
    //hapservice= hap_add_button_service("Button");
	hapservice = hap_add_motion_service( "Motion", motion_callback, NULL);

	set_motioncallback(callback_motion);
   //and finally init HAP
    hap_init_homekit_server();
#endif
}
#ifdef ENABLE_HAP
void callback_motion(bool isDetected, long level) {
	notifyMotion(isDetected);
}
void notifyMotion(bool val) {
	homekit_characteristic_t * ch = homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_MOTION_DETECTED);
	if (ch) {
		if (ch->value.bool_value != val) {
			Serial.println("Notify Motion");
			ch->value.bool_value = val;
			homekit_characteristic_notify(ch, ch->value);
		}
	}
}
void motion_callback(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
	//nothing to do
}
void init_hap_storage() {
	Serial.print("init_hap_storage");


	int size = hap_get_storage_size_ex();
	int erpromsize = ERPROM_START + size;
	if (erpromsize > 512) {
		Serial.print("unable to initialize erprom");
	}
	EEPROM.begin(erpromsize);
	char* buf = new char[size];
	memset(buf, 0xff, size);
	EEPROM.readBytes(ERPROM_START, buf, size);

	Serial.printf("Storage size %d \r\n", size);
	// Serial.println(readed);
	hap_init_storage_ex(buf, size);

	delete[]buf;

}
void storage_changed(char * szstorage, int size) {
	if (EEPROM.length() < (ERPROM_START + size)) {
		Serial.print("unable to write erprom");
	}
	EEPROM.writeBytes(ERPROM_START, szstorage, size);
	EEPROM.commit();
}
#endif
void loop() {
  // put your main code here, to run repeatedly:
#ifdef WIFI_MANAGER
  if(piotWebConf){
    piotWebConf->doLoop();
    return;
  }
#endif
  delay(10000);
}

#ifdef WIFI_MANAGER



void handleRoot()
{
if(piotWebConf)
  piotWebConf->handleCaptivePortal();
}
void startConfigPortal(){
  Serial.println("startConfigPortal...");
  pdnsServer=new DNSServer();
  pserver=new WebServer();
  piotWebConf=new  IotWebConf("ESPCAM", pdnsServer, pserver, "");

  piotWebConf->init();
  pserver->on("/", handleRoot);
  pserver->on("/config", []{ piotWebConf->handleConfig(); });
  pserver->onNotFound([](){ piotWebConf->handleNotFound(); });
}
#endif
