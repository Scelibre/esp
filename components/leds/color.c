#include <leds.h>

#include <logging.h>

uint8_t leds_parameter_default_for_type(enum leds_parameter_type parameter_type)
{
  switch (parameter_type) {
    case LEDS_PARAMETER_NONE:
      return 0;

    case LEDS_PARAMETER_DIMMER:
      return 255;

    case LEDS_PARAMETER_WHITE:
      return 0;

    default:
      // unknown
      return 0;
  }
}

bool leds_color_active (struct leds_color color, enum leds_parameter_type parameter_type)
{
  switch (parameter_type) {
    case LEDS_PARAMETER_NONE:
      return color.r || color.g || color.b;

    case LEDS_PARAMETER_DIMMER:
      return (color.r || color.g || color.b) && color.dimmer;

    case LEDS_PARAMETER_WHITE:
      return color.r || color.g || color.b || color.white;

    default:
      LOG_FATAL("invalid parameter_type=%u", parameter_type);
  }
}

unsigned leds_colors_active (const struct leds_color *pixels, unsigned count, enum leds_parameter_type parameter_type)
{
  unsigned active = 0;

  for (unsigned i = 0; i < count; i++) {
    if (leds_color_active(pixels[i], parameter_type)) {
      active++;
    }
  }

  return active;
}
