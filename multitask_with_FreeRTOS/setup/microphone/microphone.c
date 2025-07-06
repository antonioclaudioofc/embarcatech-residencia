#include "microphone.h"

void setup_microphone()
{
    adc_init();

    adc_gpio_init(MIC_PIN);
}