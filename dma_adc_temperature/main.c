#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

#include "setup/setup.h"
#include "utils/ssd1306.h"

#define NUM_SAMPLES 100 // Número de amostras por ciclo de leitura

uint16_t adc_buffer[NUM_SAMPLES]; // Buffer para armazenar as amostras do ADC
int dma_chan;
dma_channel_config cfg;

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1};

// Converte o valor bruto do ADC (12 bits) para temperatura em graus Celsius
float convert_to_celsius(uint16_t raw)
{
    const float conversion_factor = 3.3f / (1 << 12); // Fator de conversão para 3.3V e 12 bits
    float voltage = raw * conversion_factor;          // Converte valor para tensão
    return 27.0f - (voltage - 0.706f) / 0.001721f;    // Fórmula do datasheet do RP2040
}

float read_temperature()
{
    adc_fifo_drain();

    // Configura o ADC para colocar dados no FIFO
    adc_run(false); // Desliga ADC temporariamente
    adc_fifo_setup(
        true, // Envia dados para o FIFO
        true, // Habilita DMA para o FIFO
        1,    // Gatilho a cada amostra
        false,
        false);
    adc_run(true); // Liga ADC para começar a amostrar

    // Inicia a transferência DMA: do FIFO ADC para adc_buffer
    dma_channel_configure(
        dma_chan,
        &cfg,
        adc_buffer,    // Endereço de destino na RAM
        &adc_hw->fifo, // Endereço de origem (registrador FIFO do ADC)
        NUM_SAMPLES,   // Número de transferências (amostras)
        true           // Inicia imediatamente
    );

    // Aguarda até que a transferência DMA seja concluída
    dma_channel_wait_for_finish_blocking(dma_chan);

    // Desliga o ADC após capturar os dados
    adc_run(false);

    // Calcula a média das temperaturas lidas
    float sum = 0.0f;
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        sum += convert_to_celsius(adc_buffer[i]); // Converte cada valor para °C e soma
    }

    float avg_temp = sum / NUM_SAMPLES;

    return avg_temp;
}

void show_temperature_on_display(float temperature)
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 20, 10, "Temperatura:");

    char text[10];
    snprintf(text, sizeof(text), "%.1f C", temperature);

    int text_len = strlen(text);
    int text_width = text_len * 6;
    int x = (120 - text_width) / 2; // Centraliza horizontalmente

    ssd1306_draw_string(ssd, x, 30, text);

    render_on_display(ssd, &frame_area);
}

void clear_display()
{
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);
}

bool alarm_callback(repeating_timer_t *t)
{
    float temperature = read_temperature();
    show_temperature_on_display(temperature);

    // Temperatura média em °C
    printf("Temperatura média: %.2f °C\n", temperature); // Imprime no terminal
    return true;
}

int main()
{
    setup();

    clear_display();
    calculate_render_area_buffer_length(&frame_area);

    // Configura o canal DMA para receber dados do ADC
    dma_chan = dma_claim_unused_channel(true);      // Requisita um canal DMA disponível
    cfg = dma_channel_get_default_config(dma_chan); // Obtem configuração padrão

    // Configurações do canal DMA
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16); // Cada leitura é de 16 bits
    channel_config_set_read_increment(&cfg, false);           // Endereço fixo (registrador ADC FIFO)
    channel_config_set_write_increment(&cfg, true);           // Incrementa para armazenar em adc_buffer[]
    channel_config_set_dreq(&cfg, DREQ_ADC);                  // Dispara automaticamente com dados do ADC

    static repeating_timer_t timer;
    add_repeating_timer_ms(1000, alarm_callback, NULL, &timer);

    while (true)
    {
        tight_loop_contents();
    }
}
