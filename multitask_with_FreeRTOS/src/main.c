#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include "setup/setup.h"
#include "setup/led/led.h"
#include "setup/buzzer/buzzer.h"

#define ADC_VRY_CHANNEL 0
#define ADC_VRX_CHANNEL 1
#define ADC_MICROPHONE_CHANNEL 2

// Handles das tarefas
TaskHandle_t selfTest = NULL;
TaskHandle_t aliveTask = NULL;
TaskHandle_t joystickMonitorAlarm = NULL;
TaskHandle_t managerTaskHandle = NULL;

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

void set_rgb_led(int red, int green, int blue)
{
    gpio_put(LED_RED, red);
    gpio_put(LED_GREEN, green);
    gpio_put(LED_BLUE, blue);
}

void test_rgb_leds()
{
    set_rgb_led(1, 0, 0); // Red
    printf("RGB LED: Vermelho\n");
    vTaskDelay(pdMS_TO_TICKS(300));

    set_rgb_led(0, 1, 0); // Green
    printf("RGB LED: Verde\n");
    vTaskDelay(pdMS_TO_TICKS(300));

    set_rgb_led(0, 0, 1); // Blue
    printf("RGB LED: Azul\n");
    vTaskDelay(pdMS_TO_TICKS(300));

    set_rgb_led(0, 0, 0); // Off
}

void test_buzzer()
{
    buzzer_on();
    printf("Buzzer: LIGADO\n");
    vTaskDelay(pdMS_TO_TICKS(300));
    buzzer_off();
    printf("Buzzer: DESLIGADO\n");
}

void test_buttons()
{
    bool button_a = !gpio_get(BUTTON_A);
    bool button_b = !gpio_get(BUTTON_B);
    bool joystick_sw = !gpio_get(SW_PIN);

    printf("Botão A: %s\n", button_a ? "Pressionado" : "Solto");
    printf("Botão B: %s\n", button_b ? "Pressionado" : "Solto");
    printf("Joystick SW: %s\n", joystick_sw ? "Pressionado" : "Solto\n");
}

void test_joystick()
{
    adc_select_input(ADC_VRY_CHANNEL);
    uint16_t raw_y = adc_read();

    adc_select_input(ADC_VRX_CHANNEL);
    uint16_t raw_x = adc_read();

    printf("Joystick X: %4d\n", raw_x);
    printf("Joystick Y: %4d\n", raw_y);
}

void test_microphone()
{
    adc_select_input(ADC_MICROPHONE_CHANNEL);
    uint16_t mic_level = adc_read();

    printf("Nível do microfone: %4d\n", mic_level);
}

// Task A
void self_test(void *parameter)
{
    // Etapa 1: RGB
    printf("\n[1/5] Testando LEDs RGB...\n");
    test_rgb_leds();
    sleep_ms(300);

    // Etapa 2: Buzzer
    printf("\n[2/5] Testando Buzzer...\n");
    test_buzzer();
    sleep_ms(300);

    // Etapa 3: Botões
    printf("\n[3/5] Testando Botões...\n");
    test_buttons();
    sleep_ms(300);

    // Etapa 4: Joystick Analógico
    printf("\n[4/5] Testando Joystick Analógico...\n");
    test_joystick();
    sleep_ms(300);

    // Etapa 5: Microfone (ADC2)
    printf("\n[5/5] Testando Microfone...\n");
    test_microphone();
    sleep_ms(300);

    vTaskDelete(NULL); // Tarefa 1 se auto-deleta
}

// Task B
void alive_task(void *parameter)
{
    while (true)
    {
        set_rgb_led(1, 0, 0);
        printf("Tarefa 2: LED -> Ligado\n");
        vTaskDelay(pdMS_TO_TICKS(500));

        set_rgb_led(0, 0, 0);
        printf("Tarefa 2: LED -> Desligado\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task C
void joystick_monitor_alarm(void *parameter)
{
    const float VREF = 3.300f;
    const uint16_t ADC_MAX = 4095;

    while (true)
    {
        adc_select_input(ADC_VRY_CHANNEL);
        uint16_t raw_y = adc_read();
        float voltage_y = (raw_y * VREF) / ADC_MAX;

        adc_select_input(ADC_VRX_CHANNEL);
        uint16_t raw_x = adc_read();
        float voltage_x = (raw_x * VREF) / ADC_MAX;

        printf("Tarefa 3: Tensão -> X: %.3f V | Y: %.3f V\n", voltage_x, voltage_y);

        if (voltage_x > 3 || voltage_y > 3)
        {
            buzzer_on();
        }
        else
        {
            buzzer_off();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

int main()
{
    setup();

    xTaskCreate(self_test, "Self-Test", 1024, NULL, 1, &selfTest);
    xTaskCreate(alive_task, "Alive Task", 256, NULL, 1, &aliveTask);
    xTaskCreate(joystick_monitor_alarm, "Monitor de Joystick e Alarme", 1024, NULL, 1, &joystickMonitorAlarm);

    vTaskStartScheduler();

    while (true)
    {
    }
}