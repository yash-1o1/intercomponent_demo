/*
 * sensor_stat_queue.h
 *
 *  Created on: jan 12, 2022
 *      Author: Yash
 */

#ifndef SENSOR_STAT_QUEUE_H_
#define SENSOR_STAT_QUEUE_H_

#include <FreeRTOS.h>
#include <queue.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include "debug.h"

#define MAX_TOPIC_LENGTH 20
typedef enum {TIMER70, TIMER500, CHAIN} statMsgType;

typedef struct {
    statMsgType msgType;
    char    topic[MAX_TOPIC_LENGTH];
    uint32_t sequence;
    int32_t dataValue1;
    int32_t dataValue2;
} statMsg;

void statThreadQueueInit();
void statThreadReadFromQueue(statMsg* msg);
void statQueueSendTimer70(char* topic, uint32_t sequence, int32_t value);
void statQueueSendTimer500(char* topic, uint32_t sequence, int32_t average, int32_t time);
void statQueueSendChain(char* topic, uint32_t sequence);

#endif /* SENSOR_STAT_QUEUE_H_ */
