#pragma once

#include "Arduino.h"
#include "esp_camera.h"
typedef void(*callback_motiondetected_t)(bool isDetected,long level);
void set_motioncallback(callback_motiondetected_t f);


extern int8_t motion_detection_enabled;

uint8_t run_motion_detection();