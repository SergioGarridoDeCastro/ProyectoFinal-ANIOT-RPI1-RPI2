/* esp_event (event loop library) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "event_source.h"
#include "esp_event_base.h"
#include "muestradora.h"
#include "i2c_config.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_pm.h"
#include <time.h>
#include <sys/time.h>
#include <portmacro.h>
#include <sntp.h>
#include <esp_netif_sntp.h>
#include <esp_err.h>
#include "scan_gap_ble.h"
#include "mqtt.h"


static const char *TAG = "user_event_loops";

//Definicion de la maquina de estados 
typedef enum{
    STATE_INIT,
    STATE_PROVISIONING_WIFI,
    STATE_PROVISIONING_THINGSBOARD,
    STATE_MONITORITATION,
    STATE_TRANSMISION,
    STATE_LOW_POWER,
    STATE_OTA
}state_machine_t;

//El estado inicial se considera STATE_INIT
static state_machine_t current_state = STATE_INIT;

esp_event_loop_handle_t loop_with_task;
esp_event_loop_handle_t loop_without_task;
esp_event_loop_handle_t loop_monitor_main;
esp_event_loop_handle_t loop_wifi;
esp_event_loop_handle_t loop_scan_ble;

QueueHandle_t callback_queue;
QueueHandle_t temperature_queue;


//Variables para topics MQTT
int piso;
int aula;
int numero;
cJSON *valor_sgp30;
CborValue valor_si7021;

//Variables datos muestreados
float temperature;
float humidity;
float co2;

//Handler para la monitorizacion
void monitorize_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    char msg[64];
    temperature = *((float *)(event_data));
    snprintf(msg, sizeof msg, "%f", temperature);
    ESP_LOGI(TAG, "Temp %s", msg);
}

void states_machine()
{
    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Main task");
    }
}

static int initialize_sntp(void){
        ESP_LOGI(TAG, "Initializing SNTP");
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("poolntp.org");
        //config.timezone = 1; //UTC+1 para Madrid
        //config.smooth_sync = true;
        //config.sync_cb = time_sync_notification_cb;
        esp_err_t sntp_init_result = esp_netif_sntp_init(&config);
        if(sntp_init_result != ESP_OK){
            ESP_LOGE(TAG, "SNTP initialization failed with error %d", sntp_init_result);
            return sntp_init_result;
        }
        // Configurar la zona horaria (UTC+1 para Madrid)
    esp_sntp_set_timezone(1);
        return ESP_OK;
}

static void obtain_time(void){
    if(initialize_sntp() != ESP_OK){
        ESP_LOGE(TAG, "SNTP initialization failed");
        return;
    }

    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    time_t now;

    while(esp_netif_sntp_sync_wait(portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT &&  ++retry < retry_count ){
        ESP_LOGI(TAG, "Esperando... (%d/%d)", retry, retry_count);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

/*
// handler para el wifi disconected
static void event_loop_task_mqtt(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "SE LANZA EL EVENTO WIFI DESCONECTADO DESDE COMPONENT Y SE GUARDA EN LA COLA DE EVENTOS \n");
    xQueueSend(callback_queue,&id,0);
}*/

