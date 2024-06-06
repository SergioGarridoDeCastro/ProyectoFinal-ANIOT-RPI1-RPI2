#include "mqtt_api.h"
#include "mqtt_client.h"
#include "cJSON.h"
//#include "cbor.h"
#include <esp_event.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "esp_partition.h"


//#ifdef CONFIG_USE_MQTT

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t cliente_mqtt  = NULL;
static void *mqtt_event_handler = NULL;
static int sampling_frequency = 30;
static bool is_provisioned = false;
const char *access_token;

#define PROVISIONING_TOPIC "v1/gateway/control/node_provisioning"
#define SAMPLING_FREQUENCY_TOPIC "v1/gateway/configure/frequency"
#define MIN(a, b) ((a) < (b) ? (a) : (b))




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

//Funcion para iniciar MQTT
esp_err_t init_publisher_mqtt(void *event_handler, char *device_token, char *cert){
    esp_err_t error;
    char url[256];

    if(sniprintf(url, sizeof(url), "mqtts://%s", CONFIG_BROKER_URL) > sizeof(url)){
        ESP_LOGE(TAG, "La url del broker MQTT es demasiado larga");
        return ESP_ERR_INVALID_SIZE;
    }

    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = url,
        .broker.verification.certificate  = (const char *) cert,
        .credentials.username = CONFIG_MQTT_USERNAME,
        .credentials.authentication.password = CONFIG_MQTT_PASSWORD,
        .network.reconnect_timeout_ms = CONFIG_RECONNECT_TIMEOUT,
        //.network.transport = MQTT_TRANSPORT_OVER_SSL, // Habilita el transporte seguro
        //.credentials.authentication.certificate = node_cert_pem_start, /* Certificado PEM para la conexión segura */
        //.client_cert_pem = (const unsigned char *) node_cert_pem_start,
        //.client_key_pem = (const unsigned char *)node_key_pem_start
    };



    cliente_mqtt = esp_mqtt_client_init(&mqtt_config);
    if (cliente_mqtt == NULL) {
        ESP_LOGE(TAG, "Error en esp_mqtt_client_init");
        return ESP_ERR_INVALID_ARG;
    }
    error = esp_mqtt_client_register_event(cliente_mqtt, ESP_EVENT_ANY_ID, event_handler, NULL);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Error en esp_mqtt_client_register_event: %s", esp_err_to_name(error));
        return error;
    }
    esp_mqtt_client_start(cliente_mqtt);
    mqtt_event_handler = event_handler;

    return ESP_OK; 
    // Realizar el provisionamiento
    //perform_provisioning();
}

esp_err_t deinit_publisher_mqtt(){
    esp_err_t error;
    error = esp_mqtt_client_unregister_event(cliente_mqtt, ESP_EVENT_ANY_ID, mqtt_event_handler);
        if (error != ESP_OK) {
        ESP_LOGE(TAG, "Error en esp_mqtt_client_unregister_event: %s", esp_err_to_name(error));
        return error;
    }
    mqtt_event_handler = NULL;
    

    error = esp_mqtt_client_destroy(cliente_mqtt);
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "Error en esp_mqtt_client_destroy: %s", esp_err_to_name(error));
        return error;
    }

    return ESP_OK;
}

//Funcion para comenzar la ejecucción del publicador MQTT 
esp_err_t start_publisher(){
    return esp_mqtt_client_start(cliente_mqtt);
}

//Funcion para parar la publicación del publicador MQTT 
esp_err_t stop_publisher(){
    return esp_mqtt_client_stop(cliente_mqtt);
}


//Funcion para manejar los comandos remotos a traves de MQTT
/*static void handle_mqtt_configure(const char *topic, const char *datos){
    if(strcmp(topic, SAMPLING_FREQUENCY_TOPIC) == 0){     
        (TAG, "Setting sampling frequency for topic %s to %f", topic, sampling_frequency);
        sscanf(datos, "%d", &sampling_frequency);
    }
    else{

    }
}*/


/*static void mqtt_configure_callback(const char *topic, const char *datos){
    handle_mqtt_configure(topic, datos);
}*/

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
    esp_mqtt_client_subscribe(cliente_mqtt, PROVISIONING_TOPIC, CONFIG_QOS_MQTT);
}

/**
 * Funcion para provisionamiento de Thingsboard por medio de MQTT
*/
/*static void public_provisioning_message(){
    cJSON *provision_data = cJSON_CreateObject();
    cJSON_AddStringToObject(provision_data, "deviceName", CONFIG_DEVICE_NAME);
    cJSON_AddStringToObject(provision_data, "provisionDeviceKey", CONFIG_DEVICE_PROVISION_KEY);
    cJSON_AddStringToObject(provision_data, "provisionDeviceSecret", CONFIG_DEVICE_PROVISION_SECRET);
    cJSON_AddStringToObject(provision_data, "credentialsType", CONFIG_ACCESS_TOKEN);
    cJSON_AddStringToObject(provision_data, "token", CONFIG_DEVICE_ACCESS_TOKEN);

    char *provisioning_request = cJSON_PrintUnformatted(provision_data);

    if(provisioning_request != NULL){
        esp_mqtt_client_publish(cliente_mqtt, PROVISIONING_TOPIC, provisioning_request, 
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
}*/

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
        esp_mqtt_client_publish(cliente_mqtt, (const char *)topic, json_data, strlen(json_data), CONFIG_QOS_MQTT, CONFIG_RETAIN_MQTT);

        // Liberar la memoria asignada por cJSON_PrintUnformatted
        free(json_data);
    } else {
        ESP_LOGE(TAG, "Error al convertir");
    }
}

/***
 * Funcion para publicar los datos del sensor Si7021
*/
static void publish_data_si7021(int piso, int aula, int numero, cJSON *valor_sensor){
    char topic[100];
    snprintf(topic, sizeof(topic), "facultad_informatica/piso_%d/aula_%d/%d/Si7021", piso, aula, numero);
    // Crear un búfer para almacenar el CBOR serializado
    uint8_t cbor_buffer[512];  // Ajusta el tamaño según tus necesidades

    // Crear un objeto CBOR en el búfer
    char *json_data = cJSON_PrintUnformatted(valor_sensor);
    if (json_data != NULL) {
        // Publicar los datos JSON directamente
        esp_mqtt_client_publish(cliente_mqtt, (const char *)topic, json_data, strlen(json_data), CONFIG_QOS_MQTT, CONFIG_RETAIN_MQTT);

        // Liberar la memoria asignada por cJSON_PrintUnformatted
        free(json_data);
    } else {
        ESP_LOGE(TAG, "Error al convertir");
    }
}

static void publish_lwt(void){
    esp_mqtt_client_publish(cliente_mqtt, CONFIG_LWT_TOPIC, CONFIG_LWT_MESSAGE, 0, 
            CONFIG_LWT_QOS, CONFIG_LWT_RETAIN);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    ESP_LOGD(TAG, "Event dispatched from event loop base = %s, event_id = %d", base, event_id);

    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((event_id)){
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            suscribe_topic_control();
            notify_node_event("Node activated");
            break;
        case MQTT_EVENT_DISCONNECTED:
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
                //handle_provisioning_response((const char *)event->data);
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

//#endif