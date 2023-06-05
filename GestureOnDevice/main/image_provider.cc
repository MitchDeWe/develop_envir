
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "app_camera_esp.h"
#include "esp_camera.h"
#include "model_settings.h"
#include "image_provider.h"
#include "esp_main.h"

static const char* TAG = "app_camera";

static uint16_t *display_buf; // buffer to hold data to be sent to display

// Initialise the camera
TfLiteStatus InitCamera() {

if (display_buf == NULL) {
  display_buf = (uint16_t *) heap_caps_malloc(172 * 172 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}
if (display_buf == NULL) {
  ESP_LOGE(TAG, "Couldn't allocate display buffer");
  return kTfLiteError;
}

  int ret = app_camera_init();
  if (ret != 0) {
    MicroPrintf("Camera init failed\n");
    return kTfLiteError;
  }
  MicroPrintf("Camera Initialized\n");
  return kTfLiteOk;
}

void *image_provider_get_display_buf()
{
  return (void *) display_buf;
}

// Get an image from the camera module
TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data) {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    ESP_LOGE(TAG, "Camera capture failed");
    return kTfLiteError;
  }

  // getting the data from the upper 172x172 in the 176*244 frame as this is closest to the desired size
  int frameOfCapture = 244;
  for (int i = 0; i < kNumRows; i++) {
    for (int j = 0; j < kNumCols; j++) {
      uint16_t pixel = ((uint16_t *) (fb->buf))[i * frameOfCapture + j];

      // for inference
      uint8_t hb = pixel & 0xFF;
      uint8_t lb = pixel >> 8;

      // Make the values floats as this is the necessary value for the model input
      float r = static_cast< float > ((lb & 0x1F) << 3) / static_cast< float >(255);
      float g = static_cast< float > (((hb & 0x07) << 5) | ((lb & 0xE0) >> 3)) / static_cast< float >(255);
      float b = static_cast< float > ((hb & 0xF8)) / static_cast< float >(255);

      //image_data[i * kNumCols + j] = [r,g,b];

      // to display
      display_buf[i * kNumCols + j] = pixel;
    }
  }

  esp_camera_fb_return(fb);
  /* here the esp camera can give you grayscale image directly */
  return kTfLiteOk;
}
