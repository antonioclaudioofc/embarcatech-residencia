#ifndef MQTT_STATE_H
#define MQTT_STATE_H

#include <stdint.h>
#include <stdbool.h>

// Variáveis compartilhadas entre arquivos
extern uint32_t last_ip_bin;
extern bool mqtt_started;

// Buffer OLED e área global
extern uint8_t oled_buffer[];
extern struct render_area area;

// Variável de controle do ping
extern volatile uint32_t ping_interval_ms;

void set_new_ping_interval(uint32_t new_interval);

#endif
