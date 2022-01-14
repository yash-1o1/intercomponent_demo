#ifndef H_MQTT_QUEUE
#define H_MQTT_QUEUE

#include <FreeRTOS.h>
#include <queue.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

typedef enum{
    APP_MQTT_PUBLISH,
    APP_MQTT_CON_TOGGLE,
    APP_MQTT_DEINIT,
    APP_BTN_HANDLER,
    APP_MQTT_TIMER70,
    APP_MQTT_TIMER500,
    APP_MQTT_STAT,
    APP_MQTT_CHAIN,
    APP_MQTT_TOPIC_STAT
} mqttMsgType;

typedef struct {
    mqttMsgType msgType;
    int32_t dataValue;
    int32_t dataValue2;
    int32_t dataValue3;
    char*   topic;
} mqttMsg;

void mqttThreadQueueInit();
void mqttReadFromAQueue(mqttMsg* receivedMsg);
void mqttSendMsg(mqttMsg* msg);
void mqttQueueSendTimer70(int value);
void mqttQueueSendTimer500(int sensorAvg, int time);
void mqttQueueSendStatMsg(int totalIR, int totalIRNumber, int averageIR);
void mqttQueueSendChainMsg(int chainNumber, int seq, int value);
void mqttQueueSendTopicStatMsg(int count, int seq, int miss, char* topic);

#endif
