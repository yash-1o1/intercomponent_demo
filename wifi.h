#ifndef WIFI_H
#define WIFI_H
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/net/wifi/slnetifwifi.h>
#include <ti/net/slnet.h>
#include <pthread.h>
#include "network_if.h"
#include "debug_if.h"

int32_t wifiInit();

#endif
