idf_component_register(SRCS "mqtt_api.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp-mqtt mqtt esp_event esp_timer nvs_flash json protocol_examples_common tcp_transport qrcode
                       EMBED_TXTFILES node_cert.pem node_new_cert.pem node_key.pem)
                       
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/esp-mqtt/include)
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)

# Incluir la ruta del componente MQTT explícitamente
include_directories($ENV{IDF_PATH}/components/mqtt/esp-mqtt/include)

component_compile_options(-Wno-error=format= -Wno-format) #para eliminar error log_color 