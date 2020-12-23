

#include "servo_control.h"

uint8_t chanel_v=7;
uint8_t chanel_h=8;

void setup_servo(){
  Serial.println("setup_servo_pin");
  setup_servo_pin(SERVO_V_PIN,chanel_v);   //keep servo on previous position
  setup_servo_pin(SERVO_H_PIN,chanel_h);
}
  
void setup_servo_pin(uint8_t pin, uint8_t chanel)
{
    
    ledcSetup(chanel, SERVO_FREQ, SERVO_BITS); // channel , 50 Hz, 
    ledcAttachPin(pin, chanel);   // GPIO  assigned to channel 
   // ledcWrite(chanel, CALC_SERVO_PULSE(90)); //start from the middle  //keep servo on previous position
}
void set_servo_v(uint8_t val){
   Serial.println("set_servo_v");
  ledcWrite(chanel_v, CALC_SERVO_PULSE(val));  
}
void set_servo_h(uint8_t val){
  Serial.println("set_servo_h");
   ledcWrite(chanel_h, CALC_SERVO_PULSE(val));  
}
