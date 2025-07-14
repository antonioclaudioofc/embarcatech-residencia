#include "led.h"
#include "pico/stdlib.h"
#include "general_config.h"

void setup_led()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
}

void led_on(void)
{
    gpio_put(LED_PIN, 1);
}

void led_off(void)
{
    gpio_put(LED_PIN, 0);
}