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

static const char *TAG = "user_event_loops";

void monitorize_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{

    char msg[64];
    float temp = *((float *)(event_data));
    snprintf(msg, sizeof msg, "%f", temp);
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

static void initialize_sntp(void){
        ESP_LOGI(TAG, "Initializing SNTP");
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("poolntp.org");
        config.timezone = 1; //UTC+1 para Madrid
        //config.smooth_sync = true;
        //config.sync_cb = time_sync_notification_cb;
        esp_netif_sntp_init(&config);
}

static void obtain_time(void){
    if(initialize_sntp() != ESP_OK{
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

void app_main(void)
{
    ESP_LOGI(TAG, "setting up");
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

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime(&now, &timeinfo);

    if(timeinfo.tm_year < (2024 - 1900)){
        ESP_LOG(TAG, "Time is not set yet, Connecting to WiFi and getting time over NTP");
        obtain_time();
        time(&now);
    }

    ESP_ERROR_CHECK(err);
    muestradora(1000000);
    xTaskCreate(states_machine, "states_machine", 4096, NULL, tskIDLE_PRIORITY, NULL);
}