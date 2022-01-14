#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include "adc.h"
#include "debug.h"

#include <ti/drivers/ADC.h>

/* Driver configuration */
#include "ti_drivers_config.h"

static ADC_Handle adc0;
static ADC_Params adcParams;

#define ADC_RAW_RANGE 139
#define ADC_ACTUAL_RANGE 330

void adcInit() {
    // init of ADC
    ADC_Params_init(&adcParams);
    adcParams.isProtected = true;
    adc0 = ADC_open(CONFIG_ADC_0, &adcParams);
    if (adc0 == NULL) {
        // if ADC_open() failed
        fatalError(ERR_START_ADC);
    }
}

uint32_t convertADC() {
    int_fast16_t res;
    uint16_t adcValue;
    uint32_t adcValueUv = 0;
    res = ADC_convert(adc0, &adcValue);
    if (res == ADC_STATUS_SUCCESS) {
        adcValueUv = ADC_convertToMicroVolts(adc0, adcValue);
        adcValueUv = adcValueUv / ADC_RAW_RANGE * ADC_ACTUAL_RANGE;
    }
    return adcValueUv;
}
