#include "custom_leds.h"

void custom_leds_program_off(struct leds_color color, const uint8_t speed, const uint8_t color_value, const uint16_t tick, const unsigned index, size_t len, enum leds_parameter_type parameter)
{
  // nothing here
}

void custom_leds_program_full(struct leds_color color, const uint8_t speed, const uint8_t color_value, const uint16_t tick, const unsigned index, size_t len, enum leds_parameter_type parameter)
{
  custom_leds_color_set(color, color_value);
}