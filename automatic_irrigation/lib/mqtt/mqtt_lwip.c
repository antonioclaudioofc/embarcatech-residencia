/**
 * @file mqtt_lwip.c
 * @brief Implementação do cliente MQTT usando a pilha lwIP no Raspberry Pi Pico W.
 *
 * Este módulo gerencia a conexão com o broker MQTT e a publicação/assinatura de mensagens.
 * Utiliza a pilha lwIP e a API MQTT (lwip/apps/mqtt.h) para comunicação TCP/IP.
 *
 * Principais responsabilidades:
 * - Criar e configurar o cliente MQTT.
 * - Conectar-se ao broker definido via IP.
 * - Assinar múltiplos tópicos e registrar callbacks de entrada.
 * - Publicar mensagens de forma segura, evitando congestionamento de envio.
 * - Notificar o núcleo 0 via FIFO sobre resultados de publicação.
 */

#include <stdio.h>
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"
#include "general_config.h"
#include "display_utils.h"
#include "mqtt_lwip.h"

// ========================
// VARIÁVEIS GLOBAIS INTERNAS
// ========================

bool publish_online = false;

/**
 * @brief Armazena o nome do tópico recebido no último pacote MQTT.
 */
static char received_topic[64] = {0};

/**
 * @brief Ponteiro para o cliente MQTT.
 *
 * Criado com mqtt_client_new() e usado em todas as operações MQTT.
 */
static mqtt_client_t *mqtt_client;

/**
 * @brief Estrutura com informações do cliente MQTT (ID, credenciais).
 *
 * Inicializada com client_id "pico_lwip". Pode ser expandida para login/senha.
 */
static struct mqtt_connect_client_info_t client_info;

/**
 * @brief Flag de controle para impedir publicações simultâneas.
 */
static bool publish_in_progress = false;

// ========================
// CALLBACKS DE ASSINATURA E DADOS
// ========================

/**
 * @brief Callback chamado ao identificar o tópico de uma nova mensagem recebida.
 *
 * Armazena o nome do tópico para uso posterior no callback de dados.
 */
static void mqtt_message_cb(void *arg, const char *topic, u32_t tot_len)
{
    strncpy(received_topic, topic, sizeof(received_topic) - 1);
    received_topic[sizeof(received_topic) - 1] = '\0';
}

/**
 * @brief Callback chamado com os dados de uma mensagem recebida.
 *
 * Se o tópico for `TOPICO_CONFIG_INTERVALO`, interpreta o payload como um número inteiro
 * e envia via FIFO ao núcleo 0 para atualizar o intervalo do PING.
 */
static void mqtt_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    if (strncmp(received_topic, TOPIC_CONFIG_INTERVAL, strlen(TOPIC_CONFIG_INTERVAL)) == 0)
    {
        char buffer[16] = {0};
        memcpy(buffer, data, len < sizeof(buffer) - 1 ? len : sizeof(buffer) - 1);
        uint32_t new_value = (uint32_t)atoi(buffer);

        if (new_value >= 1000 && new_value <= 60000)
        {
            multicore_fifo_push_blocking((0xABCD << 16) | (new_value & 0xFFFF));
        }
    }
    else if (strncmp(received_topic, TOPIC_IRRIGATION, strlen(TOPIC_IRRIGATION)) == 0)
    {
        // Trata o comando de irrigação ON/OFF enviado via MQTT
        char comando[8] = {0};
        memcpy(comando, data, len < sizeof(comando) - 1 ? len : sizeof(comando) - 1);

        if (strcmp(comando, "ON") == 0)
        {
            multicore_fifo_push_blocking((0xB1B1 << 16) | 1);
        }
        else if (strcmp(comando, "OFF") == 0)
        {
            multicore_fifo_push_blocking((0xB1B1 << 16) | 0);
        }
    }
}

/**
 * @brief Callback chamado após tentativa de assinatura de um tópico.
 *
 * Exibe uma mensagem de sucesso ou falha no terminal.
 */
static void mqtt_sub_cb(void *arg, err_t result)
{
    if (result == ERR_OK)
    {
        printf("[MQTT] Tópico assinado com sucesso.\n");
    }
    else
    {
        printf("[MQTT] Falha ao assinar tópico. Código: %d\n", result);
    }
}

// ========================
// CALLBACKS DE CONEXÃO E PUBLICAÇÃO
// ========================

/**
 * @brief Callback chamado após tentativa de conexão com o broker MQTT.
 *
 * Se a conexão for aceita, registra os callbacks de entrada e assina os tópicos necessários.
 */
