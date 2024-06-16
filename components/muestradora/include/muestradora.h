#ifndef MUESTRADORA_H_
#define MUESTRADORA_H_

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(SENSOR); // declaration of the task events family
enum
{
    SENSOR_TEMP,
    SENSOR_TVOC,
    SENSOR_ECO2
};

void muestradora(int period);
void muestradora_stop_timer();
void muestradora_pause(int out);
#endif
