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
#include "mqtt.h"

#include <cjson/cJSON.h>
#include "cbor.h"

static esp_mqtt_client_handle_t mqtt_client;
static int sampling_frequency = 30;

#define PROVISIONING_TOPIC "v1/gateway/control/node_provisioning"
#define SAMPLING_FREQUENCY_TOPIC "v1/gateway/configure/frequency"

//Funcion para manejar los comandos remotos a traves de MQTT
static void handle_mqtt_configure(const char *topic, const char *datos){
    if(strcmp(topic, SAMPLING_FREQUENCY_TOPIC) == 0){
        ESP_LOGI(TAG, "Setting sampling frequency for topic %s to %f", topic, frequency);
        sscanf(datos, "%d", &sampling_frequency);
    }
    else if(strcmp(topic, PROVISIONING_TOPIC) == 0){ 
        ESP_LOGI(TAG, "Provisioning node for topic %s to %f", topic, frequency);
        
    }
    else{

    }
}


static void mqtt_configure_callback(const char *topic, const char *datos){
    handle_mqtt_configure(topic, datos);
}

//Funcion para notificar la de activación/caídas de nodos y otros eventos de interes
static void notify_node_event(const char *event){
    
}

/***
 * Funcion para suscribirse al topic de control
*/
static void suscribe_topic_control(){
    esp_mqtt_client_subscribe(mqtt_client, PROVISIONING_TOPIC, 0);
}

static void topic_control_callback(const char *topic, const char *payload){}

/***
 * Funcion para publicar los datos del sensor SGP30
*/
static void publish_data_sgp30(int piso, int aula, int numero, cJSON valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/SGP30", piso, aula, numero);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &valor_sensor,  0, CONFIG_QOS, 0);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &valor_sensor,  0, CONFIG_QOS, 0);
}

/***
 * Funcion para publicar los datos del sensor Si7021
*/
static void publish_data_si7021(int piso, int aula, int numero, cJSON valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/Si7021", piso, aula, numero);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &valor_sensor, 0, 1, 0);
}

static void publish_lwt(void){
    esp_mqtt_client_publish(mqtt_client, CONFIG_LWT_TOPIC, CONFIG_LWT_MESSAGE, 0, 
            CONFIG_LWT_QOS, CONFIG_LWT_RETAIN);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base = %s, event_id = %d", base, event_id);

    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            suscribe_topic_control();
            suscribe_topic_control();
            notify_node_event("Node activated");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            notify_node_event("Node disconnected");
            publish_lwt();
            publish_lwt();
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            //Se llama a mqtt_configure_callback
            mqtt_configure_callback((const char *) event->topic, (const char *) event->data);
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);

            //Se llama a mqtt_configure_callback
            mqtt_configure_callback((const char *) event->topic, (const char *) event->data);
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
        //.network.reconnect_timeout_ms = CONFIG_RECONNECT_TIMEOUT,
        //.network.reconnect_timeout_ms = CONFIG_RECONNECT_TIMEOUT,
        .transport = MQTT_TRANSPORT_OVER_SSL, // Habilita el transporte seguro
        //.cert_pem = /* Certificado PEM para la conexión segura */
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