idf_component_register(SRCS "provisionamiento.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_event nvs_flash esp_wifi wifi_provisioning  espressif__qrcode)
