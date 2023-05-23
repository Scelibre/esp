#include "leds.h"

#include <logging.h>

#define min(x, y) (((x) < (y)) ? (x) : (y))

unsigned leds_format_count(enum leds_format format, size_t len)
{
  switch (format) {
    case LEDS_FORMAT_RGB:
    case LEDS_FORMAT_BGR:
    case LEDS_FORMAT_GRB:
      return len / 3;

    case LEDS_FORMAT_RGBA:
    case LEDS_FORMAT_RGBW:
      return len / 4;

    case LEDS_FORMAT_CUSTOM:
      return len / 11;

    default:
      LOG_FATAL("invalid format=%d", format);
  }
}

void leds_set_format_rgb(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  uint8_t parameter = leds_parameter_default(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 3; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      leds->pixels[params.offset + i * params.segment + j] = (struct leds_color) {
        .r = data[i * 3 + 0],
        .g = data[i * 3 + 1],
        .b = data[i * 3 + 2],

        .parameter = parameter,
      };
    }
  }
}

void leds_set_format_bgr(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  uint8_t parameter = leds_parameter_default(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 3; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      leds->pixels[params.offset + i * params.segment + j] = (struct leds_color) {
        .b = data[i * 3 + 0],
        .g = data[i * 3 + 1],
        .r = data[i * 3 + 2],

        .parameter = parameter,
      };
    }
  }
}

void leds_set_format_grb(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  uint8_t parameter = leds_parameter_default(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 3; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      leds->pixels[params.offset + i * params.segment + j] = (struct leds_color) {
        .g = data[i * 3 + 0],
        .r = data[i * 3 + 1],
        .b = data[i * 3 + 2],

        .parameter = parameter,
      };
    }
  }
}

void leds_set_format_rgba(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  enum leds_parameter_type parameter = leds_parameter_type(leds);
  uint8_t parameter_default = leds_parameter_default(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 4; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      leds->pixels[params.offset + i * params.segment + j] = (struct leds_color) {
        .r = data[i * 4 + 0],
        .g = data[i * 4 + 1],
        .b = data[i * 4 + 2],

        .dimmer = (parameter == LEDS_PARAMETER_DIMMER) ? data[i * 4 + 3] : parameter_default,
      };
    }
  }
}

void leds_set_format_rgbw(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  enum leds_parameter_type parameter = leds_parameter_type(leds);
  uint8_t parameter_default = leds_parameter_default(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  for (unsigned i = 0; i < params.count && len >= (i + 1) * 4; i++) {
    for (unsigned j = 0; j < params.segment; j++) {
      leds->pixels[params.offset + i * params.segment + j] = (struct leds_color) {
        .r = data[i * 4 + 0],
        .g = data[i * 4 + 1],
        .b = data[i * 4 + 2],

        .white = (parameter == LEDS_PARAMETER_WHITE) ? data[i * 4 + 3] : parameter_default,
      };
    }
  }
}

uint16_t tick = 0; 

struct leds_color custom_leds_color_get(const uint8_t value, enum leds_parameter_type parameter) {
  uint8_t red = (value >> 5) * 32;
  uint8_t green = ((value & 28) >> 2) * 32;
  uint8_t blue = (value & 3) * 64;
  uint8_t white = 0;
  if (parameter == LEDS_PARAMETER_WHITE) {
    white = min(red, min(green, blue));
    red -= white;
    green -= white;
    blue -= white;
  }
  return (struct leds_color) {
    .r = red,
    .g = green,
    .b = blue,
    .white = white
  };
}

void custom_leds_color_compose(struct leds_color color1, struct leds_color color2) {
  if (color1.r < color2.r) {
    color1.r = color2.r;
  }
  if (color1.g < color2.g) {
    color1.g = color2.g;
  }
  if (color1.b < color2.b) {
    color1.b = color2.b;
  }
  if (color1.white < color2.white) {
    color1.white = color2.white;
  }
}

void custom_leds_color_dimmer(struct leds_color color, uint8_t dimmer, enum leds_parameter_type parameter) {
  double d = ((double) (0xFF - dimmer) / ((double) 0xFF));
  if (parameter == LEDS_PARAMETER_WHITE) {
    color.white = color.white * d;
  } else {
    color.r = color.r * d;
    color.g = color.g * d;
    color.b = color.b * d;
  }
}

void leds_set_format_custom(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  enum leds_parameter_type parameter = leds_parameter_type(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);

  uint8_t dimmer = data[0];
  uint8_t strobe = data[1];
  struct leds_color empty = (struct leds_color) {
    .r = 0,
    .g = 0,
    .b = 0,
    .white = 0
  };
  if ((dimmer == 0xFF) || ((strobe > 0) && (tick / (0xFFFF / (64 * (1 + strobe))) % 2))) { 
    // No calc, dimmer or strobe -> light off
    for (unsigned i = 0; i < params.count; i++) {
      leds->pixels[params.offset + i] = empty;
    }
  } else {
    for (unsigned j = 0; j < 2; j++) {
      uint8_t type = data[2 + j * 3];
      if (type == 0) continue;
      uint8_t speed = data[3 + j * 3];
      struct leds_color color = custom_leds_color_get(data[4 + j * 3], parameter);
      switch (type) {
        case 1: //FULL
          for (unsigned i = 0; i < params.count; i++) {
            custom_leds_color_compose(leds->pixels[params.offset + i], color);
          }
          break;

        default:
          break;
      }
    }

    if (dimmer > 0) {
      if (parameter == LEDS_PARAMETER_DIMMER) {
        for (unsigned i = 0; i < params.count; i++) {
          leds->pixels[params.offset + i].dimmer = 0xFF - dimmer;
        }
      } else {
        for (unsigned i = 0; i < params.count; i++) {
          custom_leds_color_dimmer(leds->pixels[params.offset + i], dimmer, parameter);
        }
      }
    }
  }

  if (tick == 0xFFFF) {
    tick = 0;
  } else {
    tick++;
  }
}
