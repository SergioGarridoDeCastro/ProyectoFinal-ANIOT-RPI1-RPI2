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
#include <mqtt_client.h>

static esp_mqtt_client_handle_t mqtt_client;
static int sampling_frequency = 30;
static bool is_provisioned = false;
const char *access_token;

#define PROVISIONING_TOPIC "v1/gateway/control/node_provisioning"
#define SAMPLING_FREQUENCY_TOPIC "v1/gateway/configure/frequency"

static const char *TAG = "MQTT";

//Para añadir SSL/TLS sobre MQTT
#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t node_cert_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t node_cert_pem_start[]   asm("_binary_node_cert_pem_start");
#endif
extern const uint8_t mnode_cert_pem_end[]   asm("_binary_node_cert_pem_end");

static void send_binary(esp_mqtt_client_handle_t cliente){
    esp_partition_mmap_handle_t out_handle;
    const void *binary_address;
    const esp_partition_t *partition;
    esp_partition_mmap(partition, 0, partition->size, ESP_PARTITION_MMAP_DATA, &binary_address, &out_handle);
    // sending only the configured portion of the partition (if it's less than the partition size)
    int binary_size = MIN(CONFIG_BROKER_BIN_SIZE_TO_SEND, partition->size);
    int msg_id = esp_mqtt_client_publish(cliente, "/topic/binary", binary_address, binary_size, 0, 0);
    ESP_LOGI(TAG, "binary sent with msg_id=%d", msg_id);
}



//Funcion para manejar los comandos remotos a traves de MQTT
static void handle_mqtt_configure(const char *topic, const char *datos){
    if(strcmp(topic, SAMPLING_FREQUENCY_TOPIC) == 0){     
        (TAG, "Setting sampling frequency for topic %s to %f", topic, sampling_frequency);
        sscanf(datos, "%d", &sampling_frequency);
    }
    else{

    }
}


static void mqtt_configure_callback(const char *topic, const char *datos){
    handle_mqtt_configure(topic, datos);
}

static void notify_node_event(const char *event){
    if (strcmp(event, "Node activated") == 0) {
        ESP_LOGI(TAG, "Node Event: %s", event);        
    } else if (strcmp(event, "Node disconnected") == 0) {
        ESP_LOGI(TAG, "Node Event: %s", event);
    } else if (strcmp(event, "Low battery") == 0) {
        ESP_LOGI(TAG, "Node Event: %s", event);
    } else if (strcmp(event, "Error reading sensor") == 0) {
        ESP_LOGI(TAG, "Node Event: %s", event);
    } else if (strcmp(event, "CO2 high") == 0) {
        ESP_LOGI(TAG, "Node Event: %s", event);
    } else {
        // Caso por defecto o manejo de eventos adicionales
    }
}

/**
 * Funcion para suscribirse al topic de provisionamiento
*/
static void suscribe_topic_provisioning(){
    esp_mqtt_client_subscribe(mqtt_client, PROVISIONING_TOPIC, CONFIG_QOS_MQTT);
}

/**
 * Funcion para provisionamiento de Thingsboard por medio de MQTT
*/
static void public_provisioning_message(){
    cJSON *provision_data = cJSON_CreateObject();
    cJSON_AddStringToObject(provision_data, "deviceName", CONFIG_DEVICE_NAME);
    cJSON_AddStringToObject(provision_data, "provisionDeviceKey", CONFIG_DEVICE_PROVISION_KEY);
    cJSON_AddStringToObject(provision_data, "provisionDeviceSecret", CONFIG_DEVICE_PROVISION_SECRET);
    cJSON_AddStringToObject(provision_data, "credentialsType", CONFIG_ACCESS_TOKEN);
    cJSON_AddStringToObject(provision_data, "token", CONFIG_DEVICE_ACCESS_TOKEN);

    char *provisioning_request = cJSON_PrintUnformatted(provision_data);

    if(provisioning_request != NULL){
        esp_mqtt_client_publish(mqtt_client, PROVISIONING_TOPIC, provisioning_request, 
            sizeof(provisioning_request), CONFIG_QOS_MQTT, CONFIG_RETAIN_MQTT);
        free(provisioning_request);
    }
    else{
        ESP_LOGE(TAG, "Error al crear mensaje de provisionamiento");
    }
}

