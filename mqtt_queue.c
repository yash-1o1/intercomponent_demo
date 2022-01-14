#include "mqtt_queue.h"
#include "debug.h"

static QueueHandle_t mqttTaskQueue;
#define MQTT_QUEUE_SIZE 10

void mqttThreadQueueInit() {
    mqttTaskQueue = xQueueCreate(MQTT_QUEUE_SIZE, sizeof(mqttMsg));
    if (mqttTaskQueue == NULL) {
        dbgEvent(ERR_MQTT_TASK_QUEUE_CREATE);
    }
}

void mqttReadFromAQueue(mqttMsg* receivedMsg) {
    if (mqttTaskQueue == NULL)
        return;
    BaseType_t xStatus;
    xStatus = xQueueReceive(mqttTaskQueue, receivedMsg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void mqttSendMsg(mqttMsg* msg) {
    if (mqttTaskQueue == NULL)
        return;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, 0);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void mqttQueueSendTimer70(int value) {
    if (mqttTaskQueue == NULL)
        return;
    mqttMsg msg;
    msg.msgType = APP_MQTT_TIMER70;
    msg.dataValue = value;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}
void mqttQueueSendTimer500(int sensorAvg, int time) {
    if (mqttTaskQueue == NULL)
        return;
    mqttMsg msg;
    msg.msgType = APP_MQTT_TIMER500;
    msg.dataValue = sensorAvg;
    msg.dataValue2 = time;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void mqttQueueSendStatMsg(int totalIR, int totalIRNumber, int averageIR) {
    if (mqttTaskQueue == NULL)
        return;
    mqttMsg msg;
    msg.msgType = APP_MQTT_STAT;
    msg.dataValue = totalIR;
    msg.dataValue2 = totalIRNumber;
    msg.dataValue3 = averageIR;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void mqttQueueSendChainMsg(int chainNumber, int seq, int value) {
    if (mqttTaskQueue == NULL)
        return;
    mqttMsg msg;
    msg.msgType = APP_MQTT_CHAIN;
    msg.dataValue = chainNumber;
    msg.dataValue2 = seq;
    msg.dataValue3 = value;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}

void mqttQueueSendTopicStatMsg(int count, int seq, int miss, char* topic) {
    if (mqttTaskQueue == NULL)
        return;
    mqttMsg msg;
    msg.msgType = APP_MQTT_TOPIC_STAT;
    msg.dataValue = count;
    msg.dataValue2 = seq;
    msg.dataValue3 = miss;
    msg.topic      = topic;
    BaseType_t xStatus;
    xStatus = xQueueSendToBack(mqttTaskQueue, &msg, portMAX_DELAY);
    if (xStatus != pdPASS) {
        dbgEvent(ERR_SENSOR_TASK_QUEUE_MSG);
    }
}
