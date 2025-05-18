#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "hardware/gpio.h"

#include "setup/setup.h"
#include "setup/led/led.h"
#include "buzzer/buzzer.h"
#include "utils/ssd1306.h"

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};

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

void turn_on_buzzer()
{
    buzzer_on();
    sleep_ms(1000);
    buzzer_off();
}

// Função para acender um LED por 1 segundos
void turn_on_led(int pin)
{
    gpio_put(pin, 1); // Acende o LED (nível baixo)
    sleep_ms(1000);   // Aguarda 1 segundos
    gpio_put(pin, 0); // Apaga o LED (nível alto)
}

void clear_display()
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

void show_comand_on_display(char *comand)
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    int text_len = strlen(comand);
    int text_width = text_len * 6;
    int x = (120 - text_width) / 2; // Centraliza horizontalmente

    ssd1306_draw_string(ssd, x, 30, comand);

    render_on_display(ssd, &frame_area);
}

int main()
{
    setup();

    clear_display();
    calculate_render_area_buffer_length(&frame_area);

    // Aguarda a conexão USB com o host
    while (!tud_cdc_connected())
    {
        sleep_ms(100);
    }
    printf("USB conectado!\n");

    while (true)
    {
        if (tud_cdc_available())
        {                                                    // Verifica se há dados disponíveis
            char buf[64];                                    // Buffer para armazenar os dados recebidos
            uint32_t count = tud_cdc_read(buf, sizeof(buf)); // Lê os dados
            buf[count] = '\0';                               // Adiciona terminador de string

            // Verifica os valores recebidos e acende o LED correspondente
            if (strcmp(buf, "vermelho") == 0)
            {
                show_comand_on_display("vermelho");
                turn_on_led(LED_RED);
                clear_display();
            }
            else if (strcmp(buf, "verde") == 0)
            {
                show_comand_on_display("verde");
                turn_on_led(LED_GREEN);
                clear_display();
            }
            else if (strcmp(buf, "azul") == 0)
            {
                show_comand_on_display("azul");
                turn_on_led(LED_BLUE);
                clear_display();
            }
            else if (strcmp(buf, "som") == 0)
            {
                show_comand_on_display("som");
                turn_on_buzzer();
                clear_display();
            }

            // Ecoa os dados recebidos de volta ao host
            tud_cdc_write(buf, count);
            tud_cdc_write_flush();
        }
        tud_task(); // Executa tarefas USB
    }

    return 0;
}