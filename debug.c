#include <stdint.h>
#include <stdbool.h>
#include "debug.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>

#include <FreeRTOS.h>
#include <task.h>

#define GPIO_ON 1
#define GPIO_OFF 0
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40

bool checkBitIsOne(uint8_t uBitData, uint8_t uBitFlag) {
    if (0x00 == uBitData) {
        return false;
    }
    else if (uBitData == uBitFlag) {
        return true;
    }
    return false;
}

void dbgEvent(unsigned int event) {
    // Start Update GPIO
    return;
    GPIO_write(CONFIG_GPIO_10, GPIO_ON);
    GPIO_write(CONFIG_GPIO_3, checkBitIsOne(event & BIT0, BIT0));
    GPIO_write(CONFIG_GPIO_4, checkBitIsOne(event & BIT1, BIT1));
    GPIO_write(CONFIG_GPIO_5, checkBitIsOne(event & BIT2, BIT2));
    GPIO_write(CONFIG_GPIO_6, checkBitIsOne(event & BIT3, BIT3));
    GPIO_write(CONFIG_GPIO_7, checkBitIsOne(event & BIT4, BIT4));
    GPIO_write(CONFIG_GPIO_8, checkBitIsOne(event & BIT5, BIT5));
    GPIO_write(CONFIG_GPIO_9, checkBitIsOne(event & BIT6, BIT6));
    GPIO_write(CONFIG_GPIO_10, GPIO_OFF);
}

void fatalError(unsigned int event) {
    portDISABLE_INTERRUPTS();
    vTaskSuspendAll();
    GPIO_write(CONFIG_GPIO_10, GPIO_ON);
    GPIO_write(CONFIG_GPIO_3, checkBitIsOne(event & BIT0, BIT0));
    GPIO_write(CONFIG_GPIO_4, checkBitIsOne(event & BIT1, BIT1));
    GPIO_write(CONFIG_GPIO_5, checkBitIsOne(event & BIT2, BIT2));
    GPIO_write(CONFIG_GPIO_6, checkBitIsOne(event & BIT3, BIT3));
    GPIO_write(CONFIG_GPIO_7, checkBitIsOne(event & BIT4, BIT4));
    GPIO_write(CONFIG_GPIO_8, checkBitIsOne(event & BIT5, BIT5));
    GPIO_write(CONFIG_GPIO_9, checkBitIsOne(event & BIT6, BIT6));
    GPIO_write(CONFIG_GPIO_10, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_10, GPIO_ON);
    GPIO_write(CONFIG_GPIO_10, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
    while (1) {};
}

