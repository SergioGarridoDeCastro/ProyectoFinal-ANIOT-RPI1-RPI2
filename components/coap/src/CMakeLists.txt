# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "coap_client.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi cjson
                    #EMBED_TXTFILES certs/coap_ca.pem certs/coap_client.crt certs/coap_client.key
                    )
