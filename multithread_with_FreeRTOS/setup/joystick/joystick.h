#include "hardware/adc.h"
#include "pico/stdlib.h"

#define VRY_PIN 26
#define VRX_PIN 27
#define SW_PIN 22

#define ADC_VRX_PIN_CHANNEL 1
#define ADC_VRY_PIN_CHANNEL 0

void setup_joystick(void);