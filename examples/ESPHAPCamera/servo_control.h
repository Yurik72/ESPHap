#ifndef __SERVO_CONTROL_H
#define __SERVO_CONTROL_H

#include <Arduino.h>


#define MIN_PULSE_WIDTH       1500     // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH      8500     // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define MIN_SERVO_VAL 0
#define MAX_SERVO_VAL 180
#define SERVO_BITS 16
#define SERVO_FREQ 50

#define CALC_SERVO_PULSE(val) map(val,MIN_SERVO_VAL,MAX_SERVO_VAL,MIN_PULSE_WIDTH,MAX_PULSE_WIDTH)
#include "config.h"

#define SERVO_V_PIN 12
#define SERVO_H_PIN 13
extern uint8_t chanel_v;
extern uint8_t chanel_h;

void setup_servo();
  
void setup_servo_pin(uint8_t pin, uint8_t chanel);
void set_servo_v(uint8_t val);
void set_servo_h(uint8_t val);

#endif
