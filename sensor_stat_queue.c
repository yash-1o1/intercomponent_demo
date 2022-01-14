/*
 * sensor_stat_queue.c
 *
 *  Created on: jan 12, 2022
 *      Author: Yash
 */

#include "sensor_stat_queue.h"
#include <stdio.h>

static QueueHandle_t statTaskQueue;
#define STAT_QUEUE_SIZE 3

void statThreadQueueInit() {
    statTaskQueue = xQueueCreate(STAT_QUEUE_SIZE, sizeof(statMsg));
    if (statTaskQueue == NULL) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_CREATE);
    }
}

void statThreadReadFromQueue(statMsg* msg) {
    if (statTaskQueue == NULL)
        return;
    BaseType_t xStatus;
    xStatus = xQueueReceive(statTaskQueue, msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        fatalError(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void statQueueSendTimer70(char* topic, uint32_t sequence, int32_t value) {
    if (statTaskQueue == NULL)
        return;
    statMsg msg;
    msg.msgType = TIMER70;
    msg.dataValue1 = value;
    msg.sequence = sequence;
    snprintf(msg.topic, MAX_TOPIC_LENGTH, "%s", topic);
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(statTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void statQueueSendTimer500(char* topic, uint32_t sequence, int32_t average, int32_t time) {
    if (statTaskQueue == NULL)
        return;
    statMsg msg;
    msg.msgType = TIMER500;
    msg.dataValue1 = average;
    msg.dataValue2 = time;
    msg.sequence = sequence;
    snprintf(msg.topic, MAX_TOPIC_LENGTH, "%s", topic);
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(statTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void statQueueSendChain(char* topic, uint32_t sequence) {
    if (statTaskQueue == NULL)
        return;
    statMsg msg;
    msg.msgType = CHAIN;
    msg.sequence = sequence;
    snprintf(msg.topic, MAX_TOPIC_LENGTH, "%s", topic);
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(statTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

