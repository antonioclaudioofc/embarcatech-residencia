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

typedef enum
{
  EVENT_JOYSTICK,
  EVENT_BUTTON
} EventType;

typedef struct
{
  uint16_t vrx;
  uint16_t vry;
} JoystickData;

typedef struct
{
  EventType type;
  JoystickData data; // válido apenas se type == EVENT_JOYSTICK
} Event;

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
    Event joystick;
    joystick.type = EVENT_JOYSTICK;

    adc_select_input(ADC_VRX_CHANNEL);
    joystick.data.vrx = adc_read();

    adc_select_input(ADC_VRY_CHANNEL);
    joystick.data.vry = adc_read();

    xQueueSend(queue, &joystick, 0);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// --- Tarefa 2: Leitura do botão ---
void button_task(void *param)
{
  while (true)
  {
    if (!gpio_get(SW_PIN))
    {
      Event button;
      button.type = EVENT_BUTTON;
      xQueueSend(queue, &button, 0);
      vTaskDelay(pdMS_TO_TICKS(50)); // debounce
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
  Event event;

  while (true)
  {
    if (xQueueReceive(queue, &event, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      if (event.type == EVENT_JOYSTICK)
      {
        // Protege acesso ao printf
        if (xSemaphoreTake(usb_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
          printf("Joystick - VRX: %d, VRY: %d\n", event.data.vrx, event.data.vry);
          xSemaphoreGive(usb_mutex);
        }

        if (abs((int)event.data.vrx - 2048) > 1000 || abs((int)event.data.vry - 2048) > 1000)
        {
          xSemaphoreGive(buzzer_semaphore);
        }
      }
      else if (event.type == EVENT_BUTTON)
      {
        xSemaphoreGive(buzzer_semaphore);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

// === Tarefa 4: Controle do Buzzer ===
void buzzer_task(void *param)
{
  while (true)
  {
    if (xSemaphoreTake(buzzer_semaphore, 0  ) == pdTRUE)
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

  queue = xQueueCreate(5, sizeof(Event));
  usb_mutex = xSemaphoreCreateMutex();
  buzzer_semaphore = xSemaphoreCreateCounting(2, 0);

  xTaskCreate(joystick_task, "Joystick", 256, NULL, 1, &joystick_handle);
  xTaskCreate(button_task, "Botão", 256, NULL, 1, &button_handle);
  xTaskCreate(data_processing_task, "Processador", 512, NULL, 1, &data_processing_handle);
  xTaskCreate(buzzer_task, "Buzzer", 512, NULL, 1, &buzzer_handle);

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