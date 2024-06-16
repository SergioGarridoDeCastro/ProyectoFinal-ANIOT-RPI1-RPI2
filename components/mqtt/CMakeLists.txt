idf_component_register(SRCS "mqtt_api.c"
                       INCLUDE_DIRS "."
                       REQUIRES mqtt esp_event esp_timer espressif__cbor nvs_flash json protocol_examples_common tcp_transport qrcode
                       EMBED_TXTFILES node_cert.pem)
                       
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/esp-mqtt/include)
# Incluir la ruta del componente MQTT explícitamente
include_directories($ENV{IDF_PATH}/components/mqtt/esp-mqtt/include)

component_compile_options(-Wno-error=format= -Wno-format) #para eliminar error log_color 