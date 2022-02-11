#pragma once

#include <dmx.h>
#include <uart.h>

static const struct uart_options dmx_uart_options = {
  .baud_rate   = UART_BAUD_250000,
  .data_bits   = UART_DATA_8_BITS,
  .parity_bits = UART_PARITY_DISABLE,
  .stop_bits   = UART_STOP_BITS_2,
};
