#include "automation.h"
#include "esphome/core/log.h"

namespace esphome {
namespace zigbee {

float get_r_from_xy(float x, float y) {
  float z = 1.0f - x - y;
  float X = x / y;
  float Z = z / y;
  float r = X * 1.4628067f - 0.1840623f - Z * 0.2743606f;
  return r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
}

float get_g_from_xy(float x, float y) {
  float z = 1.0f - x - y;
  float X = x / y;
  float Z = z / y;
  float g = -X * 0.5217933f + 1.4472381f + Z * 0.0677227f;
  return g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
}

float get_b_from_xy(float x, float y) {
  float z = 1.0f - x - y;
  float X = x / y;
  float Z = z / y;

  float b = X * 0.0349342f - 0.0968930f + Z * 1.2884099f;

  return b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;
}

}  // namespace zigbee
}  // namespace esphome
