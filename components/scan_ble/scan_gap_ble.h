#ifndef SCAN_GAP_BLE_H
#define SCAN_GAP_BLE_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "bta_api.h"
#include "bt_trace.h"
#include "btc_manage.h"
#include <time.h>
#include "esp_log.h"
#include <esp_gatt_defs.h>
#include <esp_gattc_api.h>
#include <esp_event_base.h>
#include <esp_bt.h>
#include <esp_event.h>
#include <esp_timer.h>
#include "esp_event.h"
#include "esp_timer.h"

enum{
    SCAN_BLE_EVENT_DEVICE_FOUND
};
ESP_EVENT_DECLARE_BASE(SCAN_BLE);

/* Declare static functions */
static void esp_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

// Prototipos de funciones
esp_event_loop_handle_t init_ble(void);
double estimate_distance(int rssi);
double kalman_filter(int rssi);
void update_ble_devices_list(esp_bd_addr_t bd_addr, int rssi, int estimated_distance);
void forget_undetected_devices(void);
static void remove_device(int index);
static void scan_timer_callback(void *arg);
static int find_device_index(esp_bd_addr_t bd_addr);


#endif // #ifndef SCAN_GAP_BLE_H