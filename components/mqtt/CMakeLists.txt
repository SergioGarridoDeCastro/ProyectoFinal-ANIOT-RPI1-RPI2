set(srcs "src/mqtt.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include/"
                       REQUIRES esp_wifi esp_timer esp_event i2c_config esp-si7021 driver esp32_SGP30 nvs_flash protocol_examples_common)

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
