
////  !!!! This sketch is not finished yet
///   It just shows how to add/integrate window covering charachteristic with apple
///   window covering functionality should be reviewed and properly implemented
///   especially function handle_motor() , should implement physical writing "something" to GPIOs, depending on Motor driver




#include <Arduino.h>


#ifdef ESP32
#include <SPIFFS.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>

#include <ESP8266mDNS.h>
#include "coredecls.h"
#endif
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

#ifdef ESP8266
#define INFO_(message, ...) printf_P(PSTR(">>> Window covering: " message "\n"), ##__VA_ARGS__)
#define ERROR_(message, ...) printf_P(PSTR("!!! Window covering: " message "\n"), ##__VA_ARGS__)
#else
#define INFO_(message, ...) printf(">>> Window covering: " message "\n", ##__VA_ARGS__)
#define ERROR_(message, ...) printf("!!! Window covering: " message "\n", ##__VA_ARGS__)
#endif

const char* HOSTNAME="EspWindow";
const int identity_led=2;




extern "C"{
#include "homeintegration.h"
}
#ifdef ESP8266
#include "homekitintegrationcpp.h"
#endif
#include <hapfilestorage/hapfilestorage.hpp>




homekit_service_t* service_windowcovering=NULL;
homekit_characteristic_t * ch_current_pos= NULL;
homekit_characteristic_t * ch_target_pos=NULL;
homekit_characteristic_t * ch_position_state=NULL;

homekit_characteristic_t * ch_hold=NULL;

#ifdef ESP32
// to be checked
// #define ESP_TASK
#endif

#ifdef ESP_TASK
#include <task.h>
TaskHandle_t updateStateTask;
#else
esp_timer_create_args_t _timerConfig;
esp_timer_handle_t _timer=NULL;
#endif


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
        ERROR_("SPIFFS Mount failed\n");
     }
#endif
#ifdef ESP8266
     if (!SPIFFS.begin()) {
      ERROR_("SPIFFS Mount failed\n");
     }
#endif

 
 
 startwifimanager();




/// now will setup homekit device

    //this is for custom storaage usage
    // In given example we are using \pair.dat   file in our spiffs system
    //see implementation below
    INFO("Free heap: %d \n",system_get_free_heap_size());
  
  
    init_hap_storage("/pair.dat");
  
  

    /// We will use for this example only one accessory (possible to use a several on the same esp)
    //Our accessory type is light bulb , apple interface will proper show that
    hap_setbase_accessorytype(homekit_accessory_category_window_covering);
    /// init base properties
    hap_initbase_accessory_service("ES","Yurik72","0","EspHapLed","1.0");
 
    
    // for base accessory registering window covering
    service_windowcovering = hap_add_windowcovering_service("window",hap_callback_process,0);
   
  ch_current_pos= homekit_service_characteristic_by_type(service_windowcovering, HOMEKIT_CHARACTERISTIC_CURRENT_POSITION);
  ch_target_pos= homekit_service_characteristic_by_type(service_windowcovering, HOMEKIT_CHARACTERISTIC_TARGET_POSITION);
  ch_position_state= homekit_service_characteristic_by_type(service_windowcovering, HOMEKIT_CHARACTERISTIC_POSITION_STATE);
  //ch_hold=hap_add_hold_characteristik_to_windowcovering(service_windowcovering,hap_callback_process,0);
  //Set  default values
  INIT_CHARACHTERISTIC_VAL(int,ch_current_pos,50);
  INIT_CHARACHTERISTIC_VAL(int,ch_target_pos,50);
 
 
  create_update_task();   
  hap_init_homekit_server();
  
}
void loop() {
 
#ifdef ESP8266
  hap_homekit_loop();
#endif
}
void handle_motor(){
  if(!ch_target_pos || !ch_position_state || !ch_current_pos ){
    ERROR_("Positions charachteristic are not defined\n");
    return;
  }
  uint8_t current_position = ch_current_pos->value.int_value;
  int8_t target_position = ch_target_pos->value.int_value ;
  INFO_("Handle Motor State:%d, Current:%d, Target:%d\n",ch_position_state->value.int_value,current_position,target_position);  
  switch(ch_position_state->value.int_value){
    case WINDOWCOVERING_POSITION_STATE_OPENING:
      INFO_("Handle motor OPEN \n");
      break;
    case WINDOWCOVERING_POSITION_STATE_CLOSING:
      INFO_("Handle motor CLOSE \n");
      break;
    case WINDOWCOVERING_POSITION_STATE_STOPPED:
     INFO_("Handle motor STOP \n");
      break;
    default:
     ERROR_("Unknown value of target position charachteristic\n");
  }
}

