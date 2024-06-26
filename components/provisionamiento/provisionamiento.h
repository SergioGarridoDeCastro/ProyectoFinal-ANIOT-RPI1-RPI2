#ifndef PROVISIONAMIENTO
#define PROVISIONAMIENTO

#include "esp_event.h"

typedef struct {
    char* wifi_ssid;
    char* wifi_pass;
    char* data_piso;
    char* data_aula;
    char* data_url;
} prov_info_t;

enum {
    PROVISION_DONE,
    PROVISION_ERROR
};

ESP_EVENT_DECLARE_BASE(PROVISION_EVENT);

esp_err_t init_provision(void *event_handler);
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen, uint8_t **outbuf, ssize_t *outlen, void *priv_data);
prov_info_t* get_wifi_info();
void start_provisioning();
void stop_provisioning();


#endif