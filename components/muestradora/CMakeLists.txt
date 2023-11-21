set(srcs "src/muestradora.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include"
                       REQUIRES esp_timer esp_event i2c_config esp-si7021 driver esp32_SGP30)
