#include "display/display.h"
#include "temperature_sensor/temperature_sensor.h"

void setup()
{
    stdio_init_all();

    setup_display();

    setup_temperature_sensor();
}