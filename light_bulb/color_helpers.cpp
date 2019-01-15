#include "color_helpers.h"

#include <cmath>

/**@brief Function to convert hue_stauration to RGB color space.
 *
 * @param[IN]  hue          Hue value of color.
 * @param[IN]  saturation   Saturation value of color.
 * @param[IN]  brightness   Brightness value of color.
 * @param[OUT] p_led_params Pointer to structure containing parameters to write to LED characteristic
 */
RgbColor ConvertHsbToRgb(uint8_t hue, uint8_t saturation, uint8_t brightness)
{
    /* C, X, m are auxiliary variables */
    float C     = 0.0;
    float X     = 0.0;
    float m     = 0.0;
    /* Convertion HSB --> RGB */
    C = (brightness / 255.0f) * (saturation / 254.0f);
    X = (hue / 254.0f) * 6.0f;
    /* Casting in var X is necessary due to implementation of floating-point modulo_2 */
    /*lint -e653 */
    X = (X - (2 * (((uint8_t) X) / 2)));
    /*lint -restore */
    X -= 1.0f;
    X = C * (1.0f - ((X > 0.0f) ? (X) : (-1.0f * X)));
    m = (brightness / 255.0f) - C;

    RgbColor result;
    /* Hue value is stored in range (0 - 255) instead of (0 - 360) degree */
    if (hue <= 42) /* hue < 60 degree */
    {
        result.r_value = (uint8_t)((C + m) * 255.0f);
        result.g_value = (uint8_t)((X + m) * 255.0f);
        result.b_value = (uint8_t)((0.0f + m) * 255.0f);
    }
    else if (hue <= 84)  /* hue < 120 degree */
    {
        result.r_value = (uint8_t)((X + m) * 255.0f);
        result.g_value = (uint8_t)((C + m) * 255.0f);
        result.b_value = (uint8_t)((0.0f + m) * 255.0f);
    }
    else if (hue <= 127) /* hue < 180 degree */
    {
        result.r_value = (uint8_t)((0.0f + m) * 255.0f);
        result.g_value = (uint8_t)((C + m) * 255.0f);
        result.b_value = (uint8_t)((X + m) * 255.0f);
    }
    else if (hue < 170)  /* hue < 240 degree */
    {
        result.r_value = (uint8_t)((0.0f + m) * 255.0f);
        result.g_value = (uint8_t)((X + m) * 255.0f);
        result.b_value = (uint8_t)((C + m) * 255.0f);
    }
    else if (hue <= 212) /* hue < 300 degree */
    {
        result.r_value = (uint8_t)((X + m) * 255.0f);
        result.g_value = (uint8_t)((0.0f + m) * 255.0f);
        result.b_value = (uint8_t)((C + m) * 255.0f);
    }
    else                /* hue < 360 degree */
    {
        result.r_value = (uint8_t)((C + m) * 255.0f);
        result.g_value = (uint8_t)((0.0f + m) * 255.0f);
        result.b_value = (uint8_t)((X + m) * 255.0f);
    }
    return result;
}

RgbColor ConvertXyToRgb(uint16_t input_x, uint16_t input_y) {
    float x = float(input_x) / 0xffff;
    float y = float(input_y) / 0xffff;
    float z = 1.0f - x - y;
    float Y = 1.0f; // Brightness. TODO: Use value from level control cluster?
    float X = (Y / y) * x;
    float Z = (Y / y) * z;
    float r = X * 3.2406f - Y * 1.5372f - Z * 0.4986f;
    float g = -X * 0.9689f + Y * 1.8758f + Z * 0.0415f;
    float b = X * 0.0557f - Y * 0.2040f + Z * 1.0570f;

    if (r > b && r > g && r > 1.0f) {
        // red is too big
        g = g / r;
        b = b / r;
        r = 1.0f;
    }
    else if (g > b && g > r && g > 1.0f) {
        // green is too big
        r = r / g;
        b = b / g;
        g = 1.0f;
    }
    else if (b > r && b > g && b > 1.0f) {
        // blue is too big
        r = r / b;
        g = g / b;
        b = 1.0f;
    }

    r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
    g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
    b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;
    return { uint8_t(r * 256), uint8_t(g * 256), uint8_t(b * 256) };
}