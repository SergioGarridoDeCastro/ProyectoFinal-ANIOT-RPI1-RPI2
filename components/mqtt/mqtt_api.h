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
esp_err_t init_publisher_mqtt (void *event_handler, char *device_token, char *cert);
void init_mqtt();
esp_err_t deinit_publisher_mqtt();
esp_err_t start_publisher();
esp_err_t stop_publisher();
static void notify_node_event(const char *event);
static void suscribe_topic_provisioning();
static void publish_data_sgp30(int piso, int aula, int numero, cJSON *valor_sensor);
static void publish_data_si7021(int piso, int aula, int numero, cJSON *valor_sensor);
static void publish_lwt(void);
esp_err_t publish(char *topic, char *data);
static void log_error_if_nonzero(const char *message, int error_code);
static void mqtt_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
esp_err_t subscribe(char *topic);
esp_err_t unsubscribe(char *topic);
void handle_mqtt_configure(const char *topic, const char *datos);
void mqtt_configure_callback(const char *topic, const char *datos);
static esp_err_t parse_received_device_token_thingsboard(char *response_tb, int response_tb_len);
static void reset_mqtt_client();

void suscribe_topic_control(void); // Corregí el nombre de la función






#endif  // MQTT_API