static void handle_provisioning_response(const char *response) {
    cJSON *json_response = cJSON_Parse(response);

    if (json_response != NULL) {
        const char *status = cJSON_GetObjectItemCaseSensitive(json_response, "provisionDeviceStatus")->valuestring;
        const char *credentials_type = cJSON_GetObjectItemCaseSensitive(json_response, "credentialsType")->valuestring;

        if (strcmp(status, "SUCCESS") == 0) {
            if (strcmp(credentials_type, "ACCESS_TOKEN") == 0) {
                access_token = cJSON_GetObjectItemCaseSensitive(json_response, "accessToken")->valuestring;
                ESP_LOGI(TAG, "Provisioning successful. Access Token: %s", access_token);
            } else {
                ESP_LOGE(TAG, "Unsupported credentials type: %s", credentials_type);
            }
        } else {
            ESP_LOGE(TAG, "Provisioning failed. Status: %s", status);
        }

        // Liberar la memoria asignada por cJSON_Parse
        cJSON_Delete(json_response);
    } else {
        ESP_LOGE(TAG, "Error al parsear la respuesta JSON de aprovisionamiento");
    }
}

static char* getAccessToken(){
    return access_token;
}

/***
 * Funcion para publicar los datos del sensor SGP30
*/
static void publish_data_sgp30(int piso, int aula, int numero, cJSON *valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/SGP30", piso, aula, numero);

    // Convertir el objeto cJSON a una cadena de caracteres
    char *json_data = cJSON_PrintUnformatted(valor_sensor);
    if (json_data != NULL) {
        // Publicar los datos JSON directamente
        esp_mqtt_client_publish(mqtt_client, (const char *)topic, json_data, strlen(json_data), CONFIG_QOS_MQTT, CONFIG_RETAIN_MQTT);

        // Liberar la memoria asignada por cJSON_PrintUnformatted
        free(json_data);
    } else {
        ESP_LOGE(TAG, "Error al convertir");
    }
}

/***
 * Funcion para publicar los datos del sensor Si7021
*/
static void publish_data_si7021(int piso, int aula, int numero, CborValue valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/Si7021", piso, aula, numero);
    // Crear un búfer para almacenar el CBOR serializado
    uint8_t cbor_buffer[512];  // Ajusta el tamaño según tus necesidades

    // Crear un objeto CBOR en el búfer
    CborEncoder encoder;
    cbor_encoder_init(&encoder, cbor_buffer, sizeof(cbor_buffer), 0);
    cbor_encode_(&encoder, valor_sensor);

    // Obtener el tamaño del CBOR serializado
    size_t cbor_size = cbor_encoder_get_buffer_size(&encoder, cbor_buffer);
    esp_mqtt_client_publish(mqtt_client,(const char *) topic, (const char *) &cbor_buffer, cbor_size, CONFIG_QOS_MQTT, CONFIG_RETAIN_MQTT);
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

    switch ((event_id)){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
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

            if (strncmp(event->data, "send binary please", event->data_len) == 0) {
                ESP_LOGI(TAG, "Sending the binary");
                send_binary(client);
            }
            if(!is_provisioned){
                is_provisioned = true;
                // Manejar la respuesta de aprovisionamiento
                handle_provisioning_response((const char *)event->data);
                break;
            }
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
        .network.reconnect_timeout_ms = CONFIG_RECONNECT_TIMEOUT,
        .network.transport = MQTT_TRANSPORT_OVER_SSL, // Habilita el transporte seguro
        .credentials.authentication.certificate = node_cert_pem_start /* Certificado PEM para la conexión segura */,
        //.client_cert_pem = (const unsigned char *) node_cert_pem_start,
        //.client_key_pem = (const unsigned char *)node_key_pem_start
    };

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_config.broker.address.uri, "FROM_STDIN") == 0) {
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
        mqtt_config.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */


    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    // Realizar el provisionamiento
    perform_provisioning();
}