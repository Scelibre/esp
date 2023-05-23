#include "custom_leds.h"

void custom_leds_dimmer(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter)
{
  uint8_t dimmer = 0xFF - value;

  if (parameter == LEDS_PARAMETER_DIMMER) {
    color.dimmer = dimmer;
  } else {
    color.r = color.r * (((double) dimmer) / 0xFF);
    color.g = color.g * (((double) dimmer) / 0xFF);
    color.b = color.b * (((double) dimmer) / 0xFF);
    if (parameter = LEDS_PARAMETER_WHITE) {
      color.white = color.white * (((double) dimmer) / 0xFF);
    }
  }
}

void custom_leds_strobe(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter)
{
  if (tick / (0xFFFF / (64 * (1 + value))) % 2) {
    if (parameter == LEDS_PARAMETER_DIMMER) {
      color.dimmer = 0;
    } else {
      color.r = 0;
      color.g = 0;
      color.b = 0;
      if (parameter = LEDS_PARAMETER_WHITE) {
        color.white = 0;
      }
    }
  }
}

void custom_leds_flash_special(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter)
{
  // TODO
}

void custom_leds_continuous_special(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter)
{
  // TODO
}