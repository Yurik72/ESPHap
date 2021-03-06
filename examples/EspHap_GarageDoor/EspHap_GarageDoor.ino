#include <Arduino.h>
#include "config.h"
#include "simplesensor.h"
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager


#ifdef ENABLE_REMOTE_DEBUG
#define ENABLE_WEB_SERVER false
#include "RemoteDebug.h"
#else
#define DEBUG(fmt, ...)  printf("(%s) " fmt, __func__, ##__VA_ARGS__)
#endif



#ifdef ENABLE_OTA
#include "wifi_ota.h"
#endif

#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif

#if ENABLE_WEB_SERVER
#ifdef ESP8266
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#endif

#ifdef ESP32
#include <WebServer.h>
WebServer server(80);
#endif
#endif


#if defined(ESP32) && defined(ENABLE_OTA)
#include <Update.h>
#endif

#if ENABLE_WEB_SERVER
#include "spiffs_webserver.h"
bool isWebserver_started = false;
#endif


// For reporting heap usage on the serial output every 5 seconds
//static uint32_t next_heap_millis = 0;
unsigned long next_led_millis = 0;

extern "C" {
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include "hapfilestorage/hapfilestorage.hpp"

#if ENABLE_WEB_SERVER
#include "spiffs_webserver.h"
#endif

homekit_service_t* service_garagedoor = NULL;

homekit_characteristic_t * ch_currentstate = NULL;
homekit_characteristic_t * ch_targetstate = NULL;
homekit_characteristic_t * ch_obstruction = NULL;

SimpleSensor Sensor_open(OPEN_PIN);
SimpleSensor Sensor_close(CLOSE_PIN);


#ifdef ENABLE_REMOTE_DEBUG
RemoteDebug Debug;
#endif

void sensor_callback(uint8_t gpio_num, uint8_t state)
{
  current_door_state_update_from_sensor();
}

uint8_t current_door_state = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN; //HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_UNKNOWN;
uint8_t target_door_state = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN;//HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_UNKNOWN;

void startwifimanager() {
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect(homekit_name, NULL)) {
    ESP.restart();
    delay(1000);
  }
}

void relay_write(bool on) {
  digitalWrite(RELAY_PIN, on ? HIGH : LOW);
}

void current_state_set(uint8_t new_state) {
  if (ch_currentstate) {
    DEBUG("set current state to %i\n", new_state);
    current_door_state = new_state;
    HAP_NOTIFY_CHANGES(int, ch_currentstate, new_state, 0)
  }
}

void target_state_set(uint8_t new_state) {
  if (ch_targetstate) {
    DEBUG("set target state to %i\n", new_state);
    target_door_state = new_state;
    HAP_NOTIFY_CHANGES(int, ch_targetstate, new_state, 0)
  }
}

void obstruction_state_set(uint8_t new_state) {
  if (ch_obstruction) {
    DEBUG("set obstruction to %i\n", new_state);
    HAP_NOTIFY_CHANGES(int, ch_obstruction, new_state, 0)
  }
}

void current_door_state_update_from_sensor() {

  uint8_t state_open = Sensor_open.getstate();
  uint8_t state_close = Sensor_close.getstate();

  if  (  state_open == LOW ) {
    DEBUG("Pin Open closed\n");
  }
  else {
    DEBUG("Pin Open open\n");
  }

  if  ( state_close == LOW ) {
    DEBUG("Pin Close closed\n");
  }
  else {
    DEBUG("Pin Close open\n");
  }


  uint8_t currentstate;
  uint8_t targetstate;

  currentstate = current_door_state;
  targetstate = target_door_state;

  // Read the sensors and use some logic to determine state
  if ( state_open == LOW ) {
    // If PIN_SENSOR_OPENED is low, it's being pulled to ground, which means the switch at the top of the track is closed, which means the door is open
    targetstate = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN; // NOP, if already set
    currentstate = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN;
  }
  else if ( state_close == LOW ) {
    // If PIN_SENSOR_CLOSED is low, it's being pulled to ground, which means the switch at the bottom of the track is closed, which means the door is closed
    targetstate = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED; // NOP, if already set
    currentstate = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED;
  } else {
    // If neither, then the door is in between switches, so we use the last known state to determine which way it's probably going
    if (currentstate == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED) {
      // Current door state was "closed" so we are probably now "opening"
      currentstate = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING;
    } else if ( currentstate == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN ) {
      // Current door state was "opened" so we are probably now "closing"
      currentstate = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING;
    }

    // If it is traveling, then it might have been started by the button in the garage. Set the new target state:
    if ( currentstate == HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING ) {
      targetstate = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN;
    } else if ( currentstate = HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING ) {
      targetstate = HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED;
    }
  }

  current_state_set(currentstate);
  target_state_set(targetstate);

  print_target_door_state();
  print_current_door_state();
}

