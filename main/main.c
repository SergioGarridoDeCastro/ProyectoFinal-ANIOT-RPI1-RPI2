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
#include "nvs_component.h"
#include <string.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_pm.h"
#include <time.h>
#include <sys/time.h>
#include <portmacro.h>
#include <sntp.h>
#include "esp_sntp.h"
#include <esp_netif_sntp.h>
#include "mqtt_api.h"
#include "provisionamiento.h"
#include "wifi_station.h"
#include "scan_gap_ble.h"
#include "ota.h"
#include "protocol_examples_common.h"
static const char *TAG = "user_event_loops";
static char *DEVICE_TOKEN = NULL;

//Definicion de la maquina de estados 
typedef enum{
    STATE_INIT,
    STATE_PROVISIONED,
    STATE_MONITORITATION,
    STATE_TRANSMISION,
    STATE_LOW_POWER,
    STATE_OTA
}state_machine_t;

//Definicion de las transicciones entre estados
typedef enum{
    //Provisionamiento
    TRANSICION_PROVISION,

    TRANSICION_WIFI_CONECTADO,
    TRANSICION_WIFI_DESCONECTADO,

    //T
    TRANSICION_LECTURA_SENSORES,
    TRANSICION_TRANSMISION,
    TRANSICION_ESCANEO_BLE,
    TRANSICION_MODO_LOW_POWER,
} transicion_t;

//El estado inicial se considera STATE_INIT
static state_machine_t current_state = STATE_INIT;

esp_event_loop_handle_t loop_with_task;
esp_event_loop_handle_t loop_without_task;
esp_event_loop_handle_t loop_monitor_main;
esp_event_loop_handle_t loop_wifi;



//Handler para la monitorizacion
void monitorize_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    char msg[64];
    float temp = *((float *)(event_data));
    snprintf(msg, sizeof msg, "%f", temp);
    ESP_LOGI(TAG, "Temp %s", msg);
    switch (id)
    {
    case SENSOR_TEMP:
        ESP_LOGI(TAG, "SENSOR_TEMP %s", msg);
        break;
    case SENSOR_ECO2:
        ESP_LOGI(TAG, "SENSOR_ECO2 %s", msg);
        break;
    case SENSOR_TVOC:
        ESP_LOGI(TAG, "SENSOR_TVOC %s", msg);
        break;
    default:
        break;
    }
}

//Handler para el provisionamiento
void provisionamiento_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data){

}

//Handler para el wifi
void wifi_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data){
    if(base == WIFI_EVENT){
        switch (id) {

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "IP ACQUIRED\n");
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                //trans.tipo = TRANS_WIFI_DISCONECT;
                //xQueueSend(fsm_queue, &trans, portMAX_DELAY);
                ESP_LOGI(TAG, "WIFI disconnected\n");
                break;

            default:
                ESP_LOGE("WIFI_HANDLER", "Evento desconocido.");
        }
    }
    else if (base == IP_EVENT) {
        TAG = "IP_HANDLER";
        switch (id) {
            
            case IP_EVENT_STA_GOT_IP:
                //xQueueSend(fsm_queue, &trans, portMAX_DELAY);
                ESP_LOGI(TAG, "IP ACQUIRED\n");
                break;

            default:
                ESP_LOGE("IP_HANDLER", "Evento desconocido.");
        }
    }
}


static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static int initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.smooth_sync = true;
    config.sync_cb = time_sync_notification_cb;
    
    esp_err_t sntp_init_result = esp_netif_sntp_init(&config);
    if (sntp_init_result != ESP_OK) {
        ESP_LOGE(TAG, "SNTP initialization failed with error %d", sntp_init_result);
        return sntp_init_result;
    }

    // Esperar a que la sincronización sea exitosa
    int retry = 0;
    const int retry_count = 10;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count) {
        ESP_LOGI(TAG, "Esperando sincronización de tiempo... (%d/%d)", retry, retry_count);
    }

    if (retry == retry_count) {
        ESP_LOGE(TAG, "Sincronización de tiempo fallida");
        esp_netif_sntp_deinit();
        return ESP_ERR_TIMEOUT;
    }

    // Configurar zona horaria
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Configuración para UTC+1 y horario de verano
    tzset();

    esp_netif_sntp_deinit();
    return ESP_OK;
}

