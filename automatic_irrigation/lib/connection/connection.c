/**
 * @file connection.c
 * @brief Núcleo 1 - Cliente Wi-Fi com reconexão automática e envio via FIFO.
 * Envia status da conexão (azul, verde, vermelho), número da tentativa e IP ao núcleo 0.
 */

#include "connection.h"
#include "pico/cyw43_arch.h"
#include "wifi_status.h"
#include "pico/multicore.h"
#include <stdio.h>
#include <string.h>

uint8_t wifi_status_rgb = 0;

bool is_wifi_connected(void)
{
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

void send_status_to_core0(uint16_t status, uint16_t attempt)
{
    uint32_t packet = ((attempt & 0xFFFF) << 16) | (status & 0xFFFF);
    multicore_fifo_push_blocking(packet);
}

void send_ip_to_core0(uint8_t *ip)
{
    uint32_t ip_bin = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
    // Usa tentativa = 0xFFFE para indicar pacote de IP
    uint32_t packet = (0xFFFE << 16) | 0;
    multicore_fifo_push_blocking(packet);
    multicore_fifo_push_blocking(ip_bin);
}

void connect_wifi(void)
{
    send_status_to_core0(wifi_status_rgb, 0); // inicializando

    if (cyw43_arch_init())
    {
        send_status_to_core0(wifi_status_rgb, 0); // falha init
        return;
    }

    cyw43_arch_enable_sta_mode();

    for (uint16_t attempt = 1; attempt <= 5; attempt++)
    {
        int result = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 3000);

        bool connected = (result == 0) && is_wifi_connected();
        wifi_status_rgb = connected ? 1 : 2;
        send_status_to_core0(wifi_status_rgb, attempt);

        if (connected)
        {
            uint8_t *ip = (uint8_t *)&cyw43_state.netif[0].ip_addr.addr;
            send_ip_to_core0(ip);
            return;
        }

        sleep_ms(CONNECTION_TIMEOUT);
    }

    wifi_status_rgb = 2;
    send_status_to_core0(wifi_status_rgb, 0);
}

void monitor_connection_and_reconnect(void)
{
    while (true)
    {
        sleep_ms(CONNECTION_TIMEOUT);

        if (!is_wifi_connected())
        {
            wifi_status_rgb = 2;
            send_status_to_core0(wifi_status_rgb, 0);

            cyw43_arch_enable_sta_mode();

            for (uint16_t attempt = 1; attempt <= 5; attempt++)
            {
                int result = cyw43_arch_wifi_connect_timeout_ms(
                    WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, CONNECTION_TIMEOUT);

                bool reconnected = (result == 0) && is_wifi_connected();
                wifi_status_rgb = reconnected ? 1 : 2;
                send_status_to_core0(wifi_status_rgb, attempt);

                if (reconnected)
                {
                    uint8_t *ip = (uint8_t *)&cyw43_state.netif[0].ip_addr.addr;
                    send_ip_to_core0(ip);
                    break;
                }

                sleep_ms(CONNECTION_TIMEOUT);
            }

            if (!is_wifi_connected())
            {
                wifi_status_rgb = 2;
                send_status_to_core0(wifi_status_rgb, 0);
            }
        }
    }
}

// Função a ser chamada no núcleo 1
void wifi_core1_function(void)
{
    connect_wifi();
    monitor_connection_and_reconnect();
}
