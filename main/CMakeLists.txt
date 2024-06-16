# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "station_example_main.c" "main.c"
                    INCLUDE_DIRS "." 
                    REQUIRES esp_wifi  driver wifi_provisioning nvs_flash muestradora
                    )
component_compile_options(-Wno-error=format= -Wno-format) #para eliminar error log_color 
