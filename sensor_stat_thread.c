
#include <FreeRTOS.h>
#include <timers.h>
#include "sensor_stat_queue.h"
#include "mqtt_queue.h"
#include <string.h>
#include "debug_if.h"
#include <stdio.h>
#include "common.h"

#define MAX_TOPIC 5
#define MAX_TOPIC_LENGTH 20
static int numberOfIRMsg = 0;
static int totalIRReading = 0;
static int averageIRReading = 0;
static int msgCount = 0;

typedef struct
{
    char topic[MAX_TOPIC_LENGTH];
    uint32_t msgCount;
    uint32_t latestSeq;
} trackingInfo;

static trackingInfo infoList[MAX_TOPIC];

void vTimerCallback(TimerHandle_t xTimer) {
    mqttQueueSendStatMsg(totalIRReading, numberOfIRMsg, averageIRReading / msgCount);
    numberOfIRMsg = 0;
    totalIRReading = 0;
    averageIRReading = 0;
    msgCount = 0;
}

void statCallback(TimerHandle_t xTimer) {
    int i;
    for (i = 0; i < MAX_TOPIC; i++) {
        if (strlen(infoList[i].topic) != 0) {
            int miss = infoList[i].latestSeq - infoList[i].msgCount;
            mqttQueueSendTopicStatMsg(infoList[i].msgCount, infoList[i].latestSeq, miss, infoList[i].topic);
        }
    }
}


void *statThread(void *arg0) {
    statMsg msg;
    TimerHandle_t sensorTimer;
    TimerHandle_t statTimer;
    dbgEvent(SENSOR_TASK_ENTER);
    memset(&infoList, 0, sizeof(infoList));
    statThreadQueueInit();
    if (BOARD_NAME == CHAIN3 || BOARD_NAME == CHAIN4) {
        sensorTimer = xTimerCreate("statTimer", 1000, pdTRUE, (void*) 0, vTimerCallback);
    }
    statTimer = xTimerCreate("statTimer", 5000, pdTRUE, (void*) 1, statCallback);
    if (sensorTimer != NULL) {
        if (xTimerStart(sensorTimer, 0) != pdTRUE) {
            dbgEvent(ERR_CREATE_TIMER);
        }
    }
    if (statTimer != NULL) {
        if (xTimerStart(statTimer, 0) != pdTRUE) {
            dbgEvent(ERR_CREATE_TIMER);
        }
    }
    dbgEvent(SENSOR_TASK_ENTER_WHILE);
    while (1) {
        dbgEvent(SENSOR_TASK_SR_QUEUE_BEGIN);
        // Makes one subroutine call to receive a message from the "sensor_thread_queue" message queue
        statThreadReadFromQueue(&msg);
        dbgEvent(SENSOR_TASK_SR_QUEUE_BEGIN);
        int status = 0;
        int i;
        for (i = 0; i < MAX_TOPIC; i++) {
            if (strlen(infoList[i].topic) == 0) {
                snprintf(infoList[i].topic, MAX_TOPIC_LENGTH, "%s", msg.topic);
                status = 1;
                break;
            }
            else if (strcmp(infoList[i].topic, msg.topic) == 0) {
                status = 1;
                break;
            }
        }
        if (status == 0) {
            fatalError(1);
        }
        status = 0;
        for (i = 0; i < MAX_TOPIC; i++) {
            if (strcmp(infoList[i].topic, msg.topic) == 0) {
                infoList[i].msgCount = infoList[i].msgCount + 1;
                infoList[i].latestSeq = msg.sequence;
                LOG_INFO("topic: %s, msgCount: %d, sequence: %d", infoList[i].topic, infoList[i].msgCount, infoList[i].latestSeq);
                break;
            }
        }
        if (msg.msgType == TIMER70) {
            numberOfIRMsg++;
            totalIRReading += msg.dataValue1;
        }
        else if (msg.msgType == TIMER500) {
            averageIRReading += msg.dataValue1;
            msgCount++;
        }
        dbgEvent(SENSOR_TASK_SR_QUEUE_END);
    }
}


