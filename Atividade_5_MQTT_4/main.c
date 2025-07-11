/**
 * @file main.c
 * @brief Núcleo 0 - Controle principal do sistema embarcado com Raspberry Pi Pico W.
 *
 * Este código roda no núcleo 0 do RP2040 e é responsável por:
 * - Inicializar o hardware local (OLED, PWM, fila, núcleo 1).
 * - Receber mensagens do núcleo 1 via FIFO (como IP e status MQTT).
 * - Iniciar o cliente MQTT após obter o IP.
 * - Enviar mensagens periódicas ("PING") para o broker MQTT.
 * - Coordenar a exibição de mensagens no OLED.
 * - Processar comandos recebidos por FIFO, como alteração do tempo do PING.
 */

#include "fila_circular.h"
#include "rgb_pwm_control.h"
#include "configura_geral.h"
#include "oled_utils.h"
#include "ssd1306_i2c.h"
#include "mqtt_lwip.h"
#include "lwip/ip_addr.h"
#include "pico/multicore.h"
#include <stdio.h>
#include "estado_mqtt.h"
#include <stdbool.h>
#include "pico/time.h"

#define INTERVALO_MS 5000

// Variável global e dinâmica para o tempo entre envios de PING
volatile uint32_t intervalo_ping_ms = 5000;

volatile uint8_t cor_rgb_pendente = 255;
absolute_time_t tempo_rgb_expiracao;
bool exibir_cor_agendada = false;

absolute_time_t tempo_pico_w = {0};

// Protótipos de funções externas e internas do núcleo 0
extern void funcao_wifi_nucleo1(void);
extern void espera_usb();
extern void tratar_ip_binario(uint32_t ip_bin);
extern void tratar_mensagem(MensagemWiFi msg);
void inicia_hardware();
void inicia_core1();
void verificar_fifo(void);
void tratar_fila(void);
void inicializar_mqtt_se_preciso(void);
void enviar_ping_periodico(void);
void setup_servo(void);
uint16_t angle_to_duty(float angle);
void mover_servo_para_angulo(float angle);

// Fila de comunicação entre os núcleos e controle de tempo de envio
FilaCircular fila_wifi;
absolute_time_t proximo_envio;
char mensagem_str[50];
bool ip_recebido = false;

static bool servo_ativo = false;
static absolute_time_t tempo_servo_expira = {0};

int main()
{
    inicia_hardware();
    inicia_core1();

    while (true)
    {
        verificar_fifo();              // trata mensagens FIFO
        tratar_fila();                 // trata fila circular (ex: ACK do PING)
        inicializar_mqtt_se_preciso(); // conecta ao broker, se necessário
        enviar_ping_periodico();       // envia PING no tempo certo

        // Verifica se o servo está ativo e se já passou o tempo para desligar
        if (servo_ativo && absolute_time_diff_us(get_absolute_time(), tempo_servo_expira) <= 0)
        {
            printf("[NÚCLEO 0] Desligando irrigação (servo 0°)...\n");
            mover_servo_para_angulo(180);

            servo_ativo = false;
        }

        if (publicar_online && cliente_mqtt_ativo())
        {
            if (is_nil_time(tempo_pico_w))
            {
                tempo_pico_w = make_timeout_time_ms(2000);
            }
            else if (absolute_time_diff_us(get_absolute_time(), tempo_pico_w) <= 0)
            {
                publicar_online_retain(); // envia a mensagem
                publicar_online = false;  // garante que só publique uma vez
                tempo_pico_w = nil_time;
            }
        }

        // Exibe a cor RGB no final do ciclo
        if (exibir_cor_agendada && to_ms_since_boot(get_absolute_time()) >= to_ms_since_boot(tempo_rgb_expiracao))
        {
            mostrar_cor_rgb(cor_rgb_pendente);
            exibir_cor_agendada = false;
            cor_rgb_pendente = 255;
        }

        sleep_ms(50); // intervalo entre ciclos
    }

    return 0;
}

/**
 * @brief Verifica a FIFO para processar mensagens recebidas do núcleo 1.
 */
