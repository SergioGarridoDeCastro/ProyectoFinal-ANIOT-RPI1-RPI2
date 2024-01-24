#include <esp_event_base.h>
#include "cJSON.h"
#include <stdint.h>
#include "cbor.h"


const char *lwt_topic; //topic del mensaje LWT (Last Will and Testament)
const char *lwt_msg; //contenido del mensaje LWT.
int lwt_qos; //QoS del mensaje LWT.
int lwt_retain; //flag retain para el mensaje LWT.
int lwt_msg_len; //longitud del mensaje LWT.
int keepalive; //valor del temporizador de keepalive (por defecto 120 segundos).


static void handle_mqtt_configure(const char *topic, const char *datos);
static void mqtt_configure_callback(const char *topic, const char *datos);
static void notify_node_event(const char *event);
static void suscribe_topic_control();
static void publish_data_sgp30(int piso, int aula, int numero, cJSON valor_sensor);
static void publish_data_si7021(int piso, int aula, int numero, CborValue valor_sensor);
static void publish_lwt(void);
static void log_error_if_nonzero(const char *message, int error_code);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void init_publisher_mqtt (void);