static void obtain_time(void){
    if(initialize_sntp() != ESP_OK){
        ESP_LOGE(TAG, "SNTP initialization failed");
        return;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "La hora actual es: %s", asctime(&timeinfo));
}

void states_machine()
{
   /* state_machine_t estado_actual = STATE_INIT;
    if(init_provisioning(provisionamiento_handler) != ESP_OK){
        ESP_LOGE(TAG, "Fallo en el provisionamiento");
        return;
    }*/
    while (1)
    {   
        //transicion_t transicion;
        switch (current_state)
        {
        case STATE_INIT:
            ESP_ERROR_CHECK(wifi_init_sta(wifi_handler));
            ESP_LOGI(TAG, "WiFi sta iniciado");
            ESP_ERROR_CHECK(init_provisioning(provisionamiento_handler));
            ESP_LOGI(TAG, "Provisionamiento realizado");
            vTaskDelay(5000);
            current_state = STATE_PROVISIONED;
            break;
        case STATE_PROVISIONED:
            prov_info_t* info_wifi = get_wifi_info();
            ESP_ERROR_CHECK(wifi_connect(info_wifi->wifi_ssid, info_wifi->wifi_pass));
            //init_ble();
            ESP_LOGI(TAG, "Conectado a %s", info_wifi->wifi_ssid);
            obtain_time();
            #if CONFIG_USE_MQTT_PROTOCOL
                init_mqtt();
                start_publisher();
                ESP_LOGI(TAG, "Publisher MQTT iniciado correctamente");
            #endif
            current_state = STATE_MONITORITATION;
            break;
        case STATE_MONITORITATION:
            //double distancia = estimacion_de_aforo();
            ESP_LOGI(TAG, "BLE iniciado correctamente");
            current_state = STATE_TRANSMISION;
            break;
        case STATE_TRANSMISION:
            #if CONFIG_USE_COAP_PROTOCOL
                //Envio Coap
                ESP_LOGI(TAG, "Transmision por COAP");
            #elif CONFIG_USE_MQTT_PROTOCOL
                
                ESP_LOGI(TAG, "Transmision por MQTT");
                char * topic = "test/topic";
                subscribe(topic);
                ESP_LOGI(TAG, "Suscrito a topic %s", topic);
                publish(topic, "Test ESP32");
        
                //publish_data_si7021(1,2, 3, valorSensor);
                //publish_data_sgp30(1,2, 3, valorSensor);
                //publish("", aforo);
            #else
                ESP_LOGE(TAG, "Protocolo de transporte no soportado");
            #endif
            current_state = STATE_LOW_POWER;
            break;
        case STATE_LOW_POWER:
            break;
        case STATE_OTA:
            break;
        default:
            break;
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Main task");
    }
}


/*
// handler para el wifi disconected
static void event_loop_task_mqtt(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    ESP_LOGI(TAG, "SE LANZA EL EVENTO WIFI DESCONECTADO DESDE COMPONENT Y SE GUARDA EN LA COLA DE EVENTOS \n");
    xQueueSend(callback_queue,&id,0);
}*/



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
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR, SENSOR_TEMP, monitorize_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR, SENSOR_ECO2, monitorize_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SENSOR, SENSOR_TVOC, monitorize_handler, NULL));
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(esp_netif_init());
    esp_pm_config_t config_power_mode = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true,
    };
    esp_pm_configure(&config_power_mode);
    ESP_ERROR_CHECK(err);
    //    muestradora(1000000);

    
    //ESP_ERROR_CHECK(wifi_connect(CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD));
    

    //check_updates();


    /*
    #if CONFIG_USE_MQTT_PROTOCOL
        //Iniciación MQTT. Se le pasa el handler de los eventos
        //MQTT para que se registre.
        init_mqtt();
        ESP_ERROR_CHECK(err);
        start_publisher();
    #else
        ESP_LOGE(TAG, "Protocolo mal configurado");
        return;
    #endif*/
    
    xTaskCreate(states_machine, "states_machine", 4096, NULL, tskIDLE_PRIORITY, NULL);
}