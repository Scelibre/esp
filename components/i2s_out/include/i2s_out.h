#pragma once

#include <stddef.h>
#include <stdint.h>

struct i2s_out;

struct i2s_out_clock_options {
  uint32_t clkm_div   : 6; // clock divider
  uint32_t bck_div    : 6; // clock divider
};

static const struct i2s_out_clock_options I2S_DMA_CLOCK_3M2 = { .clkm_div = 5, .bck_div = 10 };

struct i2s_out_options {
  struct i2s_out_clock_options clock;

  /* When the I2S TX FIFO empties, the I2S output will loop the last output value. */
  uint32_t eof_value;
  unsigned eof_count;
};

int i2s_out_new(struct i2s_out **i2s_outp, size_t buffer_size);

int i2s_out_open(struct i2s_out *i2s_out, struct i2s_out_options options);

int i2s_out_write(struct i2s_out *i2s_out, void *data, size_t len);
int i2s_out_write_all(struct i2s_out *i2s_out, void *data, size_t len);

int i2s_out_close(struct i2s_out *i2s_out);
