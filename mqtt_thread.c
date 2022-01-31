/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*****************************************************************************

   Application Name     -   MQTT Client
   Application Overview -   The device is running a MQTT client which is
                           connected to the online broker. Three LEDs on the
                           device can be controlled from a web client by
                           publishing msg on appropriate topics. Similarly,
                           message can be published on pre-configured topics
                           by pressing the switch buttons on the device.

   Application Details  - Refer to 'MQTT Client' README.html

*****************************************************************************/
#include <mqtt_if.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <mqueue.h>

#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/slnetifwifi.h>

#include <ti/drivers/SPI.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

#include <ti/net/mqtt/mqttclient.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "wifi.h"
#include "network_if.h"
#include "uart_term.h"
#include "mqtt_if.h"
#include "debug_if.h"
#include "mqtt_queue.h"
#include "sensor_stat_queue.h"
#include "common.h"

#include "ti_drivers_config.h"

#include "jsmn.h"

#define NUM_TOKEN 10
jsmn_parser parser;
jsmntok_t tokens[NUM_TOKEN];
// un-comment this if you want to connect to an MQTT broker securely
//#define MQTT_SECURE_CLIENT

#define MQTT_MODULE_TASK_PRIORITY   2
#define MQTT_MODULE_TASK_STACK_SIZE 2048

#define MQTT_WILL_TOPIC             "jesus_cc32xx_will_topic"
#define MQTT_WILL_MSG               "will_msg_works"
#define MQTT_WILL_QOS               MQTT_QOS_0
#define MQTT_WILL_RETAIN            false

#define MQTT_CLIENT_PASSWORD        "dZ4zwI5V14BH"
#define MQTT_CLIENT_USERNAME        "wxxzbmgb"
#define MQTT_CLIENT_KEEPALIVE       0
#define MQTT_CLIENT_CLEAN_CONNECT   true
#define MQTT_CLIENT_MQTT_V3_1       true
#define MQTT_CLIENT_BLOCKING_SEND   true

#ifndef MQTT_SECURE_CLIENT
#define MQTT_CONNECTION_FLAGS           MQTTCLIENT_NETCONN_URL
#define MQTT_CONNECTION_ADDRESS         "driver.cloudmqtt.com"
#define MQTT_CONNECTION_PORT_NUMBER     18771
#else
#define MQTT_CONNECTION_FLAGS           MQTTCLIENT_NETCONN_IP4 | MQTTCLIENT_NETCONN_SEC
#define MQTT_CONNECTION_ADDRESS         "192.168.178.67"
#define MQTT_CONNECTION_PORT_NUMBER     8883
#endif

#if (BOARD_NAME == CHAIN1)
    char ClientId[] = {"Chain1"};
#elif (BOARD_NAME == CHAIN2)
    char ClientId[] = {"Chain2"};
#elif (BOARD_NAME == CHAIN3)
    char ClientId[] = {"Chain3"};
#elif (BOARD_NAME == CHAIN4)
    char ClientId[] = {"Chain4"};
#endif

int connected;
int deinit;
int longPress = 0;

struct topic
{
    char* topic;
    int publication;
    int sensorValue;
    int sensorAverage;
};

MQTT_IF_InitParams_t mqttInitParams =
{
     MQTT_MODULE_TASK_STACK_SIZE,   // stack size for mqtt module - default is 2048
     MQTT_MODULE_TASK_PRIORITY      // thread priority for MQTT   - default is 2
};

MQTTClient_Will mqttWillParams =
{
     MQTT_WILL_TOPIC,    // will topic
     MQTT_WILL_MSG,      // will message
     MQTT_WILL_QOS,      // will QoS
     MQTT_WILL_RETAIN    // retain flag
};

MQTT_IF_ClientParams_t mqttClientParams =
{
     ClientId,                  // client ID
     MQTT_CLIENT_USERNAME,      // user name
     MQTT_CLIENT_PASSWORD,      // password
     MQTT_CLIENT_KEEPALIVE,     // keep-alive time
     MQTT_CLIENT_CLEAN_CONNECT, // clean connect flag
     MQTT_CLIENT_MQTT_V3_1,     // true = 3.1, false = 3.1.1
     MQTT_CLIENT_BLOCKING_SEND, // blocking send flag
     &mqttWillParams            // will parameters
};