void create_update_task() {
#ifdef ESP_TASK
    xTaskCreate(update_state, "UpdateState", 256, NULL, tskIDLE_PRIORITY, &updateStateTask);
    vTaskSuspend(updateStateTask);
#else
  _timerConfig.arg = 0;
  _timerConfig.callback = update_window_state_arg;
  _timerConfig.dispatch_method = ESP_TIMER_TASK;
  _timerConfig.name = "W";
  esp_timer_create(&_timerConfig, &_timer);
#endif
}

void start_update_task(){
#ifdef ESP_TASK
  
#else
  esp_timer_start_periodic(_timer,   10*500ULL);
#endif
}
void suspend_update_task(){
#ifdef ESP_TASK
   vTaskSuspend(updateStateTask);
#else
  esp_timer_stop(_timer);
#endif

}
void resume_update_task(){

#ifdef ESP_TASK
    vTaskResume(updateStateTask);
#else 
  esp_timer_start_periodic(_timer,   10*500ULL);
#endif

}
void update_window_state_arg(void* arg){
  update_window_state();
}
void update_window_state() {

#ifdef ESP_TASK
    while (true) {
#endif
        uint8_t position = ch_current_pos->value.int_value;
        int8_t direction = 0;
        if(ch_position_state->value.int_value == WINDOWCOVERING_POSITION_STATE_OPENING)
          direction+=1;
        if(ch_position_state->value.int_value == WINDOWCOVERING_POSITION_STATE_CLOSING)
          direction+=-1;

        int16_t newPosition = position + direction;
        

        INFO_("position %u, target %u\n", newPosition, ch_target_pos->value.int_value);
        HAP_NOTIFY_CHANGES(int, ch_current_pos, newPosition, 0)


        if (newPosition == ch_target_pos->value.int_value) {
            INFO_("reached destination %u\n", newPosition);
            HAP_NOTIFY_CHANGES(int, ch_position_state, WINDOWCOVERING_POSITION_STATE_STOPPED,0)
            suspend_update_task();
        }
        handle_motor();
#ifdef ESP_TASK
        vTaskDelay(pdMS_TO_TICKS(500));
    }
#endif
}

void notify_hap(int newPos){


}

void hap_callback_process(homekit_characteristic_t *ch, homekit_value_t value, void *context) {
    INFO_("hap_callback \n");
    if(!service_windowcovering || !ch_target_pos || !ch_position_state){
       ERROR_("service/charachteristic are not defined \n");
      return;
   
    }
    if (ch_target_pos->value.int_value == ch_current_pos->value.int_value) {
        INFO_("Current position equal to target. Stopping.\n");
        
        HAP_NOTIFY_CHANGES(int, ch_position_state, WINDOWCOVERING_POSITION_STATE_STOPPED,0)
        
         suspend_update_task();
    } else {
        int newState= ch_target_pos->value.int_value > ch_current_pos->value.int_value
            ? WINDOWCOVERING_POSITION_STATE_OPENING
            : WINDOWCOVERING_POSITION_STATE_CLOSING;
        HAP_NOTIFY_CHANGES(int, ch_position_state, newState,0)
        resume_update_task();
    }
 
   
     

}
