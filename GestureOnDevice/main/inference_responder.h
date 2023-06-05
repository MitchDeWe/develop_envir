#ifndef INFERENCE_RESPONDER_H_
#define INFERENCE_RESPONDER_H_

#include "tensorflow/lite/c/common.h"

void RespondToInference(int colour, int action_guess, int countdown);

#endif  // INFERENCE_RESPONDER_H_
