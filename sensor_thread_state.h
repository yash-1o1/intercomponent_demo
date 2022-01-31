#ifndef H_SENSOR_THREAD_STATE
#define H_SENSOR_THERAD_STATE

#include "sensor_thread_queue.h"
#include "mqtt_queue.h"

typedef enum {INIT_AVERAGE, UPDATE_AVERAGE} sensorThreadState;
void sensorThreadStateMachine(sensorMsg* msg);

#endif
