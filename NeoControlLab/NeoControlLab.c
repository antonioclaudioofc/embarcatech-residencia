#include <stdio.h>
#include "pico/stdlib.h"
#include "neopixel_driver.h"
#include "testes_cores.h"
#include "efeitos.h"
#include "efeito_curva_ar.h"
#include "numeros_neopixel.h"
#include <time.h>
#include <stdlib.h>
#include "pico/time.h" // Garante acesso a time_us_32()

#define BUTTON_A 5

volatile bool debounce_button = false;
volatile int vezes = 0;

void setup_button()
{
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
}

// Inicializa o sistema e a matriz NeoPixel
void setup()
{
    stdio_init_all();
    sleep_ms(1000);  // Aguarda conexão USB (opcional)
    npInit(LED_PIN); // Inicializa matriz NeoPixel
    setup_button();
    srand(time_us_32()); // Semente para aleatoriedade
}

// Sorteia número inteiro entre [min, max]
int sorteia_entre(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

// Exibe o número sorteado de 1 a 6
void mostrar_numero_sorteado(int numero)
{
    switch (numero)
    {
    case 1:
        mostrar_numero_1();
        break;
    case 2:
        mostrar_numero_2();
        break;
    case 3:
        mostrar_numero_3();
        break;
    case 4:
        mostrar_numero_4();
        break;
    case 5:
        mostrar_numero_5();
        break;
    case 6:
        mostrar_numero_6();
        break;
    }
}

// Callback para debounce do botão
int64_t debounce_callback(alarm_id_t id, void *user_data)
{
    debounce_button = false;
    return 0;
}

int64_t display_number_callback(alarm_id_t id, void *user_data)
{
    if (vezes > 0)
    {
        int n = sorteia_entre(1, 6);
        printf("Número sorteado: %d\n", n);
        mostrar_numero_sorteado(n);
        vezes--;

        // Agenda a próxima exibição
        add_alarm_in_ms(10, display_number_callback, NULL, false);
    }

    return 0;
}

void button_irq_handler(uint gpio, uint32_t events)
{
    if (debounce_button)
        return;

    debounce_button = true;
    add_alarm_in_ms(300, debounce_callback, NULL, false);

    vezes = sorteia_entre(100, 500); // Loop entre 10 e 50 execuções
    printf("Mostrando %d números aleatórios...\n", vezes);

    add_alarm_in_ms(150, display_number_callback, NULL, false);
}

int main()
{
    setup();

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, button_irq_handler);

    while (true)
    {
        tight_loop_contents();
    }

    return 0;
}