/**
 * @file mqtt_state.c
 * @brief Estado compartilhado do cliente MQTT e estruturas globais do OLED.
 *
 * Este arquivo armazena **variáveis globais** utilizadas em múltiplos arquivos do sistema,
 * facilitando o compartilhamento de estado entre o código principal (`main.c`) e os
 * módulos auxiliares, como `main_auxiliar.c` e `mqtt_lwip.c`.
 *
 * Ele define:
 * - O último endereço IP recebido (`last_ip_bin`), utilizado para iniciar o cliente MQTT;
 * - Um flag (`mqtt_started`) que garante que o cliente MQTT só será iniciado uma vez;
 * - Um buffer de vídeo (`oled_buffer`) para escrita no display OLED;
 * - A estrutura `area`, que define a região da tela sendo desenhada.
 *
 * O objetivo é **centralizar informações compartilhadas** entre os diversos arquivos
 * do projeto, evitando duplicação e facilitando a manutenção e legibilidade do código.
 */

#include "mqtt_state.h"              // Declaração das variáveis externas
#include "lib/ssd1306/ssd1306_i2c.h" // Define tamanho do buffer e estrutura de renderização

// ================================
// DEFINIÇÕES GLOBAIS ÚNICAS
// ================================

/**
 * @brief Endereço IP mais recente recebido via núcleo 1.
 *
 * Usado pelo núcleo 0 para verificar se a rede já obteve um IP válido
 * e então iniciar o cliente MQTT.
 */
uint32_t last_ip_bin = 0;

/**
 * @brief Flag de controle que indica se o cliente MQTT já foi iniciado.
 *
 * Isso evita múltiplas tentativas de iniciar o cliente MQTT após o recebimento do IP.
 * Deve ser verificado no loop principal.
 */
bool mqtt_started = false;

/**
 * @brief Buffer de vídeo para o display OLED.
 *
 * Este buffer contém os dados de pixels que serão renderizados na tela.
 * Seu tamanho é definido pela função `ssd1306_buffer_length`, de acordo com
 * a resolução do display (tipicamente 128x64).
 */
uint8_t oled_buffer[ssd1306_buffer_length];

/**
 * @brief Estrutura que define a área da tela a ser desenhada.
 *
 * Usada pelas funções de renderização para aplicar atualizações no OLED,
 * como `render_on_display()`.
 */
struct render_area area;

volatile uint32_t ping_interval_ms = 5000;
