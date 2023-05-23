#include "custom_leds.h"

#include <logging.h>

uint16_t tick = 0;

const struct custom_leds_dmx_channel_executor *custom_leds_dmx_channel_executors[CUSTOM_LEDS_DMX_CHANNEL_COUNT] = {
  [CUSTOM_LEDS_DIMMER]              = &custom_leds_dimmer,
  [CUSTOM_LEDS_STROBE]              = &custom_leds_strobe,
  [CUSTOM_LEDS_FLASH_SPECIAL]       = &custom_leds_flash_special,
  [CUSTOM_LEDS_CONTINUOUS_SPECIAL]  = &custom_leds_continuous_special,
};

const struct custom_leds_program *custom_leds_programs[CUSTOM_LEDS_PROGRAM_TYPE_COUNT] = {
  [CUSTOM_LEDS_PROGRAM_TYPE_OFF]    = &custom_leds_program_off,
  [CUSTOM_LEDS_PROGRAM_TYPE_FULL]   = &custom_leds_program_full,
};

void custom_leds_color_set(struct leds_color color, const uint8_t value, enum leds_parameter_type parameter) {
  uint8_t red = (value >> 5) * 32;
  uint8_t green = ((value & 28) >> 2) * 32;
  uint8_t blue = (value & 3) * 64;
  if (parameter == LEDS_PARAMETER_WHITE) {
    uint8_t white = min(red, min(green, blue));
    if (leds_color.white < white) {
      leds_color.white = white;
    }
    red -= white;
    green -= white;
    blue -= white;
  }
  if (leds_color.r < red) {
    leds_color.r = red;
  }
  if (leds_color.g < green) {
    leds_color.g = green;
  }
  if (leds_color.b < blue) {
    leds_color.b = blue;
  }
}

void leds_process_format_custom(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params) {
  enum leds_parameter_type parameter = leds_parameter_type(leds);
  uint8_t parameter_default = leds_parameter_default(leds);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 4; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      struct leds_color color = (struct leds_color) {
        .r = 0,
        .g = 0,
        .b = 0,
        .white = 0,
      };
      for (unsigned k = 0; k < CUSTOM_LEDS_PROGRAM_COUNT; k++) {
        uint8_t f = k * CUSTOM_LEDS_PROGRAM_DMX_CHANNEL_COUNT;
        custom_leds_dmx_channel_executor[data[f]](color, data[f + 1], data[f + 2], tick, i, len, parameter);
      }
      for (unsigned k = 0; k < CUSTOM_LEDS_DMX_CHANNEL_COUNT; k++) {
        uint8_t f = CUSTOM_LEDS_PROGRAM_COUNT * CUSTOM_LEDS_PROGRAM_DMX_CHANNEL_COUNT;
        custom_leds_program[k](color, data[f + k], tick, i, len, parameter);
      }
      leds->pixels[params.offset + i * params.segment + j] = color;
    }
  }
}