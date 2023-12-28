set(srcs "src/api_rest.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_server)