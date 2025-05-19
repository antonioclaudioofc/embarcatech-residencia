#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "pico/multicore.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

const int VRX = 26;
const int ADC_CHANNEL_0 = 0;

const int RED_LED = 13;
const int BLUE_LED = 12;
const int GREEN_LED = 11;

const int LOW = 1365;
const int MODERATE = 2730;
const int HIGH = 4096;

volatile int flag_estado = 2;

const int BUZZER_PIN = 21;
const int BUZZER_FREQUENCY = 100;

void setup_joystick()
{
    adc_init();
    adc_gpio_init(VRX);
}

void setup_leds()
{
    gpio_init(RED_LED);
    gpio_init(BLUE_LED);
    gpio_init(GREEN_LED);
    gpio_set_dir(RED_LED, GPIO_OUT);
    gpio_set_dir(BLUE_LED, GPIO_OUT);
    gpio_set_dir(GREEN_LED, GPIO_OUT);
}

void setup_buzzer()
{
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz((clk_sys) / (BUZZER_FREQUENCY * 4096)));
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void beep()
{
    pwm_set_gpio_level(BUZZER_PIN, 4096);
}

void disable_beep()
{
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    multicore_fifo_push_blocking(flag_estado);
    return 2000;
}

void toggle_led(int red, int blue, int green)
{
    gpio_put(RED_LED, red);
    gpio_put(BLUE_LED, blue);
    gpio_put(GREEN_LED, green);
}

void core1_main()
{
    while (true)
    {
        if (multicore_fifo_rvalid())
        {
            int flag = multicore_fifo_pop_blocking();

            if (flag == 1)
            {
                toggle_led(0, 0, 1);
                disable_beep();
            }
            else if (flag == 2)
            {
                toggle_led(0, 1, 0);
                disable_beep();
            }
            else
            {
                toggle_led(1, 0, 0);
                beep();
            }
        }
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);

    multicore_launch_core1(core1_main);

    uint16_t vrx_value;
    setup_joystick();
    setup_leds();
    setup_buzzer();

    add_alarm_in_ms(2000, alarm_callback, NULL, true);

    while (true)
    {
        adc_select_input(ADC_CHANNEL_0);
        sleep_us(2);
        vrx_value = adc_read();

        if (vrx_value < LOW)
        {
            flag_estado = 1;
        }
        else if (vrx_value < MODERATE)
        {
            flag_estado = 2;
        }
        else if (vrx_value >= MODERATE && vrx_value <= HIGH)
        {
            flag_estado = 3;
        }

        tight_loop_contents();
    }

    return 0;
}