
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "cJSON.h"
#include "cbor.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t mqtt_client;
const char *lwt_topic; //topic del mensaje LWT (Last Will and Testament)
const char *lwt_msg; //contenido del mensaje LWT.
int lwt_qos; //QoS del mensaje LWT.
int lwt_retain; //flag retain para el mensaje LWT.
int lwt_msg_len; //longitud del mensaje LWT.
int keepalive; //valor del temporizador de keepalive (por defecto 120 segundos).


//Funcion para manejar los comandos remotos a traves de MQTT
static void handle_mqtt_commands(const char *topic, const char *datos, cJSON valor_sensor){

}

//Funcion para notificar la de activación/caídas de nodos y otros eventos de interes
static void notify_node_event(const char *event){

}

static void publish_data_sgp30(int piso, int aula, int numero, cJSON valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/SGP30", piso, aula, numero);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &valor_sensor, 0, 1, 0);
}

static void publish_data_si7021(int piso, int aula, int numero, cJSON valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/Si7021", piso, aula, numero);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &valor_sensor, 0, 1, 0);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base = %s, event_id = %d", base, event_id);

    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        notify_node_event("Node activated");
        break;
    case MQTT_EVENT_DISCONNECTED:
        notify_node_event("Node disconnected");
        break;
    case MQTT_EVENT_SUBSCRIBED:

        break;
    case MQTT_EVENT_UNSUBSCRIBED:

        break;
    case MQTT_EVENT_PUBLISHED:

        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void init_publisher_mqtt (void){
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
        .network.reconnect_timeout_ms = CONFIG_RECONNECT_TIMEOUT,
    };

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.broker.address.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */


    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}