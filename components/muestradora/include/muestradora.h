#ifndef MUESTRADORA_H_
#define MUESTRADORA_H_

#include "esp_event.h"

ESP_EVENT_DECLARE_BASE(TEMP); // declaration of the task events family
enum
{
    TEMP_OBTAINED,
};

void muestradora(int period);
void muestradora_stop_timer();
void muestradora_pause(int out);
#endif
