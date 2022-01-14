/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/SPI.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define GPIO_OFF 0

void Board_initHook(void) {

    SPI_init();
    GPIO_init();
    UART_init();
    Timer_init();
    ADC_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Configure the 8 output pins */
    GPIO_setConfig(CONFIG_GPIO_3, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_4, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_5, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_6, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_7, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_8, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_9, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_10, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);

    /* Turn off all debug pin */
    GPIO_write(CONFIG_GPIO_3, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_4, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_5, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_6, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_7, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_8, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_9, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_10, GPIO_OFF);
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_LED_2, CONFIG_GPIO_LED_OFF);
}
