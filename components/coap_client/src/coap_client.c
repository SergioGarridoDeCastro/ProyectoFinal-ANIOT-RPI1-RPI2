#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "coap_client.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "cJSON.h"

#include "nvs_flash.h"

#include "protocol_examples_common.h"
#include "coap3/coap.h"

#ifndef CONFIG_COAP_CLIENT_SUPPORT
#error COAP_CLIENT_SUPPORT needs to be enabled
#endif /* COAP_CLIENT_SUPPORT */

#define URI_PATH "/api/v1/TytItM7fbBqz6ZIuxvjU/telemetry"
#define COAP_URI_LENTGH 64
ESP_EVENT_DEFINE_BASE(OWN_COAP);
// prueba de conexión con el servidor CoAP
// #define PAYLOAD "{\"temp\": 25.5}"
static const char *TAG = "CoAP_Client";
static coap_session_t *session;
static coap_context_t *ctx;
// Declaraciones de las funciones de envío de datos CoAP para los sensores - integración
static coap_address_t *
coap_get_address(coap_uri_t *uri);

static void send_coap_message_sgp30(coap_context_t *ctx, coap_session_t *session, cJSON *valor_sensor);

cJSON *simular_datos_sgp30();
cJSON *simular_datos_si7021();

char *get_coap_uri_path();

cJSON *simular_datos_sgp30()
{
    cJSON *datos = cJSON_CreateObject();
    cJSON_AddNumberToObject(datos, "co2", 400); // Valor ficticio para CO2
    cJSON_AddNumberToObject(datos, "voc", 150); // Valor ficticio para VOC
    return datos;
}

cJSON *simular_datos_si7021()
{
    cJSON *datos = cJSON_CreateObject();
    cJSON_AddNumberToObject(datos, "humedad", 50);       // Valor ficticio para la humedad
    cJSON_AddNumberToObject(datos, "temperatura", 22.5); // Valor ficticio para la temperatura
    return datos;
}

char *get_coap_uri_path()
{
    char *uri_path = (char *)malloc(COAP_URI_LENTGH * sizeof(char));

    if (!uri_path)
        return uri_path;

    sprintf(uri_path, "api/v1/%s/telemetry", CONFIG_ACCESS_TOKEN);
    return uri_path;
}

// Funciones para enviar datos CoAP
void send_coap_message(float sensor_value)
{
    cJSON *datos = cJSON_CreateObject();
    cJSON_AddNumberToObject(datos, "SENSOR_TEMP", sensor_value);
    char *payload = cJSON_Print(datos);
    char *uri_path = get_coap_uri_path();

    coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_POST, coap_new_message_id(session), coap_session_max_pdu_size(session));
    if (!pdu)
    {
        ESP_LOGE(TAG, "No se puede crear PDU");
        return;
    }

    coap_add_option(pdu, COAP_OPTION_URI_PATH, strlen(uri_path), (const uint8_t *)uri_path);
    coap_add_data(pdu, strlen(payload), (const uint8_t *)payload);

    if (coap_send(session, pdu) == COAP_INVALID_MID)
    {
        ESP_LOGE(TAG, "No se puede enviar el PDU de CoAP"); // Usando URI_PATH definido
    }
    free(uri_path);
    free(payload);
    cJSON_Delete(datos);
    coap_io_process(ctx, 1000);
}

void send_coap_message_si7021(coap_context_t *ctx, coap_session_t *session, cJSON *datos_sensor)
{
    char *payload = cJSON_Print(datos_sensor);
    char *uri_path = get_coap_uri_path();
    ESP_LOGE(TAG, "Uri path %s", uri_path);
    coap_pdu_t *pdu = coap_pdu_init(COAP_MESSAGE_CON, COAP_REQUEST_CODE_POST, coap_new_message_id(session), coap_session_max_pdu_size(session));
    if (!pdu)
    {
        ESP_LOGE(TAG, "No se puede crear PDU");
        return;
    }

    coap_add_option(pdu, COAP_OPTION_URI_PATH, strlen(uri_path), (const uint8_t *)uri_path);
    coap_add_data(pdu, strlen(payload), (const uint8_t *)payload);

    if (coap_send(session, pdu) == COAP_INVALID_MID)
    {
        ESP_LOGE(TAG, "No se puede enviar el PDU de CoAP");
    }
    free(uri_path);
    free(payload);
}

