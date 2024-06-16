#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "cJSON.h"

#include "nvs_flash.h"

#include "protocol_examples_common.h"
#include "coap3/coap.h"

static const char *TAG = "CoAP_Client";

// Declaraciones de las funciones de envío de datos CoAP para los sensores - integración
static coap_address_t *coap_get_address(coap_uri_t *uri);
static void send_coap_message_sgp30(coap_context_t *ctx, coap_session_t *session, cJSON *valor_sensor);
static void send_coap_message_si7021(coap_context_t *ctx, coap_session_t *session, cJSON *valor_sensor);
cJSON *simular_datos_sgp30();
cJSON *simular_datos_si7021();
void start_client();
