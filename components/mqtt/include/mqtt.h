#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H
#include <esp_event_base.h>
#include "cJSON.h"
#include <stdint.h>
#include "cbor.h"

/*
const char *lwt_topic; //topic del mensaje LWT (Last Will and Testament)
const char *lwt_msg; //contenido del mensaje LWT.
int lwt_qos; //QoS del mensaje LWT.
int lwt_retain; //flag retain para el mensaje LWT.
int lwt_msg_len; //longitud del mensaje LWT.
int keepalive; //valor del temporizador de keepalive (por defecto 120 segundos).
*/

void handle_mqtt_configure(const char *topic, const char *datos);
void mqtt_configure_callback(const char *topic, const char *datos);
void notify_node_event(const char *event);
void suscribe_topic_control(void); // Corregí el nombre de la función
void publish_data_sgp30(int piso, int aula, int numero, cJSON *valor_sensor);
void publish_data_si7021(int piso, int aula, int numero, CborValue valor_sensor);
void publish_lwt(void);
void log_error_if_nonzero(const char *message, int error_code);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void init_publisher_mqtt(void);

#endif  // MQTT_PUBLISHER_H