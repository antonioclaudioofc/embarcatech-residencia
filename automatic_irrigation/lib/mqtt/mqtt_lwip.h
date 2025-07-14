#ifndef MQTT_LWIP_H
#define MQTT_LWIP_H

#include "lwip/apps/mqtt.h"

// Inicializa e conecta o cliente MQTT ao broker definido em configura_geral.h
void start_mqtt_client(void);

// Publica uma mensagem no tópico definido (TOPICO) em configura_geral.h
void publish_mqtt_message(const char *topic, const char *message);

// Loop de manutenção MQTT (reservado para uso futuro)
void mqtt_loop(void);

extern bool publish_online;
void publish_online_retain(void);
bool is_mqtt_client_active(void);

#endif