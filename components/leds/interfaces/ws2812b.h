#pragma once

#include "../interface.h"
#include "../protocols/ws2812b.h"

#if CONFIG_LEDS_UART_ENABLED
  /* interfaces/uart/ws2812b.c */
  int leds_tx_uart_ws2812b(const struct leds_interface_uart_options *options, union ws2812b_pixel *pixels, unsigned count, struct leds_limit limit);
#endif

#if CONFIG_LEDS_I2S_ENABLED
  /* interfaces/i2s/ws2812b.c */
  int leds_tx_i2s_ws2812b(const struct leds_interface_i2s_options *options, union ws2812b_pixel *pixels, unsigned count, struct leds_limit limit);
#endif
