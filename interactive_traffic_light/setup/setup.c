#include "button/button.h"
#include "led/led.h"
#include "buzzer/buzzer.h"
#include "display/display.h"

void setup()
{
    stdio_init_all();

    setup_button();

    setup_led();

    setup_buzzer();

    setup_display();
}