#ifndef MQTT_SECURE_CLIENT
MQTTClient_ConnParams mqttConnParams =
{
     MQTT_CONNECTION_FLAGS,         // connection flags
     MQTT_CONNECTION_ADDRESS,       // server address
     MQTT_CONNECTION_PORT_NUMBER,   // port number of MQTT server
     0,                             // method for secure socket
     0,                             // cipher for secure socket
     0,                             // number of files for secure connection
     NULL                           // secure files
};
#else
/*
 * In order to connect to an MQTT broker securely, the MQTTCLIENT_NETCONN_SEC flag,
 * method for secure socket, cipher, secure files, number of secure files must be set
 * and the certificates must be programmed to the file system.
 *
 * The first parameter is a bit mask which configures the server address type and security mode.
 * Server address type: IPv4, IPv6 and URL must be declared with the corresponding flag.
 * All flags can be found in mqttclient.h.
 *
 * The flag MQTTCLIENT_NETCONN_SEC enables the security (TLS) which includes domain name
 * verification and certificate catalog verification. Those verifications can be skipped by
 * adding to the bit mask: MQTTCLIENT_NETCONN_SKIP_DOMAIN_NAME_VERIFICATION and
 * MQTTCLIENT_NETCONN_SKIP_CERTIFICATE_CATALOG_VERIFICATION.
 *
 * Note: The domain name verification requires URL Server address type otherwise, this
 * verification will be disabled.
 *
 * Secure clients require time configuration in order to verify the server certificate validity (date)
 */

/* Day of month (DD format) range 1-31                                       */
#define DAY                      1
/* Month (MM format) in the range of 1-12                                    */
#define MONTH                    5
/* Year (YYYY format)                                                        */
#define YEAR                     2020
/* Hours in the range of 0-23                                                */
#define HOUR                     4
/* Minutes in the range of 0-59                                              */
#define MINUTES                  00
/* Seconds in the range of 0-59                                              */
#define SEC                      00

char *MQTTClient_secureFiles[1] = {"ca-cert.pem"};

MQTTClient_ConnParams mqttConnParams =
{
    MQTT_CONNECTION_FLAGS,                  // connection flags
    MQTT_CONNECTION_ADDRESS,                // server address
    MQTT_CONNECTION_PORT_NUMBER,            // port number of MQTT server
    SLNETSOCK_SEC_METHOD_SSLv3_TLSV1_2,     // method for secure socket
    SLNETSOCK_SEC_CIPHER_FULL_LIST,         // cipher for secure socket
    1,                                      // number of files for secure connection
    MQTTClient_secureFiles                  // secure files
};

void setTime(){

    SlDateTime_t dateTime = {0};
    dateTime.tm_day = (uint32_t)DAY;
    dateTime.tm_mon = (uint32_t)MONTH;
    dateTime.tm_year = (uint32_t)YEAR;
    dateTime.tm_hour = (uint32_t)HOUR;
    dateTime.tm_min = (uint32_t)MINUTES;
    dateTime.tm_sec = (uint32_t)SEC;
    sl_DeviceSet(SL_DEVICE_GENERAL, SL_DEVICE_GENERAL_DATE_TIME,
                 sizeof(SlDateTime_t), (uint8_t *)(&dateTime));
}
#endif

int detectLongPress(){

    int buttonPressed;

    do{
        buttonPressed = GPIO_read(CONFIG_GPIO_BUTTON_1);
    }while(buttonPressed && !longPress);

    if(longPress == 1){
        longPress = 0;
        return 1;
    }
    else{
        return 0;
    }
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}


void MQTT_EventCallback(int32_t event){

    mqttMsg queueElement;

    switch(event){

        case MQTT_EVENT_CONNACK:
        {
            deinit = 0;
            connected = 1;
            LOG_INFO("MQTT_EVENT_CONNACK\r\n");
            break;
        }

        case MQTT_EVENT_SUBACK:
        {
            LOG_INFO("MQTT_EVENT_SUBACK\r\n");
            break;
        }

        case MQTT_EVENT_PUBACK:
        {
            LOG_INFO("MQTT_EVENT_PUBACK\r\n");
            break;
        }

        case MQTT_EVENT_UNSUBACK:
        {
            LOG_INFO("MQTT_EVENT_UNSUBACK\r\n");
            break;
        }

        case MQTT_EVENT_CLIENT_DISCONNECT:
        {
            connected = 0;
            LOG_INFO("MQTT_EVENT_CLIENT_DISCONNECT\r\n");
            break;
        }

        case MQTT_EVENT_SERVER_DISCONNECT:
        {
            connected = 0;
            LOG_INFO("MQTT_EVENT_SERVER_DISCONNECT\r\n");
            queueElement.msgType = APP_MQTT_CON_TOGGLE;
            mqttSendMsg(&queueElement);
            break;
        }

        case MQTT_EVENT_DESTROY:
        {
            LOG_INFO("MQTT_EVENT_DESTROY\r\n");
            break;
        }
    }
}

