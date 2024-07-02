set(srcs "src/own_sntp.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include"
                       REQUIRES  driver esp_event esp_netif esp_wifi protocol_examples_common)
