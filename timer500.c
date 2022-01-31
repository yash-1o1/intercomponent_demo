#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include "timer500.h"
#include "debug.h"
#include "sensor_thread_queue.h"

#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define PERIOD500 500000

static Timer_Handle timer500;
static Timer_Params timerParams500;

// Determines the elapsed time since the last time this callback was called using
// the FreeRTOS routines. Converts value to microseconds and sends the value to
// the sensor thread in a C struct that you define. The C struct will contain a
// message type and the elapsed time value.
void timer500Callback(Timer_Handle myHandle, int_fast16_t status) {
    dbgEvent(TIMER_500_ISR_BEGIN);
    static uint32_t lastTick = 0;
    uint32_t currentTick = xTaskGetTickCountFromISR();
    uint32_t elapsedTick = currentTick - lastTick;
    uint32_t elapsedMs = elapsedTick / (configTICK_RATE_HZ / 1000);
    lastTick = currentTick;
    sensorQueueSendFromTimer500ISR(elapsedMs);
    dbgEvent(TIMER_500_ISR_END);
}

void timer500Init() {
    // init of timer 500
    Timer_Params_init(&timerParams500);
    timerParams500.period = PERIOD500;
    timerParams500.periodUnits = Timer_PERIOD_US;
    timerParams500.timerMode = Timer_CONTINUOUS_CALLBACK;
    timerParams500.timerCallback = timer500Callback;
    timer500 = Timer_open(CONFIG_TIMER_3, &timerParams500);
    if (timer500 == NULL) {
        fatalError(ERR_CREATE_TIMER);
    }
}

void startTimer500() {
    if (Timer_start(timer500) == Timer_STATUS_ERROR) {
        fatalError(ERR_START_TIMER);
    }
}
