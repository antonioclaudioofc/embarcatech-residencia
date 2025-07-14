/**
 * @file connection.h
 * @brief Interface do módulo Wi-Fi no núcleo 1.
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include "general_config.h"

void connect_wifi(void);
void monitor_connection_and_reconnect(void);
bool is_wifi_connected(void);
void send_status_to_core0(uint16_t status, uint16_t attempt);
void send_ip_to_core0(uint8_t *ip);

#endif