void verificar_fifo(void)
{
    if (!multicore_fifo_rvalid())
        return;

    uint32_t pacote = multicore_fifo_pop_blocking();
    uint16_t comando = pacote >> 16;
    uint16_t valor = pacote & 0xFFFF;

    // --- Comando: alteração do intervalo do PING ---
    if (comando == 0xABCD)
    {
        set_novo_intervalo_ping((uint32_t)valor);
        return;
    }

    // --- Comando: recebimento do IP ---
    if (comando == 0xFFFE)
    {
        uint32_t ip_bin = multicore_fifo_pop_blocking();
        tratar_ip_binario(ip_bin);
        ip_recebido = true;
        return;
    }

    // --- Comando: controle do SERVO com retorno automático ---
    if (comando == 0xA1A1)
    {
        if (valor == 1)
        {
            printf("[NÚCLEO 0] Ligando irrigação (servo 180°)...\n");
            pwm_set_gpio_level(SERVO_PIN, angle_to_duty(180));

            sleep_ms(3000); // espera 3 segundos

            printf("[NÚCLEO 0] Desligando irrigação (servo 0°)...\n");
            pwm_set_gpio_level(SERVO_PIN, angle_to_duty(0));
        }
        return;
    }

    // --- Comando: controle do LED RGB ---
    if (comando == 0xB1B1)
    {
        // switch (valor)
        // {
        // case 0:
        //     set_rgb_pwm(0, 0, 0);
        //     break; // OFF
        // case 1:
        //     set_rgb_pwm(0, 0, PWM_STEP);
        //     break; // RED
        // case 2:
        //     set_rgb_pwm(0, PWM_STEP, 0);
        //     break; // GREEN
        // case 3:
        //     set_rgb_pwm(0, PWM_STEP, PWM_STEP);
        //     break; // YELLOW
        // case 4:
        //     set_rgb_pwm(PWM_STEP, 0, 0);
        //     break; // BLUE
        // case 5:
        //     set_rgb_pwm(PWM_STEP, 0, PWM_STEP);
        //     break; // MAGENTA
        // case 6:
        //     set_rgb_pwm(PWM_STEP, PWM_STEP, 0);
        //     break; // CYAN
        // case 7:
        //     set_rgb_pwm(PWM_STEP, PWM_STEP, PWM_STEP);
        //     break; // WHITE
        // default:
        //     printf("[NÚCLEO 0] Código RGB inválido: %u\n", valor);
        //     return;
        // }
        // cor_rgb_pendente = valor;
        // tempo_rgb_expiracao = make_timeout_time_ms(500); // exibir 500 ms depois
        // exibir_cor_agendada = true;
        // printf("[NÚCLEO 0] LED RGB atualizado. Código: %u\n", valor);
        printf("[NÚCLEO 0] Ligando irrigação (servo 180°)...\n");
        mover_servo_para_angulo(0);

        servo_ativo = true;
        tempo_servo_expira = make_timeout_time_ms(3000); // 3 segundos para desligar
        return;
    }

    if (comando == 0xA1A1)
    {
        if (valor == 1)
        {
            printf("[NÚCLEO 0] Ligando irrigação (servo 180°)...\n");
            pwm_set_gpio_level(SERVO_PIN, angle_to_duty(180));

            servo_ativo = true;
            tempo_servo_expira = make_timeout_time_ms(3000); // 3 segundos para desligar
        }
        return;
    }

    // --- Verificação de status inválido ---
    if (valor > 2 && comando != 0x9999)
    {
        snprintf(mensagem_str, sizeof(mensagem_str),
                 "Status inválido: %u (tentativa %u)", valor, comando);
        ssd1306_draw_utf8_multiline(buffer_oled, 0, 0, "Status inválido.");
        render_on_display(buffer_oled, &area);
        sleep_ms(3000);
        oled_clear(buffer_oled, &area);
        render_on_display(buffer_oled, &area);
        printf("%s\n", mensagem_str);
        return;
    }

    // --- Mensagem válida para a fila circular ---
    MensagemWiFi msg = {.tentativa = comando, .status = valor};
    if (!fila_inserir(&fila_wifi, msg))
    {
        ssd1306_draw_utf8_multiline(buffer_oled, 0, 0, "Fila cheia. Descartado.");
        render_on_display(buffer_oled, &area);
        sleep_ms(3000);
        oled_clear(buffer_oled, &area);
        render_on_display(buffer_oled, &area);
        printf("Fila cheia. Mensagem descartada.\n");
    }
}

/**
 * @brief Processa a próxima mensagem na fila circular, se houver.
 */
void tratar_fila(void)
{
    MensagemWiFi msg_recebida;
    if (fila_remover(&fila_wifi, &msg_recebida))
    {
        tratar_mensagem(msg_recebida);
    }
}

/**
 * @brief Inicializa o cliente MQTT assim que o IP for recebido.
 */
void inicializar_mqtt_se_preciso(void)
{
    if (!mqtt_iniciado && ultimo_ip_bin != 0)
    {
        printf("[MQTT] Iniciando cliente MQTT...\n");
        iniciar_mqtt_cliente();
        mqtt_iniciado = true;

        // Garante que o primeiro envio ocorra logo após iniciar
        proximo_envio = make_timeout_time_ms(1000);
    }
}

/**
 * @brief Envia a mensagem "PING" via MQTT em intervalos definidos.
 */
void enviar_ping_periodico(void)
{
    absolute_time_t agora = get_absolute_time();

    if (to_us_since_boot(agora) >= to_us_since_boot(proximo_envio))
    {
        printf("[MQTT] Enviando PING para o tópico: %s\n", TOPICO_PING);
        publicar_mensagem_mqtt(TOPICO_PING, "PING");
        proximo_envio = make_timeout_time_ms(intervalo_ping_ms);
    }
}

/**
 * @brief Inicializa o hardware local (USB, OLED, tela limpa).
 */
void inicia_hardware()
{
    stdio_init_all();
    setup_init_oled();
    // espera_usb();
    oled_clear(buffer_oled, &area);
    render_on_display(buffer_oled, &area);
    setup_servo();
}

/**
 * @brief Mostra mensagem de inicialização e inicia o núcleo 1.
 */
void inicia_core1()
{
    ssd1306_draw_utf8_multiline(buffer_oled, 0, 0, "Núcleo 0");
    ssd1306_draw_utf8_multiline(buffer_oled, 0, 16, "Iniciando!");
    render_on_display(buffer_oled, &area);
    sleep_ms(3000);
    oled_clear(buffer_oled, &area);
    render_on_display(buffer_oled, &area);

    printf(">> Núcleo 0 iniciado. Aguardando mensagens do núcleo 1...\n");

    init_rgb_pwm();
    fila_inicializar(&fila_wifi);
    multicore_launch_core1(funcao_wifi_nucleo1);
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
void mover_servo_para_angulo(float angle)
{
    uint slice = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_enabled(slice, true); // Reativa a slice (caso tenha sido desabilitada por RGB)
    pwm_set_gpio_level(SERVO_PIN, angle_to_duty(angle));
}
