# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "api_rest.c" "app_main.c" "mqtt_publisher.c" "coap.c"
                    INCLUDE_DIRS "."
                    )