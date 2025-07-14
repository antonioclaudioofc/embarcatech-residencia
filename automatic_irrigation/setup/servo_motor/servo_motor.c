#include "servo_motor.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "general_config.h"

void setup_servo()
{
    // --- Configuração do Periférico PWM ---

    // 1. Define a função do pino escolhido como PWM.
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    // 2. Descobre qual "fatia" (slice) de hardware PWM controla este pino.
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    // 3. Configura os parâmetros do PWM para gerar um sinal de 50Hz.
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f); // 125MHz / 125 = 1us por tick
    pwm_config_set_wrap(&config, 20000);    // 20ms = 50Hz

    // 4. Inicia o PWM com a configuração
    pwm_init(slice_num, &config, true);
}

/**
 * @brief Converte um ângulo (0° a 180°) para duty cycle (em ticks)
 */
uint16_t angle_to_duty(float angle)
{
    float pulse_ms = 0.5f + (angle / 180.0f) * 2.0f;
    return (uint16_t)((pulse_ms / 20.0f) * 20000.0f);
}

/**
 * @brief Move o servo para o ângulo especificado, reativando a slice PWM se necessário.
 */
void servo_move_to_angle(float angle)
{
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_enabled(slice, true); // Reativa a slice (caso tenha sido desabilitada por RGB)
    pwm_set_gpio_level(SERVO_PIN, angle_to_duty(angle));
}