#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "timer70.h"
#include "adc.h"
#include "debug.h"
#include "sensor_thread_queue.h"

#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define PERIOD70 70000

static Timer_Handle timer70;
static Timer_Params timerParams70;

#define IR_SENSOR_RATIO 60000000000
#define IR_SENSOR_POWER_RATIO -1.387

// y = 1.3364x^1.0667

// Reads the IR sensor and sends a message to the sensor_thread in the sensor_thread_queue.
// The message will be a C string that you define. The C struct will contains a message type
// and the analog sensor value read converted to mm.
void timer70Callback(Timer_Handle myHandle, int_fast16_t status) {
    dbgEvent(TIMER_70_ISR_BEGIN);
    uint32_t adcValue;
    adcValue = convertADC();
    // TODO: convert adc value to mm for IR sensor
    uint32_t distance = (IR_SENSOR_RATIO*pow(adcValue, IR_SENSOR_POWER_RATIO));
    sensorQueueSendFromTimer70ISR(distance);
    dbgEvent(TIMER_70_ISR_END);
}

void timer70Init() {
    // init of timer 70
    Timer_Params_init(&timerParams70);
    timerParams70.period = PERIOD70;
    timerParams70.periodUnits = Timer_PERIOD_US;
    timerParams70.timerMode = Timer_CONTINUOUS_CALLBACK;
    timerParams70.timerCallback = timer70Callback;
    timer70 = Timer_open(CONFIG_TIMER_0, &timerParams70);
    if (timer70 == NULL) {
        fatalError(ERR_CREATE_TIMER);
    }
}

void startTimer70() {
    if (Timer_start(timer70) == Timer_STATUS_ERROR) {
        fatalError(ERR_START_TIMER);
    }
}

