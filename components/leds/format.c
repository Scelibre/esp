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
      return 0xFFFF;

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

struct leds_color custom_leds_color_get(const uint8_t value, enum leds_parameter_type parameter) {
  double red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t white = 0;

  uint8_t div = 255 / 7;

  if (value < div) {
    red = (value * 255) / div;
  } else if (value >= div && value < (div * 2)) {
    green = ((value - div) * 255) / div;
    red = 255;
  } else if (value >= (div * 2) && value < (div * 3)) {
    red = ((div - (value - div * 2)) * 255) / div;
    green = 255;
  } else if (value >= (div * 3) && value < (div * 4)) {
    blue = ((value - div * 3) * 255) / div;
    green = 255;
  } else if (value >= (div * 4) && value < (div * 5)) {
    green = ((div - (value - div * 4)) * 255) / div;
    blue = 255;
  } else if (value >= (div * 5) && value < (div * 6)) {
    red = ((value - div * 5) * 255) / div;
    blue = 255;
  } else if (value >= (div * 6)) {
    green = (value > div * 7) ? 255 : (((value - div * 6) * 255) / div);
    red = 255;
    blue = 255;
  }

  if (parameter == LEDS_PARAMETER_WHITE) {
    white = min(red, min(green, blue));
    red -= white;
    green -= white;
    blue -= white;
  }
  return (struct leds_color) {
    .r = green,
    .g = red,
    .b = blue,
    .white = white
  };
}

void custom_leds_color_compose(struct leds_color *color1, struct leds_color color2) {
  if (color1->r < color2.r) {
    color1->r = color2.r;
  }
  if (color1->g < color2.g) {
    color1->g = color2.g;
  }
  if (color1->b < color2.b) {
    color1->b = color2.b;
  }
  if (color1->white < color2.white) {
    color1->white = color2.white;
  }
}

void custom_leds_color_dimmer(struct leds_color *color, uint8_t dimmer) {
  double d = ((double) (0xFF - dimmer) / ((double) 0xFF));
  color->white = color->white * d;
  color->r = color->r * d;
  color->g = color->g * d;
  color->b = color->b * d;
}

int16_t tick = 0;
int tickA = 0;
int tickB = 0;
bool flashEnabled = false;
uint8_t flashType = 0;
uint16_t flashTick = 0;

void leds_set_format_custom_prog(struct leds *leds, struct leds_format_params params, int tick, uint8_t type, struct leds_color color) {
  if (type == 1) {
    for (unsigned i = 0; i < params.count; i++) {
      custom_leds_color_compose(&leds->pixels[params.offset + i], color);
    }
  } else if (type == 2) { // Allume tout puis éteind tout progressivement
    if (tick < params.count){
      for (unsigned i=0; i < tick; i++){
        custom_leds_color_compose(&leds->pixels[params.offset + i], color);
      }
    } else {
      for (unsigned i=0; i < (params.count - (tick - params.count)); i++){
        custom_leds_color_compose(&leds->pixels[params.offset + i], color);
      }
    }
  }
}

void leds_set_format_custom(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params)
{
  enum leds_parameter_type parameter = leds_parameter_type(leds);

  LOG_DEBUG("len=%u offset=%u count=%u segment=%u", len, params.offset, params.count, params.segment);


  uint8_t dimmer = data[0];
  uint8_t strobe = data[1];
  uint8_t flash = data[2];
  uint8_t special = data[3];

  for (unsigned i = 0; i < params.count; i++) {
    leds->pixels[params.offset + i] = (struct leds_color) {
      .r = 0,
      .g = 0,
      .b = 0,
      .white = 0
    };
  }
  if ((dimmer == 0xFF) || ((strobe > 0) && (tick / (0xFFFF / (80 * (1 + strobe))) % 2))) { 
    // No calc, dimmer or strobe -> light off
  } else {

    uint8_t divider = 12;

    uint8_t typeA = data[4];
    if (typeA > 0) {
      struct leds_color color = custom_leds_color_get(data[6], parameter);
      leds_set_format_custom_prog(leds, params, tickA, typeA, color);
      tickA += (data[5] - 128) / divider;
    } else {
      tickA = 0;
    }
    uint8_t typeB = data[7];
    if (typeB > 0) {
      struct leds_color color = custom_leds_color_get(data[9], parameter);
      leds_set_format_custom_prog(leds, params, tickB, typeB, color);
      tickB += (data[8] - 128) / divider;
    } else {
      tickB = 0;
    }



    if (dimmer > 0) {
      if (parameter == LEDS_PARAMETER_DIMMER) {
        for (unsigned i = 0; i < params.count; i++) {
          leds->pixels[params.offset + i].dimmer = 0xFF - dimmer;
        }
      } else {
        for (unsigned i = 0; i < params.count; i++) {
          custom_leds_color_dimmer(&leds->pixels[params.offset + i], dimmer);
        }
      }
    }
  }

  if (flash > 0) {
    if (!flashEnabled) {
      flashEnabled = true;
      flashType = flash;
      flashTick = 0;
    }
  } else {
    flashEnabled = false;
  }

  uint16_t cnt = (params.count * 2);

  if (flashTick < cnt) {
    flashTick++;
    if (flashType >= 1 && flashType <= 3) {
      uint8_t a = 255 - ((flashTick * 255) / cnt); 
      struct leds_color w = (struct leds_color) {
        .r = a,
        .g = a,
        .b = a,
      };
      for (unsigned i = 0; i < params.count; i++) {
        custom_leds_color_compose(&leds->pixels[params.offset + i], w);
      }
      flashTick += 20 * flashType; // SPEED UP
    }
  }
  
  if (tick >= cnt) {
    tick = 0;
  } else {
    tick++;
  }
  if (tickA >= cnt) {
    tickA += -cnt;
  } else if (tickA < 0) {
    tickA += cnt;
  }
  if (tickB >= cnt) {
    tickB += -cnt;
  } else if (tickB < 0) {
    tickB += cnt;
  }
}
