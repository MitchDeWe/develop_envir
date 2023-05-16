
/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
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

// Get the camera module ready
TfLiteStatus InitCamera() {

// if display support is present, initialise display buf
#if DISPLAY_SUPPORT
  if (display_buf == NULL) {
    display_buf = (uint16_t *) heap_caps_malloc(172 * 172 * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  }
  if (display_buf == NULL) {
    ESP_LOGE(TAG, "Couldn't allocate display buffer");
    return kTfLiteError;
  }
#endif

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

#if DISPLAY_SUPPORT
  // getting the data from the upper 172x172 in the 240*240 frame as this is closest to the desired size
  int frameOfCapture = 240;
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

      
      // The format is rgb so correct the rgb565 to rgb888
      //int8_t grey_pixel = ((305 * r + 600 * g + 119 * b) >> 10) - 128;

      //image_data[i * kNumCols + j] = grey_pixel;

      // to display
      display_buf[i * kNumCols + j] = pixel;
      //display_buf[2 * i * kNumCols * 2 + 2 * j + 1] = pixel;
      //display_buf[(2 * i + 1) * kNumCols * 2 + 2 * j] = pixel;
      //display_buf[(2 * i + 1) * kNumCols * 2 + 2 * j + 1] = pixel;
    }
  }
  #endif

  esp_camera_fb_return(fb);
  /* here the esp camera can give you grayscale image directly */
  return kTfLiteOk;
}