void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    if (status == MQTT_CONNECT_ACCEPTED)
    {
        display_mqtt_status("CONECTADO");

        mqtt_set_inpub_callback(client, mqtt_message_cb, mqtt_data_cb, NULL);

        mqtt_subscribe(client, TOPIC_CONFIG_INTERVAL, 0, mqtt_sub_cb, NULL);
        mqtt_subscribe(client, TOPIC_IRRIGATION, 0, mqtt_sub_cb, NULL);

        // publicar_mensagem_mqtt(TOPICO_ONLINE, "Pico W online");
        publish_online = true;
    }
    else
    {
        display_mqtt_status("FALHA");
    }
}

/**
 * @brief Callback chamado após o término de uma publicação MQTT.
 *
 * Envia via FIFO para o núcleo 0 um status de sucesso (0) ou erro (1),
 * com código de controle 0x9999.
 */
static void mqtt_pub_cb(void *arg, err_t result)
{
    publish_in_progress = false;

    printf("[MQTT] Publicação finalizada: %s\n", result == ERR_OK ? "OK" : "ERRO");

    uint16_t status = (result == ERR_OK) ? 0 : 1;
    uint32_t packet = ((0x9999 << 16) | status);
    multicore_fifo_push_blocking(packet);
}

// ========================
// FUNÇÕES PRINCIPAIS
// ========================

/**
 * @brief Inicializa e conecta o cliente MQTT ao broker.
 *
 * Converte o IP do broker, instancia o cliente e realiza a conexão
 * com os parâmetros configurados.
 */
void start_mqtt_client()
{
    ip_addr_t broker_ip;

    if (!ip4addr_aton(MQTT_BROKER_IP, &broker_ip))
    {
        printf("Endereço IP do broker inválido: %s\n", MQTT_BROKER_IP);
        return;
    }

    mqtt_client = mqtt_client_new();
    if (!mqtt_client)
    {
        printf("Erro ao criar cliente MQTT\n");
        return;
    }

    memset(&client_info, 0, sizeof(client_info));
    client_info.client_id = "pico_lwip";

    mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT, mqtt_connection_cb, NULL, &client_info);
}

/**
 * @brief Publica uma mensagem MQTT de forma segura.
 *
 * Verifica se o cliente está conectado e se nenhuma publicação anterior está pendente.
 * Usa `publicacao_em_andamento` como trava para evitar sobrecarga da fila TCP.
 *
 * @param topico  Nome do tópico a ser publicado.
 * @param mensagem  Conteúdo textual a ser enviado.
 */
void publish_mqtt_message(const char *topic, const char *message)
{
    if (!mqtt_client)
    {
        printf("[MQTT] Cliente NULL\n");
        display_mqtt_status("CLIENTE NULL");
        return;
    }

    if (!mqtt_client_is_connected(mqtt_client))
    {
        printf("[MQTT] MQTT desconectado. Ignorando publicação.\n");
        display_mqtt_status("DESCONECTADO");
        return;
    }

    if (publish_in_progress)
    {
        printf("[MQTT] Publicação anterior ainda não finalizada. Ignorado.\n");
        return;
    }

    err_t err = mqtt_publish(mqtt_client,
                             topic,
                             message,
                             strlen(message),
                             0, // QoS
                             0, // retain
                             mqtt_pub_cb,
                             NULL);

    if (err != ERR_OK)
    {
        printf("[MQTT] Erro ao publicar: %d\n", err);
        display_mqtt_status("PUB ERRO");
    }
    else
    {
        printf("[MQTT] Publicando: \"%s\" em \"%s\"\n", message, topic);
        publish_in_progress = true;
    }
}

/**
 * @brief Função reservada para manutenções futuras no cliente MQTT.
 *
 * Pode ser usada para reconexões, manutenção ativa ou ping manual.
 */
void mqtt_loop()
{
    // Espaço reservado para verificações futuras
}

void publish_online_retain(void)
{
    if (!mqtt_client || !mqtt_client_is_connected(mqtt_client))
    {
        printf("[MQTT] Cliente não conectado. Não foi possível publicar 'Pico W online'.\n");
        return;
    }

    const char *message = "Pico W online";

    err_t err = mqtt_publish(mqtt_client,
                             TOPIC_ONLINE,
                             message,
                             strlen(message),
                             0, // QoS 0
                             1, // retain = 1 ✅
                             mqtt_pub_cb,
                             NULL);

    if (err != ERR_OK)
    {
        printf("[MQTT] Falha ao publicar 'Pico W online': código %d\n", err);
        display_mqtt_status("PUB ERRO");
    }
    else
    {
        printf("[MQTT] 'Pico W online' publicado com retain.\n");
    }
}

bool is_mqtt_client_active(void)
{
    return mqtt_client && mqtt_client_is_connected(mqtt_client);
}
