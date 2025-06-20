#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "setup/setup.h"
#include "setup/led/led.h"

void set_traffic_light(int red, int green)
{
    gpio_put(LED_RED, red);
    gpio_put(LED_GREEN, green);
}

void red_task()
{
    while (true)
    {
        // LED vermelho acende nos primeiros 5s do ciclo
        set_traffic_light(1, 0);
        printf("Vermelho\n");
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Espera o restante do ciclo (13s - 5s)
        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

void green_task()
{
    // Espera 5s para entrar no ciclo após o vermelho
    vTaskDelay(pdMS_TO_TICKS(5000));

    while (true)
    {
        set_traffic_light(0, 1);

        printf("Verde\n");
        vTaskDelay(pdMS_TO_TICKS(5000));

        // Espera até o próximo ciclo (13s - 5s)
        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

void yellow_task(void *params)
{
    // Espera 10s para entrar no ciclo após o verde
    vTaskDelay(pdMS_TO_TICKS(10000));

    while (true)
    {
        set_traffic_light(1, 1);

        printf("Amarelo\n");
        vTaskDelay(pdMS_TO_TICKS(3000));

        // Espera até o próximo ciclo (13s - 3s)
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

int main()
{
    setup();

    xTaskCreate(red_task, "Red", 256, NULL, 1, NULL);
    xTaskCreate(green_task, "Green", 256, NULL, 2, NULL);
    xTaskCreate(yellow_task, "Yellow", 256, NULL, 3, NULL);

    vTaskStartScheduler();

    while (true)
    {
        tight_loop_contents();
    }
}
