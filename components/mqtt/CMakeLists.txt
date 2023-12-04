set(srcs "src/mqtt.c")

idf_component_register(SRCS ${srcs}
                       INCLUDE_DIRS "include")
