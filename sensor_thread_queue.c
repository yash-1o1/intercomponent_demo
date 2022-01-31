#include "sensor_thread_queue.h"
#include "debug.h"

static QueueHandle_t sensorTaskQueue;
#define SENSOR_QUEUE_SIZE 3
#define IR_SENSOR_MIN_DISTANCE 40
#define IR_SENSOR_MAX_DISTANCE 300

void sensorThreadQueueInit() {
    // Initiate sensor task queue
    sensorTaskQueue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(sensorMsg));
    if (sensorTaskQueue == NULL) {
        fatalError(ERR_SENSOR_TASK_QUEUE_CREATE);
    }
}

void sensorReadFromAQueue(sensorMsg* receivedMsg) {
    if (sensorTaskQueue == NULL)
        return;
    BaseType_t xStatus;
    xStatus = xQueueReceive(sensorTaskQueue, receivedMsg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        fatalError(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void sensorQueueSendFromTimer70ISR(uint32_t distance) {
    if (sensorTaskQueue == NULL)
        return;
    sensorMsg msg;
    msg.msgType = TIMER70_MESSAGE;
    if (distance < IR_SENSOR_MIN_DISTANCE || distance > IR_SENSOR_MAX_DISTANCE) {
        msg.dataValue = -1;
    }
    else {
        msg.dataValue = distance;
    }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xStatus;
    dbgEvent(TIMER_70_QUEUE_SR_BEGIN);
    xStatus = xQueueSendToBackFromISR(sensorTaskQueue, &msg, &xHigherPriorityTaskWoken);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
    dbgEvent(TIMER_70_QUEUE_SR_END);
}

void sensorQueueSendFromTimer500ISR(uint32_t elapsedMs) {
    if (sensorTaskQueue == NULL)
        return;
    sensorMsg msg;
    msg.msgType = TIMER500_MESSAGE;
    msg.dataValue = elapsedMs;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xStatus;
    dbgEvent(TIMER_500_QUEUE_SR_BEGIN);
    xStatus = xQueueSendToBackFromISR(sensorTaskQueue, &msg, &xHigherPriorityTaskWoken);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
    dbgEvent(TIMER_500_QUEUE_SR_END);
}
