# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "station_example_main.c" "app_main.c" "coap.c" "ble_provisioning.c" "wifi_provisioning.c"
                    INCLUDE_DIRS "."
                    )