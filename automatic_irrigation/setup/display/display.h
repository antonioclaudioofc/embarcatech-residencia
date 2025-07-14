/**
 * @file display.h
 * @brief Interface para exibição de mensagens em display OLED ou similar.
 *
 * Este cabeçalho declara a função `display_and_wait()`, responsável por apresentar uma mensagem
 * textual em uma linha específica do display gráfico e aguardar um intervalo de tempo ou evento,
 * conforme definido na implementação.
 *
 * A função é útil para exibir informações de status ou instruções em projetos com microcontroladores
 * e interfaces visuais, como o Raspberry Pi Pico com display SSD1306.
 *
 * Parâmetros:
 *  - `message`: texto a ser exibido na tela.
 *  - `line_y`: linha vertical (em pixels ou unidade de página) onde a mensagem será posicionada.
 *
 * Ideal para aplicações que exigem feedback visual ao usuário durante processos como inicialização,
 * conexão à rede, ou exibição de dados sensoriais.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

void display_and_wait(const char *message, int line_y);

#endif
