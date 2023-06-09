#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc_common.h"
#include "esp_adc_cal.h"
#include "app_button.h"

//ADC Channels
#define ADC1_CHAN0 ADC1_CHANNEL_0
//ADC Attenuation
#define ADC_ATTEN ADC_ATTEN_DB_11
//ADC Calibration
#define ADC_EF ESP_ADC_CAL_VAL_EFUSE_TP_FIT


#define PRESS_INTERVAL 500000

static uint32_t voltage = 0;

static const char *TAG = "ADC SINGLE";

static esp_adc_cal_characteristics_t adc1_chars;

static button_adc_config_t adc_buttons[4] = {{1, 2800, 3000}, {2, 2250, 2450}, {3, 300, 500}, {4, 850, 1050}};
int adc_button_num = 4;
static QueueHandle_t xQueueKeyStateO = NULL;


static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EF);
    if (ret == ESP_ERR_NOT_SUPPORTED)
    {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    }
    else if (ret == ESP_ERR_INVALID_VERSION)
    {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    }
    else if (ret == ESP_OK)
    {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid arg");
    }
    return cali_enable;
}

static void adc_button_task(void *arg)
{
    int last_button_pressed = -2;
    int button_pressed = -1;
    int64_t backup_time = esp_timer_get_time();
    int64_t last_time = esp_timer_get_time();

    //ADC1 config
    //adc_calibration_init();
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHAN0, ADC_ATTEN));

    while (1)
    {
        voltage = adc1_get_raw(ADC1_CHAN0);
        backup_time = esp_timer_get_time();
        for (int i = 0; i < adc_button_num; ++i)
        {
            if ((voltage >= adc_buttons[i].min) && (voltage <= adc_buttons[i].max))
            {
                button_pressed = adc_buttons[i].button_index;
                if ((button_pressed != last_button_pressed) || ((backup_time - last_time) > PRESS_INTERVAL))
                {
                    last_button_pressed = button_pressed;
                    last_time = backup_time;
                    xQueueOverwrite(xQueueKeyStateO, &button_pressed);
                    break;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void register_adc_button(const QueueHandle_t key_state_o)
{
    xQueueKeyStateO = key_state_o;
    xTaskCreatePinnedToCore(adc_button_task, "adc_button_scan_task", 3 * 1024, NULL,configMAX_PRIORITIES - 2 , NULL, 0);
}