/*
 * Subscribe topic callbacks
 * Topic and payload data is deleted after topic callbacks return.
 * User must copy the topic or payload data if it needs to be saved.
 */
void timer70(char* topic, char* payload){
    if (connected == 0)
        return;
    int i;
    int r;
    char *ptr;
    int sequence;
    int value;
    jsmn_init(&parser);
    r = jsmn_parse(&parser, payload, strlen(payload), tokens, NUM_TOKEN);
    LOG_INFO("TOPIC: %s \tPAYLOAD: %s\r\n", topic, payload);
    if (r < 0) {
        LOG_ERROR("Failed to parse JSON");
        return;
    }
    for (i = 1; i < r; i++) {
        if (jsoneq(payload, &tokens[i], "sequence") == 0) {
            sequence = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
        else if (jsoneq(payload, &tokens[i], "sensor_value") == 0) {
            value = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
    }
    LOG_INFO("sequence: %d, value: %d", sequence, value);
    statQueueSendTimer70(topic, sequence, value);
}

void timer500(char* topic, char* payload){
    if (connected == 0)
        return;
    int i;
    int r;
    char *ptr;
    int sequence;
    int average;
    int time;
    jsmn_init(&parser);
    r = jsmn_parse(&parser, payload, strlen(payload), tokens, NUM_TOKEN);
    LOG_INFO("TOPIC: %s \r\n PAYLOAD: %s\r\n", topic, payload);
    if (r < 0) {
        LOG_ERROR("Failed to parse JSON");
        return;
    }
    for (i = 1; i < r; i++) {
        if (jsoneq(payload, &tokens[i], "sequence") == 0) {
            sequence = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
        else if (jsoneq(payload, &tokens[i], "average") == 0) {
            average = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
        else if (jsoneq(payload, &tokens[i], "time") == 0) {
            time = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
    }
    LOG_INFO("sequence: %d, average: %d, time: %d", sequence, average, time);
    statQueueSendTimer500(topic, sequence, average, time);
}

void chainCB(char* topic, char* payload) {
    if (connected == 0)
        return;
    int i;
    int r;
    char *ptr;
    int sequence;
    int value;
    jsmn_init(&parser);
    r = jsmn_parse(&parser, payload, strlen(payload), tokens, NUM_TOKEN);
    LOG_INFO("TOPIC: %s \r\n PAYLOAD: %s\r\n", topic, payload);
    if (r < 0) {
        LOG_ERROR("Failed to parse JSON");
        return;
    }
    for (i = 1; i < r; i++) {
        if (jsoneq(payload, &tokens[i], "sequence") == 0) {
            sequence = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
        else if (jsoneq(payload, &tokens[i], "value") == 0) {
            value = strtol(payload + tokens[i+1].start, &ptr, 10);
            i++;
        }
    }
    LOG_INFO("sequence: %d, value: %d", sequence, value);
    statQueueSendChain(topic, sequence);
    sequence++;
    int chainNum;
    switch (BOARD_NAME) {
    case CHAIN1:
        chainNum = 2;
        value = value % 10;
        break;
    case CHAIN2:
        chainNum = 3;
        value = value + 6;
        break;
    case CHAIN3:
        chainNum = 4;
        value = value * 3;
        break;
    case CHAIN4:
        chainNum = 1;
        value = value / 2;
        break;
    default:
        break;
    }
    mqttQueueSendChainMsg(chainNum, sequence, value);
}

void BrokerCB(char* topic, char* payload){
    LOG_INFO("TOPIC: %s \tPAYLOAD: %s\r\n", topic, payload);
}

void ToggleLED1CB(char* topic, char* payload){
    GPIO_toggle(CONFIG_GPIO_LED_0);
    LOG_INFO("TOPIC: %s \tPAYLOAD: %s\r\n", topic, payload);
}

void ToggleLED2CB(char* topic, char* payload){
    GPIO_toggle(CONFIG_GPIO_LED_1);
    LOG_INFO("TOPIC: %s \tPAYLOAD: %s\r\n", topic, payload);
}

void ToggleLED3CB(char* topic, char* payload){
    GPIO_toggle(CONFIG_GPIO_LED_2);
    LOG_INFO("TOPIC: %s \tPAYLOAD: %s\r\n", topic, payload);
}

void mqttThread(void * args){

    int32_t ret;
    MQTTClient_Handle mqttClientHandle;
    mqttMsg queueElement;

    #if(BOARD_NAME == CHAIN1)
        struct topic cc32xx_timer70 = {"CHAIN1/Timer70", 1, 0, 0};
        struct topic cc32xx_timer500 = {"CHAIN1/Timer500", 1, 0, 0};
    #elif(BOARD_NAME == CHAIN2)
        struct topic cc32xx_timer70 = {"CHAIN2/Timer70", 1, 0, 0};
        struct topic cc32xx_timer500 = {"CHAIN2/Timer500", 1, 0, 0};
    #elif(BOARD_NAME == CHAIN3)
        struct topic cc32xx_stat = {"CHAIN3/Stat", 1, 0, 0};
    #elif(BOARD_NAME == CHAIN4)
        struct topic cc32xx_stat = {"CHAIN4/Stat", 1, 0, 0};
    #endif
    InitTerm();
    mqttThreadQueueInit();

    ret = wifiInit();
    if(ret < 0){
        LOG_ERROR("Wifi init fail");
    }

MQTT_DEMO:

    ret = MQTT_IF_Init(mqttInitParams);
    if(ret < 0){
        while(1);
    }

#ifdef MQTT_SECURE_CLIENT
    setTime();
#endif

    /*
     * In case a persistent session is being used, subscribe is called before connect so that the module
     * is aware of the topic callbacks the user is using. This is important because if the broker is holding
     * messages for the client, after CONNACK the client may receive the messages before the module is aware
     * of the topic callbacks. The user may still call subscribe after connect but have to be aware of this.
     */
    ret = MQTT_IF_Subscribe(mqttClientHandle, "Broker/To/cc32xx", MQTT_QOS_0, BrokerCB);
    ret |= MQTT_IF_Subscribe(mqttClientHandle, "cc32xx/ToggleLED1", MQTT_QOS_0, ToggleLED1CB);
    ret |= MQTT_IF_Subscribe(mqttClientHandle, "cc32xx/ToggleLED2", MQTT_QOS_0, ToggleLED2CB);
    ret |= MQTT_IF_Subscribe(mqttClientHandle, "cc32xx/ToggleLED3", MQTT_QOS_0, ToggleLED3CB);
    /* Subscribes to topic based on board number */
    #if(BOARD_NAME == CHAIN1)
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN1", MQTT_QOS_0, chainCB);
    #elif(BOARD_NAME == CHAIN2)
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN2", MQTT_QOS_0, chainCB);
    #elif(BOARD_NAME == CHAIN3)
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN1/Timer70", MQTT_QOS_0, timer70);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN2/Timer70", MQTT_QOS_0, timer70);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN1/Timer500", MQTT_QOS_0, timer500);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN2/Timer500", MQTT_QOS_0, timer500);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN3", MQTT_QOS_0, chainCB);
    #elif(BOARD_NAME == CHAIN4)
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN1/Timer70", MQTT_QOS_0, timer70);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN2/Timer70", MQTT_QOS_0, timer70);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN1/Timer500", MQTT_QOS_0, timer500);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN2/Timer500", MQTT_QOS_0, timer500);
        ret |= MQTT_IF_Subscribe(mqttClientHandle, "CHAIN4", MQTT_QOS_0, chainCB);
    #endif
    if(ret < 0){
        while(1);
    }
    else{
        LOG_INFO("Subscribed to all topics successfully\r\n");
    }

    mqttClientHandle = MQTT_IF_Connect(mqttClientParams, mqttConnParams, MQTT_EventCallback);
    if(mqttClientHandle < 0){
        while(1);
    }

    // wait for CONNACK
    while(connected == 0);

    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    while(1){
        mqttReadFromAQueue(&queueElement);
#if (BOARD_NAME == CHAIN1 || BOARD_NAME == CHAIN2)
        if(queueElement.msgType == APP_MQTT_TIMER70) {
            LOG_TRACE("APP_MQTT_PUBLISH TIMER70 \r\n");
            /* Constructs JSON object payload */
            int buffer = 100;
            char payload[100] = "";
            snprintf(payload,
                     buffer,
                     "{\"sequence\": %d, "
                     " \"sensor_value\" : \"%d\"}",
                     cc32xx_timer70.publication,
                     queueElement.dataValue);

            /* Publishes JSON object to AirMQTT */
            MQTT_IF_Publish(mqttClientHandle,
                            cc32xx_timer70.topic,
                            payload,
                            strlen(payload),
                            MQTT_QOS_0);

            /* Increments publication value for topic */
            cc32xx_timer70.publication = cc32xx_timer70.publication + 1;
        }
        else if(queueElement.msgType == APP_MQTT_TIMER500) {
            LOG_TRACE("APP_MQTT_PUBLISH TIMER500 \r\n");
            /* Constructs JSON object payload */
            int buffer = 100;
            char payload[100] = "";
            snprintf(payload,
                     buffer,
                     "{\"sequence\" : \"%d\","
                     " \"average\" : \"%d\", "
                     "\"time\" : \"%d\"}",
                     cc32xx_timer500.publication,
                     queueElement.dataValue,
                     queueElement.dataValue2);

            /* Publishes JSON object to AirMQTT */
            MQTT_IF_Publish(mqttClientHandle,
                            cc32xx_timer500.topic,
                            payload,
                            strlen(payload),
                            MQTT_QOS_0);

            /* Increments publication value for topic */
            cc32xx_timer500.publication = cc32xx_timer500.publication + 1;
        }
#else
        if(queueElement.msgType == APP_MQTT_STAT) {
            LOG_TRACE("APP_MQTT_PUBLISH STAT \r\n");
            /* Constructs JSON object payload */
            int buffer = 100;
            char payload[100] = "";
            snprintf(payload,
                     buffer,
                     "{\"sequence\" : \"%d\","
                     " \"totalIR\" : \"%d\", "
                     " \"totalIRNumber\" : \"%d\", "
                     "\"averageIR\" : \"%d\"}",
                     cc32xx_stat.publication,
                     queueElement.dataValue,
                     queueElement.dataValue2,
                     queueElement.dataValue3);

            /* Publishes JSON object to AirMQTT */
            MQTT_IF_Publish(mqttClientHandle,
                            cc32xx_stat.topic,
                            payload,
                            strlen(payload),
                            MQTT_QOS_0);

            /* Increments publication value for topic */
            cc32xx_stat.publication += 1;
        }
#endif
        else if(queueElement.msgType == APP_MQTT_CHAIN) {
            LOG_TRACE("APP_MQTT_PUBLISH CHAIN \r\n");
            /* Constructs JSON object payload */
            int buffer = 100;
            char payload[100] = "";
            snprintf(payload,
                     buffer,
                     "{\"sequence\" : \"%d\","
                     "\"value\" : \"%d\"}",
                     queueElement.dataValue2,
                     queueElement.dataValue3);
            char topic[10];
            snprintf(topic, 10, "CHAIN%d", queueElement.dataValue);
            /* Publishes JSON object to AirMQTT */
            MQTT_IF_Publish(mqttClientHandle,
                            topic,
                            payload,
                            strlen(payload),
                            MQTT_QOS_0);
        }
        else if(queueElement.msgType == APP_MQTT_TOPIC_STAT) {
            LOG_TRACE("APP_MQTT_PUBLISH CHAIN \r\n");
            int buffer = 100;
            char payload[100] = "";
            snprintf(payload,
                     buffer,
                     "{\"topic\" : \"%s\","
                     "count\" : \"%d\","
                     "sequence\" : \"%d\","
                     "\"miss\" : \"%d\"}",
                     queueElement.topic,
                     queueElement.dataValue,
                     queueElement.dataValue2,
                     queueElement.dataValue3);
            char topic[10];
            snprintf(topic, 10, "BOARD%d", BOARD_NAME);
            /* Publishes JSON object to AirMQTT */
            MQTT_IF_Publish(mqttClientHandle,
                            topic,
                            payload,
                            strlen(payload),
                            MQTT_QOS_0);
        }
        else if(queueElement.msgType == APP_MQTT_CON_TOGGLE){

            LOG_TRACE("APP_MQTT_CON_TOGGLE %d\r\n", connected);


            if(connected){
                ret = MQTT_IF_Disconnect(mqttClientHandle);
                if(ret >= 0){
                    connected = 0;
                }
            }
            else{
                mqttClientHandle = MQTT_IF_Connect(mqttClientParams, mqttConnParams, MQTT_EventCallback);
                if((int)mqttClientHandle >= 0){
                    connected = 1;
                }
            }
        }
        else if(queueElement.msgType == APP_MQTT_DEINIT){
            break;
        }
        else if(queueElement.msgType == APP_BTN_HANDLER){
            break;
        }
    }

    deinit = 1;
    if(connected){
        MQTT_IF_Disconnect(mqttClientHandle);
    }
    MQTT_IF_Deinit();

    LOG_INFO("looping the MQTT functionality of the example for demonstration purposes only\r\n");
    sleep(2);
    goto MQTT_DEMO;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
