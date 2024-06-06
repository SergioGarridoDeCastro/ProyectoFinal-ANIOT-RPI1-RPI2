#ifndef MQTT_API
#define MQTT_API
#include <stdint.h>
#include "esp_err.h"  // Incluir el encabezado para esp_err_t
#include <esp_event_base.h>
#include "cJSON.h"

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
static void notify_node_event(const char *event);
void suscribe_topic_control(void); // Corregí el nombre de la función
static void publish_data_sgp30(int piso, int aula, int numero, cJSON *valor_sensor);
static void publish_data_si7021(int piso, int aula, int numero, cJSON *valor_sensor);
static void publish_lwt(void);
static void log_error_if_nonzero(const char *message, int error_code);
static void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t init_publisher_mqtt (void *event_handler, char *device_token, char *cert);

/* Inicia el componente MQQT. Recibe como parámetro 
   el event handler para los eventos que postea.
esp_err_t mqtt_init(void *mqtt_handler, char *device_token, char *server_cert);

esp_err_t mqtt_start();
esp_err_t mqtt_deinit();
esp_err_t mqtt_stop();
esp_err_t mqtt_send(char *topic, char *data, int qos);
esp_err_t mqtt_subscribe(char *topic);
esp_err_t mqtt_unsubscribe(char *topic);*/


#endif  // MQTT_API