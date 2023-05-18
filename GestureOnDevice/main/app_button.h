#pragma once
#include "esp_event.h"
#include "driver/gpio.h"
#include "esp_log.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct
    {
        int button_index; 
        int min;          
        int max;          
    } button_adc_config_t;

    void register_adc_button(const QueueHandle_t key_state_o);

#ifdef __cplusplus
}
#endif