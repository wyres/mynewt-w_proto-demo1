
#ifndef H_ADC_H
#define H_ADC_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include "gpio.h"

#define ADC_TEMPSENSOR_DELAY_US         10U
  #define ADC_ENABLE_TIMEOUT            ( 2U)
#define ADC_DISABLE_TIMEOUT             2
#define ADC_STAB_DELAY_US               ((uint32_t) 1U)


/*
*!
 * ADC object type definition
 */
typedef struct
{
    Gpio_t AdcInput;
}Adc_t;

uint16_t BoardBatteryMeasureVolage(void);

#ifdef __cplusplus
}
#endif

#endif  /* H_ADC_H */
