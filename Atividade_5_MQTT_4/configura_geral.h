#ifndef CONFIGURA_GERAL_H
#define CONFIGURA_GERAL_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "pico/mutex.h"

// LED RGB
#define LED_R 13
#define LED_G 11
#define LED_B 12
#define PWM_STEP 0xFFFF
// #define PWM_STEP (1 << 8)

// Pinos I2C
#define SDA_PIN 14
#define SCL_PIN 15

#define SERVO_PIN 0

#define TEMPO_CONEXAO 2000
#define TEMPO_MENSAGEM 2000
#define TAM_FILA 16

#define WIFI_SSID "linux"
#define WIFI_PASS "00000000"
#define MQTT_BROKER_IP "10.119.74.30"

#define MQTT_BROKER_PORT 1883

#define TOPICO_PING "pico/PING"
#define TOPICO_ONLINE "pico/STATUS"
#define TOPICO_CONFIG_INTERVALO "pico/config/intervalo"
#define TOPICO_COMANDO_LED "pico/comando/led"
#define TOPICO_COMANDO_RGB "pico/comando/rgb"
#define TOPICO_MENSAGEM_OLED "pico/mensagem/oled"
#define TOPICO_ACIONAR_SERVO "pico/comando/servo"

// Buffers globais para OLED
extern uint8_t buffer_oled[];
extern struct render_area area;

void setup_init_oled(void);
void exibir_e_esperar(const char *mensagem, int linha_y);

#endif