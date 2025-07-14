/**
 * @file main_aux.c
 * @brief Funções auxiliares do núcleo 0 no projeto multicore com Raspberry Pi Pico W.
 *
 * Este arquivo complementa a lógica do núcleo 0, com foco em:
 * - Visualização de mensagens no display OLED.
 * - Interpretação dos dados vindos do núcleo 1 via FIFO.
 * - Controle do LED RGB com base no status da conexão Wi-Fi.
 * - Apresentação do endereço IP recebido.
 * - Atualização do tempo de envio do PING (com feedback visual).
 */

#include "lib/circular_queue/circular_queue.h"
#include "general_config.h"
#include "setup/oled/oled_utils.h"
#include "lib/ssd1306/ssd1306_i2c.h"
#include "lib/mqtt/mqtt_lwip.h"
#include "lwip/ip_addr.h"
#include "pico/multicore.h"
#include <stdio.h>
#include "src/mqtt_state.h" // Para acesso a ping_interval_ms

void handle_message(WiFiMessage msg)
{
    const char *description = "";

    if (msg.attempt == 0x9999)
    {
        if (msg.status == 0)
        {
            ssd1306_draw_utf8_multiline(oled_buffer, 0, 32, "ACK do PING OK");
        }
        else
        {
            ssd1306_draw_utf8_multiline(oled_buffer, 0, 32, "ACK do PING FALHOU");
        }
        render_on_display(oled_buffer, &area);
        return;
    }

    switch (msg.status)
    {
    case 0:
        description = "INICIALIZANDO";
        break;
    case 1:
        description = "CONECTADO";
        break;
    case 2:
        description = "FALHA";
        break;
    default:
        description = "DESCONHECIDO";
        break;
    }

    char status_line[32];
    snprintf(status_line, sizeof(status_line), "Status do Wi-Fi : %s", description);

    ssd1306_draw_utf8_multiline(oled_buffer, 0, 0, status_line);
    render_on_display(oled_buffer, &area);
    sleep_ms(3000);
    oled_clear(oled_buffer, &area);
    render_on_display(oled_buffer, &area);

    printf("[NÚCLEO 0] Status: %s (%s)\n", description, msg.attempt > 0 ? description : "evento");
}

void handle_ip_binary(uint32_t ip_bin)
{
    char ip_str[20];
    uint8_t ip[4];

    ip[0] = (ip_bin >> 24) & 0xFF;
    ip[1] = (ip_bin >> 16) & 0xFF;
    ip[2] = (ip_bin >> 8) & 0xFF;
    ip[3] = ip_bin & 0xFF;

    snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    oled_clear(oled_buffer, &area);
    ssd1306_draw_utf8_string(oled_buffer, 0, 0, ip_str);
    render_on_display(oled_buffer, &area);

    printf("[NÚCLEO 0] Endereço IP: %s\n", ip_str);
    last_ip_bin = ip_bin;
}

void display_mqtt_status(const char *text)
{
    ssd1306_draw_utf8_string(oled_buffer, 0, 16, "MQTT: ");
    ssd1306_draw_utf8_string(oled_buffer, 40, 16, text);
    render_on_display(oled_buffer, &area);

    printf("[MQTT] %s\n", text);
}

void set_new_ping_interval(uint32_t new_interval)
{
    if (new_interval >= 1000 && new_interval <= 60000)
    {
        ping_interval_ms = new_interval;

        char buffer_msg[32];
        snprintf(buffer_msg, sizeof(buffer_msg), "Intervalo: %u ms", new_interval);

        ssd1306_clear_area(oled_buffer, 0, 40, 127, 50);
        ssd1306_draw_utf8_multiline(oled_buffer, 0, 42, buffer_msg);
        render_on_display(oled_buffer, &area);

        printf("[INFO] Intervalo atualizado para %u ms\n", new_interval);
    }
    else
    {
        printf("[AVISO] Valor %u fora do intervalo permitido (1000–60000 ms)\n", new_interval);
    }
}
