/**
 * @file display.c
 * @brief Implementação de função utilitária para exibição temporária de mensagens em display OLED.
 *
 * Este módulo provê uma função de apoio para exibir mensagens curtas no display OLED controlado via I²C,
 * utilizando o driver SSD1306. Ele é utilizado para mostrar instruções ou estados durante a execução de tarefas,
 * como conexão Wi-Fi, falhas ou confirmações.
 *
 * A função exibe o texto fornecido por um curto período e, em seguida, limpa a tela automaticamente.
 *
 * Dependências:
 * - `wifi_status.h`: fornece os buffers globais (`buffer_oled`, `area`) utilizados na renderização.
 * - Funções gráficas do driver SSD1306 para manipulação do display.
 */

#include "general_config.h" // Acesso aos buffers OLED globais (antes configura_geral.h)
#include "setup/oled/oled_utils.h"
#include "lib/ssd1306/ssd1306_i2c.h"
#include "lib/ssd1306/ssd1306.h"
#include "display.h"

/**
 * @brief Exibe uma mensagem na tela OLED por 2 segundos e em seguida limpa a tela.
 *
 * @param message  Texto UTF-8 a ser exibido (pode conter múltiplas linhas).
 * @param line_y   Posição vertical (em pixels) onde a mensagem começará a ser desenhada.
 *
 * Funcionalidade:
 * - Limpa o conteúdo atual do display.
 * - Desenha o texto informado na posição especificada.
 * - Aguarda por 2000 milissegundos.
 * - Limpa novamente o display.
 *
 * Utilizada para feedback visual durante eventos como: inicialização, conexão Wi-Fi, reconexão ou erros.
 */
void display_and_wait(const char *message, int line_y)
{
    // Limpa completamente o buffer gráfico do display
    oled_clear(oled_buffer, &area);

    // Escreve o texto em formato multilinha a partir da coordenada vertical fornecida
    ssd1306_draw_utf8_multiline(oled_buffer, 0, line_y, message);

    // Envia o buffer para ser renderizado fisicamente no display
    render_on_display(oled_buffer, &area);

    // Espera 2 segundos para permitir leitura da mensagem
    sleep_ms(MESSAGE_TIMEOUT);

    // Limpa novamente o conteúdo
    oled_clear(oled_buffer, &area);
    render_on_display(oled_buffer, &area);
}
