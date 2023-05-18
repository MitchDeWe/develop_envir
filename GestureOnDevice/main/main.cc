#include "main_functions.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_main.h"
TaskHandle_t main_task_handle;

void tf_main(void) {
  setup();
  while (true) {
    loop();
  }
}

extern "C" void app_main() {

  xTaskCreatePinnedToCore((TaskFunction_t)&tf_main, "tf_main", 4 * 1024, NULL, configMAX_PRIORITIES - 2, &main_task_handle, 1);
  vTaskDelete(NULL);
}
