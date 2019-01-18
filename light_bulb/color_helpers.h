#ifndef APP_COLOR_HERLPERS
#define APP_COLOR_HERLPERS

#include <cstdint>

struct RgbColor {
  uint8_t  r_value; // Red color value.
  uint8_t  g_value; // Green color value.
  uint8_t  b_value; // Blue color value.
};

RgbColor ConvertHsbToRgb(uint8_t hue, uint8_t saturation, uint8_t brightness);

RgbColor ConvertXyToRgb(uint16_t input_x, uint16_t input_y, uint8_t brightness);

#endif // APP_COLOR_HERLPERS