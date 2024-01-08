# Embed the server root certificate into the final binary
<<<<<<< HEAD
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "station_example_main.c" "app_main.c" "wifi_provisioning.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi 
                    )
=======
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS ".")
>>>>>>> ANIOT
