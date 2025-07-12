#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "hardware/adc.h"

#include "setup/setup.h"
#include "setup/buzzer/buzzer.h"

#define ADC_VRY_CHANNEL 0
#define ADC_VRX_CHANNEL 1

// --- Handles globais ---
QueueHandle_t queue;
SemaphoreHandle_t usb_mutex;
SemaphoreHandle_t buzzer_semaphore;

// --- Handles de tarefas ---
TaskHandle_t joystick_handle, button_handle, data_processing_handle, buzzer_handle;

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

// --- Tarefa 1: Leitura do Joystick ---
void joystick_task(void *param)
{
  while (true)
  {
    uint16_t vrx, vry;

    adc_select_input(ADC_VRX_CHANNEL);
    vrx = adc_read();

    adc_select_input(ADC_VRY_CHANNEL);
    vry = adc_read();

    uint16_t data[2] = {vrx, vry};
    xQueueSend(queue, &data, 0);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// --- Tarefa 2: Leitura do botão ---
void button_task(void *param)
{
  while (true)
  {
    if (!gpio_get(BUTTON_A) || !gpio_get(BUTTON_B) || !gpio_get(SW_PIN))
    {
      xQueueSend(queue, NULL, 0);
      vTaskDelay(pdMS_TO_TICKS(200)); // debounce
    }
    else
    {
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

// --- Tarefa 3: Processamento dos Dados ---
void data_processing_task(void *param)
{
  uint16_t data[2];

  while (true)
  {
    if (xQueueReceive(queue, &data, pdMS_TO_TICKS(10)) == pdTRUE)
    {
      // Protege acesso ao printf
      if (xSemaphoreTake(usb_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        if (data != NULL)
        {
          printf("Joystick - VRX: %d, VRY: %d\n", data[0], data[1]);

          // Movimento significativo
          if (abs(data[0] - 2048) > 500 || abs(data[1] - 2048) > 500)
          {
            xSemaphoreGive(buzzer_semaphore);
          }
        }
        else
        {
          printf("Botão pressionado!\n");
          xSemaphoreGive(buzzer_semaphore);
        }

        xSemaphoreGive(usb_mutex);
      }

      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

// === Tarefa 4: Controle do Buzzer ===
void buzzer_task(void *param)
{
  while (true)
  {
    if (xSemaphoreTake(buzzer_semaphore, portMAX_DELAY) == pdTRUE)
    {
      buzzer_on();
      vTaskDelay(pdMS_TO_TICKS(100));
      buzzer_off();
    }
  }
}

int main()
{
  setup();

  queue = xQueueCreate(10, sizeof(uint16_t[2]));
  usb_mutex = xSemaphoreCreateMutex();
  buzzer_semaphore = xSemaphoreCreateCounting(2, 0);

  xTaskCreate(joystick_task, "Joystick", 256, NULL, 1, &joystick_handle);
  xTaskCreate(button_task, "Botão", 256, NULL, 1, &button_handle);
  xTaskCreate(data_processing_task, "Processador", 256, NULL, 1, &data_processing_handle);
  xTaskCreate(buzzer_task, "Buzzer", 256, NULL, 1, &buzzer_handle);

  // Afinidade de núcleo (SMP)
  vTaskCoreAffinitySet(joystick_handle, (1 << 0));        // Core 0
  vTaskCoreAffinitySet(button_handle, (1 << 0));          // Core 0
  vTaskCoreAffinitySet(data_processing_handle, (1 << 1)); // Core 1
  vTaskCoreAffinitySet(buzzer_handle, (1 << 1));          // Core 1

  vTaskStartScheduler();

  while (true)
  {
  }
}