//
static void mqtt_transport_data_task(){
    piso = CONFIG_PISO_NODO;
    aula = CONFIG_AULA_NODO;
    numero = CONFIG_NUMERO_NODO;

    valor_sgp30 = cJSON_CreateNumber(co2);

    // Crear un objeto CBOR 
    CborEncoder encoder;
    CborEncoder mapEncoder;
    uint8_t cborBuffer[64];  // Ajusta el tamaño según tus necesidades

    // Inicializar el codificador CBOR
    cbor_encoder_init(&encoder, cborBuffer, sizeof(cborBuffer), 0);
    cbor_encoder_create_map(&encoder, &mapEncoder, 2);
    cbor_encode_text_stringz(&mapEncoder, "Temperature");
    cbor_encode_float(&mapEncoder, temperature);

    cbor_encode_text_stringz(&mapEncoder, "Humidity");
    cbor_encode_float(&mapEncoder, humidity);

    // Finalizar la codificación del mapa CBOR
    cbor_encoder_close_container(&encoder, &mapEncoder);

    // Obtener el tamaño del objeto CBOR serializado
    size_t cborSize = cbor_encoder_get_buffer_size(&encoder, cborBuffer);

    // Asignar el mapa CBOR al objeto CborValue
    cbor_value_advance_fixed(&valor_si7021);  // Mueve el CborValue al siguiente valor
    cbor_value_enter_container(&valor_si7021, &mapEncoder);

    publish_data_sgp30(piso, aula, numero, valor_sgp30);
    publish_data_si7021(piso, aula, numero, valor_si7021);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

static void provisioning_task(){
    init_ap_wifi();
    init_publisher_mqtt(); //Inicia el provisionamiento
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

// handler para el wifi conectado
static void event_loop_task_scan_ble(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    
    //float sensor_temperature = *((float*) event_data);

    char* loop;

    if (handler_args == loop_scan_ble) {
        loop = "loop_with_task";
    } else {
        loop = "loop_without_task";
    }

    ESP_LOGI(TAG, "SE LANZA EL EVENTO WIFI CONECTADO DESDE COMPONENT Y SE GUARDA EN LA COLA DE EVENTOS \n");
    //xQueueSend(temperature_queue,&sensor_temperature,0);
    xQueueSend(callback_queue,&id,0);

}

void init_machine(){
    loop_scan_ble = init_ble();
    ESP_ERROR_CHECK(esp_event_handler_register_with(loop_scan_ble, SCAN_BLE, SCAN_BLE_EVENT_DEVICE_FOUND, event_loop_task_scan_ble, loop_scan_ble));

    switch(current_state){
        case STATE_INIT:
            xTaskCreate(provisioning_task, "provisioning", 4096, NULL, tskIDLE_PRIORITY, NULL);
            break;
        case STATE_OTA:
            break;
        case STATE_MONITORITATION:

            break;
        case STATE_LOW_POWER:

            break;
        case STATE_TRANSMISION:
#ifdef CONFIG_PROTCOL_TRANSPORT_MQTT_PROTOCOL
            xTaskCreate(mqtt_transport_data_task, "mqtt_transport_data_task", 4096, NULL, tskIDLE_PRIORITY, NULL);
#elif  CONFIG_PROTCOL_TRANSPORT_COAP_PROTOCOL
            xTaskCreate(mqtt_transport_data_task, "mqtt_transport_data_task", 4096, NULL, tskIDLE_PRIORITY, NULL);
#endif
            break;
    }
}

void app_main(void)
{
    //Iniciar OTA    

    ESP_LOGI(TAG, "setting up");
    int64_t t_after_us;
    int64_t time_sleep;
    double temp;

    // init temperature controller
    i2c_master_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(TEMP, TEMP_OBTAINED, monitorize_handler, NULL));

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    //Nota: No funciona con Eduroam
    time_t now;
    struct tm *timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    if(timeinfo->tm_year < (2024 - 1900)){
        ESP_LOG(TAG, "Time is not set yet, Connecting to WiFi and getting time over NTP");
        obtain_time();
        time(&now);
    }
    //Provisionamiento WiFi
    init_ap_wifi(); //¿Crear tarea?

    //Provisionamiento
    init_publisher_mqtt();

    //OTA

    //Escaneo BLE periodicamente

    //Monitorizacion periodicamente

    //Envio de datos periodicamente

    //Inicializa el servidor REST
    //ESP_ERROR_CHECK(start_rest_server(CONFIG_EXAMPLE_WEB_MOUNT_POINT));

    ESP_ERROR_CHECK(err);
    muestradora(1000000);
    xTaskCreate(states_machine, "states_machine", 4096, NULL, tskIDLE_PRIORITY, NULL);
}