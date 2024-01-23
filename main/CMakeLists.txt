# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "scan_gap_ble.c" "station_example_main.c" "main.c" "wifi_provisioning.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi 
                    )