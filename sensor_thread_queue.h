#ifndef H_SENSOR_THREAD_QUEUE
#define H_SENSOR_THREAD_QUEUE

#include <FreeRTOS.h>
#include <queue.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {TIMER70_MESSAGE, TIMER500_MESSAGE} sensorMsgType;

typedef struct {
    sensorMsgType msgType;
    int32_t dataValue;
} sensorMsg;

void sensorThreadQueueInit();
void sensorReadFromAQueue(sensorMsg* receivedMsg);
void sensorQueueSendFromTimer70ISR(uint32_t distance);
void sensorQueueSendFromTimer500ISR(uint32_t elapsedMs);
#endif
