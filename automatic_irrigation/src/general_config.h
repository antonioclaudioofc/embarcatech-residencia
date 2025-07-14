#ifndef GENERAL_CONFIG_H
#define GENERAL_CONFIG_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"
#include "pico/time.h"
#include <stdint.h>
#include <stdbool.h>
#include "pico/mutex.h"

// Pinos I2C
#define SDA_PIN 14
#define SCL_PIN 15

#define CONNECTION_TIMEOUT 2000
#define MESSAGE_TIMEOUT 2000
#define QUEUE_SIZE 16

#define SERVO_PIN 0

#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100

#define LED_PIN 11

#define WIFI_SSID "Starlink"
#define WIFI_PASS "19999999"
#define MQTT_BROKER_IP "192.168.0.14"
#define MQTT_BROKER_PORT 1883

#define TOPIC_ONLINE "pico/STATUS"
#define TOPIC_CONFIG_INTERVAL "pico/config/intervalo"
#define TOPIC_IRRIGATION "pico/irrigacao"

// Buffers globais para OLED
extern uint8_t oled_buffer[];
extern struct render_area area;

void setup_init_oled(void);
void display_and_wait(const char *message, int line_y);

#endif
