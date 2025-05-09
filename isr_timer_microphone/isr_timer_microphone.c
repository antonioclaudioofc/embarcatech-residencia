#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/timer.h"
#include "neopixel.c"

#define MIC_CHANNEL 2
#define MIC_PIN 28

#define LED_PIN 7
#define LED_COUNT 25

#define ADC_CLOCK_DIV 96.f
#define SAMPLES 200

#define INTERVAL 750
#define LIMIAR_ADC 2055

uint dma_channel;
dma_channel_config dma_cfg;

volatile float intensity_detected = 0;
uint16_t adc_buffer[SAMPLES];

const uint8_t X_pattern[9] = {0, 4, 6, 8, 12, 16, 18, 20, 24};

void setup_microphone()
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    adc_fifo_setup(
        true,
        true,
        1,
        false,
        false);

    adc_set_clkdiv(ADC_CLOCK_DIV);
}

void setup_neopixel()
{
    npInit(LED_PIN, LED_COUNT);
}

void setup_dma()
{
    dma_channel = dma_claim_unused_channel(true);

    dma_cfg = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);

    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}

void sample_mic()
{
    adc_fifo_drain();
    adc_run(false);

    dma_channel_configure(dma_channel, &dma_cfg, adc_buffer, &(adc_hw->fifo), SAMPLES, true);

    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);
    adc_run(false);
}
bool read_microphone(struct repeating_timer *t)
{
    sample_mic();

    float sum = 0.0f;
    float avg = 0.0f;
    float value = 0.0f;

    for (int i = 0; i < SAMPLES; ++i)
    {
        avg += adc_buffer[i];
        sleep_us(5);
    }
    avg /= SAMPLES;

    for (int i = 0; i < SAMPLES; ++i)
    {
        value = (float)adc_buffer[i] - avg;
        sum += value * value;
    }

    float rms = sqrtf(sum / SAMPLES);

    if (rms > 10.0f && avg > LIMIAR_ADC)
    {
        intensity_detected = 1.0;
    }
    else
    {
        intensity_detected = 0.0;
    }

    printf("Média: %.2f\n", avg);
    return true;
}

void draw_X()
{
    npClear();
    for (int i = 0; i < sizeof(X_pattern) / sizeof(X_pattern[0]); i++)
    {
        npSetLED(X_pattern[i], 255, 0, 0);
    }
    npWrite();
}

int main()
{
    stdio_init_all();
    sleep_ms(2000);
    setup_neopixel();
    setup_microphone();
    setup_dma();

    struct repeating_timer timer;
    add_repeating_timer_ms(INTERVAL, read_microphone, NULL, &timer);

    while (true)
    {
        if (intensity_detected > 0.0)
        {
            draw_X();
        }
        else
        {
            npClear();
            npWrite();
        }

        sleep_ms(300);
    }
}
