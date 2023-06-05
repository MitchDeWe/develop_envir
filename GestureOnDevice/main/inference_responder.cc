#include "inference_responder.h"
#include "tensorflow/lite/micro/micro_log.h"

#include "esp_main.h"
#include "image_provider.h"
#include "esp_lcd.h"

static QueueHandle_t xQueueLCDFrame = NULL;

void RespondToInference(int colour, int action_guess, int countdown) {

  if (xQueueLCDFrame == NULL) {
    xQueueLCDFrame = xQueueCreate(2, sizeof(struct lcd_frame));
    register_lcd(xQueueLCDFrame, NULL, false);
  }


  app_lcd_colour_for_inference(colour, countdown, action_guess);
  // display frame (freed by lcd task)
  lcd_frame_t *frame = (lcd_frame_t *) malloc(sizeof(lcd_frame_t));
  frame->width = 172;
  frame->height = 172;
  frame->buf = image_provider_get_display_buf();
  xQueueSend(xQueueLCDFrame, &frame, portMAX_DELAY);
}
