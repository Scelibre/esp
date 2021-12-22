#include <dmx_output.h>
#include "dmx_output.h"

#include <logging.h>

#include <stdlib.h>

static const struct uart_options dmx_uart_options = {
  .clock_div   = UART_BAUD_250000,
  .data_bits   = UART_DATA_BITS_8,
  .parity_bits = UART_PARITY_DISABLE,
  .stop_bits   = UART_STOP_BITS_2,
};

#define DMX_BREAK_US 92
#define DMX_MARK_US 12

int dmx_output_init (struct dmx_output *out, struct dmx_output_options options)
{
  out->options = options;

  return 0;
}

int dmx_output_new (struct dmx_output **outp, struct dmx_output_options options)
{
  struct dmx_output *out;
  int err;

  if (!(out = calloc(1, sizeof(*out)))) {
    LOG_ERROR("calloc");
    return -1;
  }

  if ((err = dmx_output_init(out, options))) {
    LOG_ERROR("dmx_output_init");
    goto error;
  }

  *outp = out;

  return 0;

error:
  free(out);

  return err;
}

int dmx_output_cmd (struct dmx_output *out, enum dmx_cmd cmd, void *data, size_t len)
{
  int err;

  if ((err = uart_open(out->options.uart, dmx_uart_options))) {
    LOG_ERROR("uart_open");
    return err;
  }

  // enable output
  if (out->options.gpio_out) {
    gpio_out_set(out->options.gpio_out, out->options.gpio_out_pins);
  }

  // send break/mark per spec minimums for transmit; actual timings will vary, these are minimums
  if ((err = uart_break(out->options.uart, DMX_BREAK_US, DMX_MARK_US))) {
    LOG_ERROR("uart1_break");
    goto error;
  }

  if ((err = uart_putc(out->options.uart, cmd)) < 0) {
    LOG_ERROR("uart_putc");
    goto error;
  }

  for (uint8_t *ptr = data; len > 0; ) {
    ssize_t write;

    if ((write = uart_write(out->options.uart, ptr, len)) < 0) {
      LOG_ERROR("uart_write");
      goto error;
    }

    ptr += write;
    len -= write;
  }

error:
  if (uart_close(out->options.uart)) {
    LOG_WARN("uart_close");
  }

  return err;
}
