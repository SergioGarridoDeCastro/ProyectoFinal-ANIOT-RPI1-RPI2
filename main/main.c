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
#include <sntp.h>
#include "ota.h"
#include "coap_client.h"
#include "protocol_examples_common.h"
static const char *TAG = "user_event_loops";

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

void states_machine()
{

    while (1)
    {
        vTaskDelay(10000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Main task");
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "setting up");
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
    esp_pm_config_esp32_t config_power_mode = {
        .max_freq_mhz = 240,
        .min_freq_mhz = 80,
        .light_sleep_enable = true,
    };
    // esp_pm_configure(&config_power_mode);
    ESP_ERROR_CHECK(err);

    // muestradora(1000000);
    ota_work();
    // coap_work();
    xTaskCreate(states_machine, "states_machine", 4096, NULL, tskIDLE_PRIORITY, NULL);
}