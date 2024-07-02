set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/examples/common_components/protocol_examples_common)
idf_component_register(SRCS "src/coap_client.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_event esp_netif esp_wifi esp-tls protocol_examples_common nvs_flash json)
