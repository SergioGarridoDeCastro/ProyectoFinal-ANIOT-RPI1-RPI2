# Embed CA, certificate & key directly into binary
idf_component_register(SRCS "coap_client.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES certs/coap_ca.pem certs/coap_client.crt certs/coap_client.key
                    INCLUDE_DIRS "/Users/jamess/mbedtls/include"
                    )
