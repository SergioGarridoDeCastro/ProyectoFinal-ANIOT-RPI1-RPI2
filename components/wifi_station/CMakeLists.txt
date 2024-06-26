idf_component_register(SRCS "wifi_station.c"
                    INCLUDE_DIRS "."
                    REQUIRES driver esp_event esp_wifi nvs_flash) 
