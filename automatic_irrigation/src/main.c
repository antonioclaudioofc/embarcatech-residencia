/**
 * @file main.c
 * @brief Núcleo 0 - Controle principal do sistema embarcado com Raspberry Pi Pico W.
 *
 * Este código roda no núcleo 0 do RP2040 e é responsável por:
 * - Inicializar o hardware local (OLED, fila, núcleo 1).
 * - Receber mensagens do núcleo 1 via FIFO (como IP e status MQTT).
 * - Iniciar o cliente MQTT após obter o IP.
 * - Coordenar a exibição de mensagens no OLED.
 * - Processar comandos recebidos por FIFO.
 */

#include "lib/circular_queue/circular_queue.h"
#include "general_config.h"
#include "setup/oled/oled_utils.h"
#include "setup/led/led.h"
#include "setup/buzzer/buzzer.h"
#include "setup/setup.h"
#include "lib/ssd1306/ssd1306_i2c.h"
#include "lib/mqtt/mqtt_lwip.h"
#include "lwip/ip_addr.h"
#include "pico/multicore.h"
#include <stdio.h>
#include "mqtt_state.h"
#include "pico/time.h"

absolute_time_t pico_w_time;

static bool servo_ativo = false;
static absolute_time_t tempo_servo_expira = {0};

// Protótipos de funções externas e internas do núcleo 0
extern void wifi_core1_function(void);
extern void handle_ip_binary(uint32_t ip_bin);
extern void handle_message(WiFiMessage msg);
void init_hardware();
void init_core1();
void check_fifo(void);
void process_queue(void);
void init_mqtt_if_needed(void);
void send_periodic_ping(void);
void setup_servo(void);
uint16_t angle_to_duty(float angle);
void servo_move_to_angle(float angle);

// Fila de comunicação entre os núcleos e controle de tempo de envio
CircularQueue wifi_queue;
absolute_time_t next_send;
char message_str[50];
bool ip_received = false;

int main()
{
    setup();
    setup_servo();
    init_core1();

    while (true)
    {
        check_fifo();
        process_queue();
        init_mqtt_if_needed();

        // Verifica se o servo está ativo e se já passou o tempo para desligar
        if (servo_ativo && absolute_time_diff_us(get_absolute_time(), tempo_servo_expira) <= 0)
        {
            printf("[NÚCLEO 0] Desligando irrigação (servo 0°)...\n");
            servo_move_to_angle(45);
            buzzer_off();
            led_off();

            servo_ativo = false;
        }

        // Publicar "Pico W online" com atraso controlado após conexão
        if (publish_online && is_mqtt_client_active())
        {
            if (is_nil_time(pico_w_time))
            {
                pico_w_time = make_timeout_time_ms(2000); // espera 2 segundos
            }
            else if (absolute_time_diff_us(get_absolute_time(), pico_w_time) <= 0)
            {
                publish_online_retain(); // envia com retain = 1
                publish_online = false;
                pico_w_time = nil_time; // zera tempo
            }
        }

        sleep_ms(50);
    }

    return 0;
}

/**
 * @brief Verifica a FIFO para processar mensagens recebidas do núcleo 1.
 */
void check_fifo(void)
{
    if (!multicore_fifo_rvalid())
        return;

    uint32_t packet = multicore_fifo_pop_blocking();
    uint16_t command = packet >> 16;
    uint16_t value = packet & 0xFFFF;

    if (command == 0xABCD)
    {
        set_new_ping_interval((uint32_t)value);
        return;
    }

    // --- Comando: controle do SERVO com retorno automático ---
    if (command == 0xB1B1)
    {
        if (value == 1)
        {
            printf("[NÚCLEO 0] Ligando irrigação (servo 180°)...\n");
            servo_move_to_angle(0);
            buzzer_on();
            led_on();

            servo_ativo = true;
            tempo_servo_expira = make_timeout_time_ms(3000); // 3 segundos para desligar
            return;
        }
    }

    if (command == 0xFFFE)
    {
        uint32_t ip_bin = multicore_fifo_pop_blocking();
        handle_ip_binary(ip_bin);
        ip_received = true;
        return;
    }

    if (value > 2 && command != 0x9999)
    {
        snprintf(message_str, sizeof(message_str),
                 "Status inválido: %u (tentativa %u)", value, command);
        ssd1306_draw_utf8_multiline(oled_buffer, 0, 0, "Status inválido.");
        render_on_display(oled_buffer, &area);
        sleep_ms(3000);
        oled_clear(oled_buffer, &area);
        render_on_display(oled_buffer, &area);
        printf("%s\n", message_str);
        return;
    }

    WiFiMessage msg = {.attempt = command, .status = value};
    if (!queue_enqueue(&wifi_queue, msg))
    {
        ssd1306_draw_utf8_multiline(oled_buffer, 0, 0, "Fila cheia. Descartado.");
        render_on_display(oled_buffer, &area);
        sleep_ms(3000);
        oled_clear(oled_buffer, &area);
        render_on_display(oled_buffer, &area);
        printf("Fila cheia. Mensagem descartada.\n");
    }
}

/**
 * @brief Processa a próxima mensagem na fila circular, se houver.
 */
void process_queue(void)
{
    WiFiMessage received_msg;
    if (queue_dequeue(&wifi_queue, &received_msg))
    {
        handle_message(received_msg);
    }
}

/**
 * @brief Inicializa o cliente MQTT assim que o IP for recebido.
 */
void init_mqtt_if_needed(void)
{
    if (!mqtt_started && last_ip_bin != 0)
    {
        printf("[MQTT] Iniciando cliente MQTT...\n");
        start_mqtt_client();
        mqtt_started = true;

        // Garante que o primeiro envio ocorra logo após iniciar
        next_send = make_timeout_time_ms(1000);
    }
}

/**
 * @brief Mostra mensagem de inicialização e inicia o núcleo 1.
 */
void init_core1()
{
    ssd1306_draw_utf8_multiline(oled_buffer, 0, 0, "Núcleo 0");
    ssd1306_draw_utf8_multiline(oled_buffer, 0, 16, "Iniciando!");
    render_on_display(oled_buffer, &area);
    sleep_ms(3000);
    oled_clear(oled_buffer, &area);
    render_on_display(oled_buffer, &area);

    printf(">> Núcleo 0 iniciado. Aguardando mensagens do núcleo 1...\n");

    queue_init(&wifi_queue);
    multicore_launch_core1(wifi_core1_function);
}

void setup_servo()
{
    // --- Configuração do Periférico PWM ---

    // 1. Define a função do pino escolhido como PWM.
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);

    // 2. Descobre qual "fatia" (slice) de hardware PWM controla este pino.
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    // 3. Configura os parâmetros do PWM para gerar um sinal de 50Hz.
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f); // 125MHz / 125 = 1us por tick
    pwm_config_set_wrap(&config, 20000);    // 20ms = 50Hz

    // 4. Inicia o PWM com a configuração
    pwm_init(slice_num, &config, true);
}

/**
 * @brief Converte um ângulo (0° a 180°) para duty cycle (em ticks)
 */
uint16_t angle_to_duty(float angle)
{
    float pulse_ms = 0.5f + (angle / 180.0f) * 2.0f;
    return (uint16_t)((pulse_ms / 20.0f) * 20000.0f);
}

/**
 * @brief Move o servo para o ângulo especificado, reativando a slice PWM se necessário.
 */
void servo_move_to_angle(float angle)
{
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_enabled(slice, true); // Reativa a slice (caso tenha sido desabilitada por RGB)
    pwm_set_gpio_level(SERVO_PIN, angle_to_duty(angle));
}