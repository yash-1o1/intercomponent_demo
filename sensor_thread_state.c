#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <queue.h>

#include "sensor_thread_state.h"
#include "debug.h"

static uint32_t sensorTotal, sensorCount;
static sensorThreadState sensorState = INIT_AVERAGE;

/* Sensor Thread FSM */
void sensorThreadStateMachine(sensorMsg* msg) {
     switch (sensorState) {
     // Initializes static variables sensorTotal and sensorCount to zero
     case INIT_AVERAGE:
         dbgEvent(SENSOR_FSM_INIT);
         sensorTotal = 0;
         sensorCount = 0;
         // Transitions to UPDATE_AVERAGE if received message is "TIMER500_MESSAGE"
         if (msg->msgType == TIMER500_MESSAGE) {
             sensorState = UPDATE_AVERAGE;
         }
         break;
     case UPDATE_AVERAGE:
         dbgEvent(SENSOR_FSM_AVERAGE);
         // If the message received is TIMER70_MESSAGE:
         // sensorTotal and sensorCount variables should be updated if the sensor value received is valid
         // The subroutine should generate a C-string containing "Sensor=" followed by the sensor value received
         // and the sensorCount (in mm). The subroutine then sends the C-string to the uart thread and the
         // uart_thread_queue message queue. The state remains UPDATE_AVERAGE.
         if (msg->msgType == TIMER70_MESSAGE) {
             // if sensor message is valid
             // dataValue should between 40 to 300 according to data sheet
             if (msg->dataValue != -1) {
                 // Update the sensorTotal and sensorCount variables
                 sensorTotal += msg->dataValue;
                 sensorCount += 1;
             }
             mqttQueueSendTimer70(msg->dataValue);
         }
         // If the message received is TIMER500_MESSAGE:
         // The subroutine should compute the average sensor value. Then the subroutine should
         // generate a C-string containing 'Sensor Avg = X; Time = Y" where X is the average sensor value
         // and Y is the elapsed time received in the message. The subroutine should then send this C-string
         // to the uart_thread in the uart_thread_queue message queue. The state should change to INIT_AVERAGE.
         else if (msg->msgType == TIMER500_MESSAGE) {
             if (sensorCount == 0) {
                 mqttQueueSendTimer500(-1, -1);
             }
             else {
                 uint32_t averageSensorValue = sensorTotal / sensorCount;
                 mqttQueueSendTimer500(averageSensorValue, msg->dataValue);
             }

         }
         else {
             fatalError(ERR_SENSOR_MSG_TYPE_OUT_OF_RANGE);
         }
         break;
     default:
         fatalError(ERR_FSM_SWITCH_OUT_OF_RANGE);
     }
}
