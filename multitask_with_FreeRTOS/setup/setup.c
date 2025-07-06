#include "setup/setup.h"

void setup()
{
    stdio_init_all();
    sleep_ms(5000);

    setup_led();

    setup_button();

    setup_buzzer();

    setup_joystick();

    setup_microphone();
}