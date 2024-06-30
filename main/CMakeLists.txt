# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "." 
                    REQUIRES esp_wifi  driver wifi_provisioning nvs_flash muestradora nvs_component ota mqtt provisionamiento wifi_station scan_gap_ble
                    )

# Añadir los directorios adicionales donde se encuentran los archivos de encabezado necesarios
set(EXTRA_COMPONENT_DIRS 
    $ENV{IDF_PATH}/components/spi_flash/sim/stubs/freertos/include/freertos 
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/riscv/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/linux/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/portable/xtensa/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/portable/riscv/include/freertos
)

# Incluir los directorios adicionales donde se encuentran los archivos de encabezado necesarios
target_include_directories(${COMPONENT_LIB} PRIVATE 
    $ENV{IDF_PATH}/components/spi_flash/sim/stubs/freertos/include/freertos 
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/riscv/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel/portable/linux/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/portable/xtensa/include/freertos
    $ENV{IDF_PATH}/components/freertos/FreeRTOS-Kernel-SMP/portable/riscv/include/freertos
)


component_compile_options(-Wno-error=format= -Wno-format) #para eliminar error log_color 