static coap_address_t *coap_get_address(coap_uri_t *uri)
{
    static coap_address_t dst_addr;
    char *phostname = NULL;
    struct addrinfo hints;
    struct addrinfo *addrres;
    int error;
    char tmpbuf[INET6_ADDRSTRLEN];

    phostname = (char *)calloc(1, uri->host.length + 1);
    if (phostname == NULL)
    {
        ESP_LOGE(TAG, "No se pudo asignar memoria para hostname");
        return NULL;
    }
    memcpy(phostname, uri->host.s, uri->host.length);

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    error = getaddrinfo(phostname, NULL, &hints, &addrres);
    if (error != 0)
    {
        ESP_LOGE(TAG, "Error en getaddrinfo: %s", strerror(errno));
        free(phostname);
        return NULL;
    }
    if (addrres == NULL)
    {
        ESP_LOGE(TAG, "getaddrinfo retornó NULL");
        free(phostname);
        return NULL;
    }
    free(phostname);

    coap_address_init(&dst_addr);
    switch (addrres->ai_family)
    {
    case AF_INET:
        memcpy(&dst_addr.addr.sin, addrres->ai_addr, sizeof(dst_addr.addr.sin));
        dst_addr.addr.sin.sin_port = htons(uri->port);
        break;
    case AF_INET6:
        memcpy(&dst_addr.addr.sin6, addrres->ai_addr, sizeof(dst_addr.addr.sin6));
        dst_addr.addr.sin6.sin6_port = htons(uri->port);
        break;
    default:
        freeaddrinfo(addrres);
        return NULL;
    }
    freeaddrinfo(addrres);

    return &dst_addr;
}

void connect_coap()
{
    int error = -1;
    ESP_ERROR_CHECK(example_connect());
    coap_uri_t uri;

    if (coap_split_uri((const uint8_t *)CONFIG_EXAMPLE_TARGET_DOMAIN_URI, strlen(CONFIG_EXAMPLE_TARGET_DOMAIN_URI), &uri) == -1)
    {
        error = 1;
        ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_ERROR, (void *)error, sizeof(error), portMAX_DELAY));
        return;
    }

    coap_address_t *dst_addr = coap_get_address(&uri);
    if (dst_addr == NULL)
    {
        error = 2;
        ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_ERROR, (void *)error, sizeof(error), portMAX_DELAY));
        return;
    }

    ctx = coap_new_context(NULL);
    if (!ctx)
    {
        error = 3;
        ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_ERROR, (void *)error, sizeof(error), portMAX_DELAY));
        return;
    }

    session = coap_new_client_session(ctx, NULL, dst_addr, COAP_PROTO_UDP);
    if (!session)
    {
        error = 4;
        ESP_LOGE(TAG, "No se puede crear la sesión del cliente");
        coap_free_context(ctx);
        ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_ERROR, (void *)error, sizeof(error), portMAX_DELAY));
        return;
    }
    ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_CONNECTED, NULL, NULL, portMAX_DELAY));
}

void disconnect_coap()
{
    coap_session_release(session);
    coap_free_context(ctx);
    coap_cleanup();
    ESP_ERROR_CHECK(example_disconnect());
    ESP_ERROR_CHECK(esp_event_post(OWN_COAP, OWN_COAP_DISCONNECTED, NULL, NULL, portMAX_DELAY));
}

void coap_work()
{

    ESP_ERROR_CHECK(example_connect());
    coap_uri_t uri;

    if (coap_split_uri((const uint8_t *)CONFIG_EXAMPLE_TARGET_DOMAIN_URI, strlen(CONFIG_EXAMPLE_TARGET_DOMAIN_URI), &uri) == -1)
    {
        ESP_LOGE(TAG, "URI no válida");
        return;
    }

    coap_address_t *dst_addr = coap_get_address(&uri);
    if (dst_addr == NULL)
    {
        ESP_LOGE(TAG, "No se puede obtener la dirección");
        return;
    }

    coap_context_t *ctx = coap_new_context(NULL);
    if (!ctx)
    {
        ESP_LOGE(TAG, "No se puede crear el contexto de CoAP");
        return;
    }

    coap_session_t *session = coap_new_client_session(ctx, NULL, dst_addr, COAP_PROTO_UDP);
    if (!session)
    {
        ESP_LOGE(TAG, "No se puede crear la sesión del cliente");
        coap_free_context(ctx);
        return;
    }

    while (1)
    {
        // send_coap_message(ctx, session);
        // send_coap_message_sgp30(ctx, session);
        // pruebas simulación de conexión con el servidor CoAP
        cJSON *datos_sgp30 = simular_datos_sgp30();
        send_coap_message_sgp30(ctx, session, datos_sgp30);
        cJSON_Delete(datos_sgp30);

        cJSON *datos_si7021 = simular_datos_si7021();
        send_coap_message_si7021(ctx, session, datos_si7021);
        cJSON_Delete(datos_si7021);

        coap_io_process(ctx, 1000); // Procesa eventos CoAP por 1 segundo
        printf("entro");
        vTaskDelay(60000 / portTICK_PERIOD_MS); // Espera 60 segundos antes de enviar el siguiente mensaje
    }

    coap_session_release(session);
    coap_free_context(ctx);
    coap_cleanup();
    ESP_ERROR_CHECK(example_disconnect());
}