void print_current_door_state() {
  String msg = "Current door state: ";
  msg += current_door_state;
  switch (current_door_state) {
    case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPEN:
      msg += " - open";
      break;
    case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSED:
      msg += " - closed";
      break;
    case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_OPENING:
      msg += " - opening";
      break;
    case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_CLOSING:
      msg += " - closing";
      break;
    case HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE_STOPPED:
      msg += " - stopped";
      break;
    default:
      msg += " - unknown";

  }
  DEBUG("%s\n",msg.c_str());
}


void print_target_door_state()
{
  String msg = "Target door state: ";
  msg += target_door_state;
  switch  (target_door_state) {
    case HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN:
      msg += " - open";
      break;
    case HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED:
      msg += " - closed";
      break;
    default:
      msg += " - unkown";
  }
  DEBUG("%s\n",msg.c_str());
}

void hap_callback_process(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
  DEBUG("hap_callback\n");
  if (!service_garagedoor) {
    DEBUG("service not defined\n");
    return;

  }

  if (ch == ch_targetstate) {
    Serial.println("processing target state");
    if (current_door_state != HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_OPEN &&
        current_door_state != HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE_CLOSED) {
      DEBUG("target_state_set  ignored: current state not open or closed.\n");
      return;
    }
    if (current_door_state == value.int_value) {
      DEBUG("target_state_set ignored: target state == current state\n");
      return;
    }
    DEBUG("Trigger relay\n");
    relay_write(true);
    // Wait for some time:
    delay(500); ///????
    // Turn OFF GPIO:
    relay_write(false);

  }

}

//---------------------

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
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(RELAY_PIN, OUTPUT);
  Sensor_open.start(sensor_callback);
  Sensor_close.start(sensor_callback);


  startwifimanager();

#ifdef ENABLE_OTA
  wifi_ota_setup();
#endif

#ifdef ENABLE_REMOTE_DEBUG
  Debug.begin(homekit_name); 
  Debug.setSerialEnabled(true);
  Debug.setResetCmdEnabled(true); 
#endif

  /// now will setup homekit device

  //this is for custom storaage usage
  // In given example we are using \pair.dat   file in our spiffs system
  //see implementation below
  DEBUG("Free heap: %i\n",system_get_free_heap_size());
  

  init_hap_storage("/pair.dat");

  /// We will use for this example only one accessory (possible to use a several on the same esp)
  //Our accessory type is light bulb , apple interface will proper show that
  hap_setbase_accessorytype(homekit_accessory_category_door);
  /// init base properties
  hap_initbase_accessory_service(homekit_name, "Arduino Homekit", "123456789", homekit_name, "1.0");


  // for base accessory registering temperature
  service_garagedoor = hap_add_garagedoor_service("garagedoor", hap_callback_process, 0);

  ch_currentstate = homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_CURRENT_DOOR_STATE);
  ch_targetstate = homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_TARGET_DOOR_STATE);
  ch_obstruction = homekit_service_characteristic_by_type(service_garagedoor, HOMEKIT_CHARACTERISTIC_OBSTRUCTION_DETECTED);

  hap_init_homekit_server();

  current_door_state = ch_currentstate->value.int_value;
  target_door_state = ch_targetstate->value.int_value;

  DEBUG("Init states\n");
  print_target_door_state();
  print_current_door_state();


  obstruction_state_set(0);
  current_door_state_update_from_sensor();



#if ENABLE_WEB_SERVER
  String strIP = String(WiFi.localIP()[0]) + String(".") + String(WiFi.localIP()[1]) + String(".") +  String(WiFi.localIP()[2]) + String(".") +  String(WiFi.localIP()[3]);
#ifdef ESP8266
  if (hap_homekit_is_paired()) {
#endif
    DEBUG("Setting web server\n");
    SETUP_FILEHANDLES
    //    server.on("/get", handleGetVal);
    //    server.on("/set", handleSetVal);
    server.begin();
    DEBUG("Web site http://%s \n", strIP.c_str());
    DEBUG("File system http://%s/browse\n",strIP.c_str());
    DEBUG("Update http://%s/update\n",strIP.c_str());
    DEBUG("/update\n");
    isWebserver_started = true;
#ifdef ESP8266
  } else
    DEBUG("Web server is NOT SET, waiting for pairing\n");
#endif

#endif
}


//---------------------

void loop() {

#ifdef ESP8266
  hap_homekit_loop();
#endif

#if ENABLE_WEB_SERVER
  if (isWebserver_started)
    server.handleClient();
#endif
#ifdef ENABLE_OTA
  ArduinoOTA.handle();
#endif

#ifdef ENABLE_REMOTE_DEBUG
  Debug.handle();
#endif


if (millis() > next_led_millis) {
    digitalWrite(LED_BUILTIN, LOW);  // Change the state of the LED
    delay(5);
    digitalWrite(LED_BUILTIN, HIGH);  // Change the state of the LED
    next_led_millis = millis() + 1000;
  }

}
//-----------
