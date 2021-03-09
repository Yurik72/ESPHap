#include "simplesensor.h"

SimpleSensor::SimpleSensor(uint8_t pin) {
  m_gpio = pin;
}
void SimpleSensor::start(sensor_callback_fn fn) {
  callback = fn;
  pinMode(m_gpio, INPUT_PULLUP);
  attachInterruptArg(m_gpio, SimpleSensor::interruptInternal, (void*)this, CHANGE);

}
void IRAM_ATTR SimpleSensor::interruptInternal(void* arg) {
  SimpleSensor*p = static_cast<SimpleSensor*>(arg);
  p->interrupt();

}
void SimpleSensor::interrupt() {
  if (callback) {
    callback(m_gpio, getstate());
  }
}
uint8_t SimpleSensor::getstate() {
  return digitalRead(m_gpio);
}
