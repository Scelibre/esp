#include "ws2812b.h"
#include "../leds.h"
#include "../interfaces/ws2812b.h"

#include <logging.h>

#include <stdlib.h>


int leds_protocol_ws2812b_init(struct leds_protocol_ws2812b *protocol, union leds_interface_state *interface, const struct leds_options *options)
{
  if (!(protocol->pixels = calloc(options->count, sizeof(*protocol->pixels)))) {
    LOG_ERROR("malloc");
    return -1;
  }

  protocol->count = options->count;

  return 0;
}

int leds_protocol_ws2812b_tx(struct leds_protocol_ws2812b *protocol, union leds_interface_state *interface, const struct leds_options *options, struct leds_limit limit)
{
  switch (options->interface) {
    case LEDS_INTERFACE_NONE:
      return 0;

  #if CONFIG_LEDS_UART_ENABLED
    case LEDS_INTERFACE_UART:
      return leds_tx_uart_ws2812b(&options->uart, protocol->pixels, protocol->count, limit);
  #endif

  #if CONFIG_LEDS_I2S_ENABLED
    case LEDS_INTERFACE_I2S:
    #if LEDS_I2S_DATA_PINS_ENABLED
      if (options->i2s.data_pins_count) {
        return leds_tx_i2s_ws2812b_parallel(&options->i2s, protocol->pixels, protocol->count, limit);
      } else {
        return leds_tx_i2s_ws2812b_serial(&options->i2s, protocol->pixels, protocol->count, limit);
      }
    #else
      return leds_tx_i2s_ws2812b_serial(&options->i2s, protocol->pixels, protocol->count, limit);
    #endif
  #endif

    default:
      LOG_ERROR("unsupported interface=%#x", options->interface);
      return -1;
  }
}

void leds_protocol_ws2812b_set(struct leds_protocol_ws2812b *protocol, unsigned index, struct leds_color color)
{
  if (index < protocol->count) {
    protocol->pixels[index] = (union ws2812b_pixel) {
      .b = color.b,
      .r = color.r,
      .g = color.g,
    };
  }
}

void leds_protocol_ws2812b_set_all(struct leds_protocol_ws2812b *protocol, struct leds_color color)
{
  for (unsigned index = 0; index < protocol->count; index++) {
    protocol->pixels[index] = (union ws2812b_pixel) {
      .b = color.b,
      .r = color.r,
      .g = color.g,
    };
  }
}

unsigned leds_protocol_ws2812b_count_active(struct leds_protocol_ws2812b *protocol)
{
  unsigned active = 0;

  for (unsigned index = 0; index < protocol->count; index++) {
    if (ws2812b_pixel_active(protocol->pixels[index])) {
      active++;
    }
  }

  return active;
}

unsigned leds_protocol_ws2812b_count_total(struct leds_protocol_ws2812b *protocol)
{
  unsigned total = 0;

  for (unsigned index = 0; index < protocol->count; index++) {
    total += ws2812b_pixel_total(protocol->pixels[index]);
  }

  return total / WS2812B_PIXEL_TOTAL_DIVISOR;
}
