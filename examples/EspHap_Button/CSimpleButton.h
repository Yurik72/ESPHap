#pragma once
#include "Arduino.h"
#define LONG_PRESS_TIME  10000
#define DEBOUNCE_TIME 50
#define BTN_QUEUE_SIZE 10

#if !defined(ESP32) 

#define portTICK_PERIOD_MS 1
#include "Array.h"
#endif
#if defined(ESP32)
#define IRAM_ATTR
#endif 
typedef enum {
	button_event_single_press,
	button_event_long_press,
} button_event_t;

typedef void(*button_callback_fn)(uint8_t gpio_num, button_event_t event);

typedef struct {
	bool ispress;
	long time;
} button_queue_item_t;

class CSimpleButton {
public:
	CSimpleButton();
	void Create(uint8_t gpio, button_callback_fn cb, bool press_val=false,bool useloop=false);
	void HandleLoop();
protected:
	void Interrupt();
	static void interruptInternal(void* arg);
	static void interruptInternal_noISR(void* arg);
	static void interruptInternal_noarg();
	uint8_t gpio_num;
	button_callback_fn callback;

	uint16_t debounce_time;
	uint16_t long_press_time;

	bool pressed_value;

	uint32_t last_press_time;
	uint32_t last_event_time;
private:

#if defined(ESP32) 
	esp_timer_handle_t _timer;

  QueueHandle_t  messsage_queue  ;
	SemaphoreHandle_t xSemaphore;
#else
  CSimpleArray<button_queue_item_t> m_queue;
  bool useInterrupt;
#endif 
	bool isUseLoop;
	void InterruptTick();
	bool state;
};
