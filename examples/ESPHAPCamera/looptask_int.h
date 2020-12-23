#ifndef __LOOP_TASK_INT_H
#define __LOOP_TASK_INT_H
#define TASK_PARAM 0
#define TASK_PRIORITY 1
#define TASK_CORE 1  //will run on core 0, usually core 1 used by wifi

#include "FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "config.h"

#define SEMAPHORE_DELAY_TICK 100000

extern TaskHandle_t taskhandle;

extern SemaphoreHandle_t sem_intloop;


static void runcore(void*param);
void setup_internal_loop();
static void loop_camera();
static void check_wifi_connection();


class sem_guard{
  public:
  sem_guard();
  ~sem_guard();
};

#endif
