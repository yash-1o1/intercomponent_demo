/*
 * Sensor task
 */

#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include "sensor_thread_state.h"
#include "sensor_thread_queue.h"
#include "timer70.h"
#include "timer500.h"
#include "debug.h"
#include "adc.h"


/* FreeRTOS include */
#include <FreeRTOS.h>
#include <queue.h>

void *sensorThread(void *arg0) {
    sensorMsg receivedMsg;
    dbgEvent(SENSOR_TASK_ENTER);
    adcInit();
    timer70Init();
    timer500Init();
    sensorThreadQueueInit();
    startTimer70();
    startTimer500();
    dbgEvent(SENSOR_TASK_ENTER_WHILE);
    while (1) {
        dbgEvent(SENSOR_TASK_SR_QUEUE_BEGIN);
        // Makes one subroutine call to receive a message from the "sensor_thread_queue" message queue
        sensorReadFromAQueue(&receivedMsg);
        dbgEvent(SENSOR_TASK_SR_QUEUE_BEGIN);
        sensorThreadStateMachine(&receivedMsg);
        dbgEvent(SENSOR_TASK_SR_QUEUE_END);
    }
}



