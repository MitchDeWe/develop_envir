#include "main_functions.h"

#include "inference_responder.h"
#include "image_provider.h"
#include "model_settings.h"
#include "gesture_model_data.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <esp_log.h>
#include "esp_main.h"
#include "app_button.h"

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

constexpr int scratchBufSize = 39 * 1024;

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 4606 * 1024 + scratchBufSize;
static uint8_t *tensor_arena;//[kTensorArenaSize]; // Maybe we should move this to external

// Wake timer for fps delays if too fast
TickType_t xLastWakeTime;
// Make sure that the frame rate is ideally 5fps
const TickType_t infer_rate = pdMS_TO_TICKS(200);

static QueueHandle_t xQueueButton = NULL;

typedef enum
{
    MENU = 1,
    PLAY,
    DOWN,
    UP
} button_name_t;

static int button_press;
static int arena_used;

void setup() {
  // Map the model into a usable data structure.
  model = tflite::GetModel(quant_gesture_model_tflite);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  if (tensor_arena == NULL) {
    tensor_arena = (uint8_t *) heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  }
  if (tensor_arena == NULL) {
    printf("Couldn't allocate memory of %d bytes\n", kTensorArenaSize);
    return;
  }

  // Interpreter with only necessary operations
  static tflite::MicroMutableOpResolver<15> micro_op_resolver;
  micro_op_resolver.AddReshape();
  micro_op_resolver.AddConv2D();
  micro_op_resolver.AddMul();
  micro_op_resolver.AddDiv();
  micro_op_resolver.AddMean();
  micro_op_resolver.AddCast();
  micro_op_resolver.AddAdd();
  micro_op_resolver.AddSum();
  micro_op_resolver.AddLogistic();
  micro_op_resolver.AddHardSwish();
  micro_op_resolver.AddBroadcastTo();
  micro_op_resolver.AddConcatenation();
  micro_op_resolver.AddStridedSlice();
  micro_op_resolver.AddDepthwiseConv2D();
  micro_op_resolver.AddAveragePool2D();

  // Build interpreter for inference
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // Allocate memory for the tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }

  // Get model input information
  input = interpreter->input(0);

  // Initialize Camera
  TfLiteStatus init_status = InitCamera();
  if (init_status != kTfLiteOk) {
    MicroPrintf("InitCamera failed\n");
    return;
  }
  // Initialise timers for measuring ticks
  xLastWakeTime = xTaskGetTickCount();

  xQueueButton = xQueueCreate(1, sizeof(int));
  register_adc_button(xQueueButton);
  arena_used = interpreter->arena_used_bytes() / 1000; 
  printf("\rArena kB allocated: %3d\n", arena_used);

}
static int runInference = 0;
static int colour = 0x18E3; //Eerie Black
static int action_guess;
static int last_pressed = 0;
static int countdown = -1;

void loop() {
  //run at 5fps
  xTaskDelayUntil(&xLastWakeTime, infer_rate);

  // Check if button was pressed between frames
   if (xQueueReceive(xQueueButton, &button_press, 0) != pdTRUE) {
    switch (button_press) {
      case MENU:
        break;
      case PLAY:
      if (last_pressed != button_press){
        runInference = 1;
        last_pressed = button_press;
      }
        break;
      case UP:
        last_pressed = button_press;
        break;
      case DOWN:
        last_pressed = button_press;
        break;
    }
   };

  // Get image
  if (kTfLiteOk != GetImage(kNumCols, kNumRows, kNumChannels, input->data.int8)) {
    MicroPrintf("Image capture failed.");
  }

  // this is for the buttons and the colour
  if (runInference > 0 && runInference <= 15) {
    //turn the panel red, run 15 frames then perform iference
    colour = 0x1f << 6; // red
    runInference +=1;
     if (runInference < 5){
      countdown = 3;
    } else if (runInference > 5 && runInference <= 10){
      countdown = 2;
    } else if (runInference > 10){
      countdown = 1;
    }
  } else if (runInference > 15 && runInference <= 30) {
    //run the inference on the model
    colour =  0x3f;; // green
    runInference +=1;
    countdown = -1;
    // This is where the inference on the model would happen if the functions were apropriate //
    action_guess = 1;
  } else if (runInference >= 31) {
    //reset to waitng for button press
    countdown = 0;
    runInference = 0;
    colour = 0x18E3;
  }

  // Respond to detection
  RespondToInference(colour, action_guess, countdown);

  vTaskDelay(1); 
}

#if defined(COLLECT_CPU_STATS)
  long long total_time = 0;
  long long start_time = 0;
  extern long long softmax_total_time;
  extern long long dc_total_time;
  extern long long conv_total_time;
  extern long long fc_total_time;
  extern long long pooling_total_time;
  extern long long add_total_time;
  extern long long mul_total_time;
#endif

// void run_inference(void *ptr) {

// }
