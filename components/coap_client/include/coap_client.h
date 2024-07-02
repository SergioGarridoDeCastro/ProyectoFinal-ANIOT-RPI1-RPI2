#ifndef COAP_CLIENT_H_
#define COAP_CLIENT_H_

#include "esp_event.h"

#define COAP_ERROR_1 "URI no válida"
#define COAP_ERROR_2 "No se puede obtener la dirección"
#define COAP_ERROR_3 "No se puede crear el contexto de CoAP"
#define COAP_ERROR_4 "No se puede crear la sesión del cliente"

ESP_EVENT_DECLARE_BASE(OWN_COAP); // declaration of the task events family
enum
{
    OWN_COAP_CONNECTED,
    OWN_COAP_ERROR,
    OWN_COAP_DISCONNECTED,
};

void coap_work();
void connect_coap();
void disconnect_coap();
void send_coap_message(float sensor_value);
#endif