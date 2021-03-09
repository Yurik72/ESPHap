#ifndef SIMPLESENSOR_H
#define SIMPLESENSOR_H
#include "Arduino.h"

typedef void(*sensor_callback_fn)(uint8_t gpio_num, uint8_t state);

class SimpleSensor
{
  public:
    SimpleSensor(uint8_t pin);
    void start(sensor_callback_fn fn);
    uint8_t getstate();
  private:
    static void   interruptInternal(void* arg);
    sensor_callback_fn callback;

    void interrupt();

    uint8_t m_gpio;
};



#endif
