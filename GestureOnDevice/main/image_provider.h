#ifndef IMAGE_PROVIDER_H_
#define IMAGE_PROVIDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_log.h"

// Returns buffer to be displayed
void *image_provider_get_display_buf();

TfLiteStatus GetImage(int image_width, int image_height, int channels, int8_t* image_data);

TfLiteStatus InitCamera();

#endif  // IMAGE_PROVIDER_H_
