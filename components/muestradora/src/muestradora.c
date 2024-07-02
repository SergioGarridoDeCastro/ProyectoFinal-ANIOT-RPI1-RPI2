#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "i2c_config.h"
#include "muestradora.h"
#include "si7021.h"
#include "SGP30.h"
ESP_EVENT_DEFINE_BASE(SENSOR);

// static const char *TAG = "Muestradora";
static void temperature_every_sec(void *arg);
static float temperature = 0.0;
static int pause = 0;
esp_timer_handle_t periodic_timer;
sgp30_dev_t main_sgp30_sensor;
static const char *TAG = "MUESTRADORA";
/*!< I2C master doesn't need buffer */

i2c_port_t i2c_num = I2C_MASTER_NUM;

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */

/**
 * @brief generic function for reading I2C data
 *
 * @param reg_addr register adress to read from
 * @param reg_data pointer to save the data read
 * @param len length of data to be read
 * @param intf_ptr
 *
 * >init: dev->intf_ptr = &dev_addr;
 *
 * @return ESP_OK if reading was successful
 */
int8_t main_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{                   // *intf_ptr = dev->intf_ptr
    int8_t ret = 0; /* Return 0 for Success, non-zero for failure */

    if (len == 0)
    {
        return ESP_OK;
    }

    uint8_t chip_addr = *(uint8_t *)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);

    if (reg_addr != 0xff)
    {
        i2c_master_write_byte(cmd, (chip_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
        i2c_master_start(cmd);
    }

    i2c_master_write_byte(cmd, (chip_addr << 1) | I2C_MASTER_READ, ACK_CHECK_EN);

    if (len > 1)
    {
        i2c_master_read(cmd, reg_data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, reg_data + len - 1, NACK_VAL);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return ret;
}

/**
 * @brief generic function for writing data via I2C
 *
 * @param reg_addr register adress to write to
 * @param reg_data register data to be written
 * @param len length of data to be written
 * @param intf_ptr
 *
 * @return ESP_OK if writing was successful
 */
int8_t main_i2c_write(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    /* Return 0 for Success, non-zero for failure */
    int8_t ret = 0;

    uint8_t chip_addr = *(uint8_t *)intf_ptr;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (chip_addr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);

    if (reg_addr != 0xFF)
    {
        i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    }

    i2c_master_write(cmd, reg_data, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);

    i2c_cmd_link_delete(cmd);

    return ret;
}

static void temperature_every_sec(void *arg)
{
    readTemperature(I2C_MASTER_NUM, &temperature);
    if (pause == 0)
    {
        ESP_ERROR_CHECK(esp_event_post(SENSOR, SENSOR_TEMP, (void *)&temperature, sizeof(temperature), portMAX_DELAY));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sgp30_IAQ_measure(&main_sgp30_sensor);

        ESP_LOGI(TAG, "TVOC: %d,  eCO2: %d", (int)main_sgp30_sensor.TVOC, (int)main_sgp30_sensor.eCO2);
        ESP_ERROR_CHECK(esp_event_post(SENSOR, SENSOR_ECO2, (void *)&main_sgp30_sensor.eCO2, sizeof(main_sgp30_sensor.eCO2), portMAX_DELAY));
        ESP_ERROR_CHECK(esp_event_post(SENSOR, SENSOR_TVOC, (void *)&main_sgp30_sensor.TVOC, sizeof(main_sgp30_sensor.TVOC), portMAX_DELAY));
    }
}
void muestradora()
{

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &temperature_every_sec,
        .name = "periodic"};

    sgp30_init(&main_sgp30_sensor, (sgp30_read_fptr_t)main_i2c_read, (sgp30_write_fptr_t)main_i2c_write);

    // SGP30 needs to be read every 1s and sends TVOC = 400 14 times when initializing
    for (int i = 0; i < 14; i++)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sgp30_IAQ_measure(&main_sgp30_sensor);
        ESP_LOGI(TAG, "SGP30 Calibrating... TVOC: %d,  eCO2: %d", main_sgp30_sensor.TVOC, main_sgp30_sensor.eCO2);
    }

    // Read initial baselines
    uint16_t eco2_baseline, tvoc_baseline;
    sgp30_get_IAQ_baseline(&main_sgp30_sensor, &eco2_baseline, &tvoc_baseline);
    ESP_LOGI(TAG, "BASELINES - TVOC: %d,  eCO2: %d", tvoc_baseline, eco2_baseline);

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