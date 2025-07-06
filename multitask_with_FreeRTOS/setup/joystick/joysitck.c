#include "joystick.h"

void setup_joystick()
{
    adc_init();

    adc_gpio_init(VRX_PIN);

    adc_gpio_init(VRY_PIN);

    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
}