#ifndef MODEL_SETTINGS_H_
#define MODEL_SETTINGS_H_

constexpr int kNumCols = 172;
constexpr int kNumRows = 172;
constexpr int kNumChannels = 3;

constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;

constexpr int kCategoryCount = 7;

extern const char* kCategoryLabels[kCategoryCount];

#endif  // MODEL_SETTINGS_H_
