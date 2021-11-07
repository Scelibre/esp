#include "ws2811.h"
#include "spi_leds.h"

#include <logging.h>

/*
 * Using 8-bit TX-inverted UART at 4M baud (0.25us per bit) to generate a WS2811 signal,
 * at two bits per (8-bit) byte.

 * The ESP8266 UART uses least-significant-bit first bit order.
 *
 * UART idle state is MARK -> high, start bit BREAK -> low, stop bit MARK -> high.
 * UART tx needs to be inverted to match the WS2811 protocol.
 *
 * One start + 8 data bits + stop frame matches 10 WS2811 periods and encodes 2 WS2811 data bits.
 * The idle period is mark -> TX' low and encodes the WS2812B reset period.
 *
 *           | IDLE  | START | BIT 0 | BIT 1 | BIT 2 | BIT 3 | BIT 4 | BIT 5 | BIT 6 | BIT 7 | END   |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 * WS2811    |Treset |  T0H  |  T0L  |  T0L  |  T0L  |  T0L  |  T1H  |  T1H  |  T1H  |  T1H  |  T1L  |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *  TX'      |   L   |   H   |   L   |   L   |   L   |   L   |   H   |   H   |   H   |   H   |   L   |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *  UART     |   M   |   B   |   M   |   M   |   M   |   M   |   B   |   B   |   B   |   B   |   M   |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *  LUT      |       |       |1 << 0 |1 << 1 |1 << 2 |1 << 3 |1 << 4 |1 << 5 |1 << 6 |1 << 7 |       |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
  *  fixed   |       |       |       |       |       |   1   |   0   |       |       |       |       |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 *  data     |       |       |   1   |   1   |   1   |       |       |  0    |   0   |   0   |       |
 * ----------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
 */
 #define WS2811_RESET_US 50

 // using least-significant-bit first bit order
 #define WS2811_LUT(x) (\
     0b00001000 \
   | (((x >> 0) & 0x1) ? 0 : 0b11100000) \
   | (((x >> 1) & 0x1) ? 0 : 0b00000111) \
 )

static const uint8_t ws2811_lut[] = {
  [0b00] = WS2811_LUT(0b00),
  [0b01] = WS2811_LUT(0b01),
  [0b10] = WS2811_LUT(0b10),
  [0b11] = WS2811_LUT(0b11),
};

static const struct uart1_options uart1_options = {
  .clock_div    = UART1_BAUD_4000000,
  .data_bits    = UART1_DATA_BITS_8,
  .parity_bits  = UART1_PARTIY_DISABLE,
  .stop_bits    = UART1_STOP_BITS_1,
  .inverted     = true,
};

int spi_leds_tx_uart_ws2811(const struct spi_leds_options *options, union ws2811_pixel *pixels, unsigned count)
{
  uint8_t buf[12];
  int err;

  if ((err = uart1_open(options->uart1, uart1_options))) {
    LOG_ERROR("uart1_open");
    return err;
  }

  if (options->gpio_out) {
    gpio_out_set(options->gpio_out, options->gpio_out_pins);
  }

  for (unsigned i = 0; i < count; i++) {
    uint32_t rgb = pixels[i]._rgb;

    buf[0]  = ws2811_lut[(rgb >> 22) & 0x3];
    buf[1]  = ws2811_lut[(rgb >> 20) & 0x3];
    buf[2]  = ws2811_lut[(rgb >> 18) & 0x3];
    buf[3]  = ws2811_lut[(rgb >> 16) & 0x3];
    buf[4]  = ws2811_lut[(rgb >> 14) & 0x3];
    buf[5]  = ws2811_lut[(rgb >> 12) & 0x3];
    buf[6]  = ws2811_lut[(rgb >> 10) & 0x3];
    buf[7]  = ws2811_lut[(rgb >>  8) & 0x3];
    buf[8]  = ws2811_lut[(rgb >>  6) & 0x3];
    buf[9]  = ws2811_lut[(rgb >>  4) & 0x3];
    buf[10] = ws2811_lut[(rgb >>  2) & 0x3];
    buf[11] = ws2811_lut[(rgb >>  0) & 0x3];

    if ((err = uart1_write_all(options->uart1, buf, sizeof(buf)))) {
      LOG_ERROR("uart1_write_all");
      goto error;
    }
  }

  if ((err = uart1_mark(options->uart1, WS2811_RESET_US))) {
    LOG_ERROR("uart1_mark");
    goto error;
  }

error:
  if (options->gpio_out) {
    gpio_out_clear(options->gpio_out);
  }

  if ((err = uart1_close(options->uart1))) {
    LOG_ERROR("uart1_close");
    return err;
  }

  return err;
}