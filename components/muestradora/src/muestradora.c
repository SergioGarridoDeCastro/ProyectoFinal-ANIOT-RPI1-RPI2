#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "i2c_config.h"
#include "muestradora.h"
#include "si7021.h"
ESP_EVENT_DEFINE_BASE(TEMP);

// static const char *TAG = "Muestradora";
static void temperature_every_sec(void *arg);
static float temperature = 0.0;
static int pause = 0;
esp_timer_handle_t periodic_timer;

static void temperature_every_sec(void *arg)
{
    readTemperature(I2C_MASTER_NUM, &temperature);
    if (pause == 0)
        ESP_ERROR_CHECK(esp_event_post(TEMP, TEMP_OBTAINED, (void *)&temperature, sizeof(temperature), portMAX_DELAY));
}

void muestradora(int period)
{

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &temperature_every_sec,
        .name = "periodic"};

    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, CONFIG_PERIOD_TEMPERATURE * 1000000);
}

void muestradora_stop_timer()
{
    esp_timer_stop(periodic_timer);
    esp_timer_delete(periodic_timer);
}

void muestradora_pause(int out)
{
    pause = out;
}