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

#include "detection_responder.h"
#include "tensorflow/lite/micro/micro_log.h"

#include "esp_main.h"
#include "image_provider.h"
#include "esp_lcd.h"

static QueueHandle_t xQueueLCDFrame = NULL;

void RespondToDetection(int colour, int action_guess, int countdown) {

  if (xQueueLCDFrame == NULL) {
    xQueueLCDFrame = xQueueCreate(2, sizeof(struct lcd_frame));
    register_lcd(xQueueLCDFrame, NULL, false);
  }


  app_lcd_colour_for_detection(colour, countdown, action_guess);
  // display frame (freed by lcd task)
  lcd_frame_t *frame = (lcd_frame_t *) malloc(sizeof(lcd_frame_t));
  frame->width = 172;
  frame->height = 172;
  frame->buf = image_provider_get_display_buf();
  xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
}
