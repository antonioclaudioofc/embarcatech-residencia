#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define LED_GREEN 11
#define LED_RED 13

#define BUTTON_A 5

#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100

#define NUM_LEDS 3

const int LED_TIMES[NUM_LEDS] = {
    10000, // Vermelho
    10000, // Verde
    3000,  // Amarelo
};

const char *LED_NAMES[NUM_LEDS] = {
    "Vermelho",
    "Verde",
    "Amarelo",
};

volatile int current_led = 0; // Índice atual (0 a 2)
volatile int seconds_remaining = 0;
volatile bool button_pressed = false;
volatile bool name_shown = false;

const int BEEP_DURATIONS[NUM_LEDS] = {
    200,  // Vermelho = beep curto
    500,  // Verde    = beep médio
    1000, // Amarelo  = beep longo
};

const int BEEP_PAUSES[NUM_LEDS] = {
    300, // Vermelho = pausa curta
    500, // Verde    = pausa média
    700, // Amarelo  = pausa longa
};

alarm_id_t beep_alarm_id;
bool beep_on = false;

alarm_id_t led_alarm_id;
repeating_timer_t timer;

void setup_leds()
{
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);
}

void setup_button()
{
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

void setup_buzzer()
{
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096));
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void buzzer_on()
{
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_gpio_level(BUZZER_PIN, 2048);
}

void buzzer_off()
{
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

int64_t beep_callback(alarm_id_t id, void *user_data)
{
    if (beep_on)
    {
        buzzer_off();
        beep_on = false;
        return BEEP_PAUSES[current_led] * 1000; // pausa antes do próximo beep
    }
    else
    {
        buzzer_on();
        beep_on = true;
        return BEEP_DURATIONS[current_led] * 1000; // tempo do beep
    }
}

void turn_on_led(int led_index)
{
    gpio_put(LED_RED, 0);
    gpio_put(LED_GREEN, 0);

    cancel_alarm(beep_alarm_id); // Garante que beep antigo pare
    beep_on = false;
    buzzer_off(); // Garante que comece silencioso

    if (!name_shown)
    {
        printf("Sinal: %s\n", LED_NAMES[led_index]);
        name_shown = true;
    }

    switch (led_index)
    {
    case 0: // Vermelho
        gpio_put(LED_RED, 1);
        break;
    case 1: // Verde
        gpio_put(LED_GREEN, 1);
        break;
    case 2: // Amarelo (vermelho + verde)
        gpio_put(LED_RED, 1);
        gpio_put(LED_GREEN, 1);
        break;
    }

    // Inicia ciclo de beeps para o LED atual
    beep_alarm_id = add_alarm_in_us(0, beep_callback, NULL, true);
}

int64_t change_led_callback(alarm_id_t id, void *user_data)
{
    cancel_alarm(beep_alarm_id); // Para beep atual
    buzzer_off();

    // Avança para o próximo LED (0 → 1 → 2 → 0)
    current_led = (current_led + 1) % 3;
    name_shown = false;
    turn_on_led(current_led);

    // Atualiza o tempo restante com base no novo LED
    seconds_remaining = LED_TIMES[current_led] / 1000;

    // Agenda um novo alarme para trocar o LED após o tempo definido (em milissegundos) e salva o ID do alarme
    led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);
    return 0;
}

bool timepiece_callback(repeating_timer_t *t)
{
    if (seconds_remaining < 5 && seconds_remaining > 0)
    {
        printf("Tempo restante: %d segundos\n", seconds_remaining);
        seconds_remaining--;
    }
    return true;
}

void button_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BUTTON_A)
    {
        cancel_alarm(led_alarm_id);  // Cancelar alarme atual
        cancel_alarm(beep_alarm_id); // Cancelar beep atual
        buzzer_off();

        printf("Botão de Pedestres acionado\n");
        add_repeating_timer_ms(1000, timepiece_callback, NULL, &timer);

        current_led = 2; // Amarelo

        turn_on_led(current_led);
        seconds_remaining = LED_TIMES[current_led] / 1000;

        // Agenda um novo alarme para trocar o LED após o tempo definido (em milissegundos) e salva o ID do alarme
        led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);
    }
}

int main()
{
    stdio_init_all();
    setup_leds();
    setup_button();
    setup_buzzer();
    sleep_ms(2000);

    current_led = 0;
    name_shown = false;
    turn_on_led(current_led);
    seconds_remaining = LED_TIMES[current_led] / 1000;

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, button_irq_handler);

    led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);

    while (true)
    {
        tight_loop_contents();
    }
}
