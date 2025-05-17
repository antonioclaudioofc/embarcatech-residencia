#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "setup/setup.h"
#include "setup/buzzer/buzzer.h"
#include "setup/button/button.h"
#include "setup/led/led.h"
#include "setup/display/display.h"
#include "utils/ssd1306_i2c.h"

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
volatile bool button_pressed = true;
volatile bool name_shown = false;
volatile bool debounce_button = false;
volatile bool is_traffic_yellow = false;

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};

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

void clear_display()
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void show_seconds_on_display(int seconds)
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 10, 10, "Tempo restante:");

    char text[4];
    snprintf(text, sizeof(text), "%d", seconds);
    ssd1306_draw_string(ssd, 50, 30, text);

    render_on_display(ssd, &frame_area);
}

bool timepiece_callback(repeating_timer_t *t)
{

    if (seconds_remaining > 0)
    {
        if (seconds_remaining <= 5)
        {
            printf("Tempo restante: %d segundos\n", seconds_remaining);
            show_seconds_on_display(seconds_remaining);
        }
        seconds_remaining--;

        // Se chegou a 0 depois de decrementar, agende a limpeza do display
        if (seconds_remaining == 0)
        {
            add_alarm_in_ms(1000, (alarm_callback_t)clear_display, NULL, false);
        }
    }

        return true;
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

    if (led_index == 0 && is_traffic_yellow) // Se está indo para vermelho após botão
    {
        // Adicionar 0.9s por motivo que adicionei uma debounce no botão de 0.2s e não estava mostrando o cronometro até o 5s
        add_repeating_timer_ms(900, timepiece_callback, NULL, &timer);
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
    cancel_alarm(beep_alarm_id); // Garante que beep antigo pare
    cancel_alarm(beep_alarm_id); // Para beep atual
    buzzer_off();

    // Avança para o próximo LED (0 → 1 → 2 → 0)
    current_led = (current_led + 1) % 3;
    cancel_repeating_timer(&timer); // Cancela se existir um cronômetro anterior
    name_shown = false;

    turn_on_led(current_led);
    is_traffic_yellow = false;

    // Atualiza o tempo restante com base no novo LED
    seconds_remaining = LED_TIMES[current_led] / 1000;

    button_pressed = true;

    // Agenda um novo alarme para trocar o LED após o tempo definido (em milissegundos) e salva o ID do alarme
    led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);
    return 0;
}

// Callback para debounce do botão
int64_t debounce_callback(alarm_id_t id, void *user_data)
{
    debounce_button = false;
    return 0;
}

void button_irq_handler(uint gpio, uint32_t events)
{
    if (!button_pressed || debounce_button)
        return;

    debounce_button = true;
    add_alarm_in_ms(200, debounce_callback, NULL, false);

    cancel_alarm(led_alarm_id);
    cancel_alarm(beep_alarm_id); // Cancelar beep atual
    buzzer_off();
    cancel_repeating_timer(&timer);

    printf("Botão de Pedestres acionado\n");

    button_pressed = false;
    is_traffic_yellow = true;

    current_led = 2; // Amarelo
    name_shown = false;

    turn_on_led(current_led);
    seconds_remaining = LED_TIMES[current_led] / 1000;

    // Agenda um novo alarme para trocar o LED após o tempo definido (em milissegundos) e salva o ID do alarme
    led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);
}

int main()
{
    setup();

    calculate_render_area_buffer_length(&frame_area);
    clear_display();

    current_led = 0;
    name_shown = false;
    turn_on_led(current_led);
    seconds_remaining = LED_TIMES[current_led] / 1000;

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, button_irq_handler);

    led_alarm_id = add_alarm_in_ms(LED_TIMES[current_led], change_led_callback, NULL, false);

    while (true)
    {
        sleep_ms(100);
    }

    return 0;
}
