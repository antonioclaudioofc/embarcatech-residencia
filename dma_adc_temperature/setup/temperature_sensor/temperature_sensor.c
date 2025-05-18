#include "temperature_sensor.h"
#include "hardware/adc.h"

void setup_temperature_sensor()
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(ADC_CHANNEL);
}