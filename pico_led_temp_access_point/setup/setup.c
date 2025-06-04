#include "led/led.h"
#include "temperature_sensor/temperature_sensor.h"

void setup()
{
    stdio_init_all();

    setup_led();

    setup_temperature_sensor();
}