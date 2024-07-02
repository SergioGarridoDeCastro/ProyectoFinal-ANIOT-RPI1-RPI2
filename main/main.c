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
#include <sys/time.h>
#include "own_sntp.h"
#include "ota.h"
#include "coap_client.h"
#include "protocol_examples_common.h"

static const char *TAG = "user_event_loops";

enum states
{
    INITIALIZATION,
    CHECK_UPDATES,
    CONNECT_COAP,
    COAP_CONNECTED,
};
enum states machine_state;
static int send_data = -1;

void monitorize_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    if (send_data == -1)
        return;
    char msg[64];
    float value = *((float *)(event_data));
    snprintf(msg, sizeof msg, "%f", value);
    ESP_LOGI(TAG, "Temp %s", msg);
    switch (id)
    {
    case SENSOR_TEMP:

        send_coap_message(value);

        ESP_LOGI(TAG, "SENSOR_TEMP %s", msg);
        break;
    case SENSOR_ECO2:

        send_coap_message(value);

        ESP_LOGI(TAG, "SENSOR_ECO2 %s", msg);
        break;
    case SENSOR_TVOC:

        send_coap_message(value);

        break;
    default:
        break;
    }
}

void coap_handler(void *handler_arg, esp_event_base_t base, int32_t id, void *event_data)
{
    switch (id)
    {
    case OWN_COAP_ERROR:
        int error = *((int *)(event_data));
        ESP_LOGI(TAG, "COAP_ERROR %d", error);
        break;
    case OWN_COAP_CONNECTED:
        ESP_LOGI(TAG, "COAP_CONNECTED");
        machine_state = COAP_CONNECTED;
        break;
    case OWN_COAP_DISCONNECTED:
        ESP_LOGI(TAG, "COAP_DISCONNECTED");
        break;
    default:
        break;
    }
}

void states_machine()
{

    while (1)
    {
        switch (machine_state)
        {
        case INITIALIZATION:
            obtain_time();
            machine_state = CHECK_UPDATES;
            break;
        case CHECK_UPDATES:
            check_updates();
            machine_state = CONNECT_COAP;
            muestradora();
            break;
        case CONNECT_COAP:
            connect_coap();
            break;
        case COAP_CONNECTED:
            send_data = 1;
            break;
        default:
            break;
        }
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
    ESP_ERROR_CHECK(esp_event_handler_register(OWN_COAP, OWN_COAP_ERROR, coap_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(OWN_COAP, OWN_COAP_CONNECTED, coap_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(OWN_COAP, OWN_COAP_DISCONNECTED, coap_handler, NULL));
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
    machine_state = INITIALIZATION;
    xTaskCreate(states_machine, "states_machine", 4096, NULL, tskIDLE_PRIORITY, NULL);
}