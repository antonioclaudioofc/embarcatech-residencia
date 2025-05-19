#include "led/led.h"
#include "buzzer/buzzer.h"
#include "display/display.h"
#include "access_point/access_point.h"

void setup()
{
    stdio_init_all();

    setup_access_point();

    setup_led();

    setup_buzzer();

    setup_display();
}