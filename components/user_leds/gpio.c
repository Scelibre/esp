#include <user_leds.h>
#include "user_leds.h"

#include <logging.h>

int user_leds_gpio_init(struct user_leds *leds, struct gpio_options *gpio_options)
{
  int err;

  leds->gpio_options = gpio_options;

  for (unsigned i = 0; i < leds->count; i++) {
    struct user_led *led = &leds->leds[i];

    LOG_INFO("[%u] gpio_pin=%2d -> in_pins=" GPIO_PINS_FMT " out_pins=" GPIO_PINS_FMT " inverted_pins=" GPIO_PINS_FMT " interrupt_pins=" GPIO_PINS_FMT, i,
      led->options.gpio_pin,
      GPIO_PINS_ARGS((led->options.mode & USER_LEDS_MODE_INPUT_BIT) ? GPIO_PINS(led->options.gpio_pin) : GPIO_PINS_NONE),
      GPIO_PINS_ARGS((led->options.mode & USER_LEDS_MODE_OUTPUT_BIT) ? GPIO_PINS(led->options.gpio_pin) : GPIO_PINS_NONE),
      GPIO_PINS_ARGS((led->options.mode & USER_LEDS_MODE_INVERTED_BIT) ? GPIO_PINS(led->options.gpio_pin) : GPIO_PINS_NONE),
      GPIO_PINS_ARGS((led->options.mode & USER_LEDS_MODE_INTERRUPT_BIT) ? GPIO_PINS(led->options.gpio_pin) : GPIO_PINS_NONE)
    );

    if (led->options.mode & USER_LEDS_MODE_INPUT_BIT) {
      gpio_options->in_pins |= GPIO_PINS(led->options.gpio_pin);
    }
    if (led->options.mode & USER_LEDS_MODE_OUTPUT_BIT) {
      gpio_options->out_pins |= GPIO_PINS(led->options.gpio_pin);
    }
    if (led->options.mode & USER_LEDS_MODE_INVERTED_BIT) {
      gpio_options->inverted_pins |= GPIO_PINS(led->options.gpio_pin);
    }
    if (led->options.mode & USER_LEDS_MODE_INTERRUPT_BIT) {
      gpio_options->interrupt_pins |= GPIO_PINS(led->options.gpio_pin);
    }
  }

  LOG_INFO("gpio in_pins=" GPIO_PINS_FMT " out_pins=" GPIO_PINS_FMT " inverted_pins=" GPIO_PINS_FMT " interrupt_pins=" GPIO_PINS_FMT,
    GPIO_PINS_ARGS(gpio_options->in_pins),
    GPIO_PINS_ARGS(gpio_options->out_pins),
    GPIO_PINS_ARGS(gpio_options->inverted_pins),
    GPIO_PINS_ARGS(gpio_options->interrupt_pins)
  );

  // TODO: initial input-only state for mixed input/output pins?
  if ((err = gpio_setup(gpio_options))) {
    LOG_ERROR("gpio_setup");
    return err;
  }

  return 0;
}

int user_leds_gpio_input(struct user_leds *leds)
{
  gpio_pins_t input_pins = 0, input_bits = 0;
  int err;

  for (unsigned i = 0; i < leds->count; i++) {
    struct user_led *led = &leds->leds[i];

    if (led->input_state) {
      input_pins |= GPIO_PINS(led->options.gpio_pin);
    }
  }

  if (input_pins) {
    if ((err = gpio_in_setup(leds->gpio_options, input_pins))) {
      LOG_ERROR("gpio_in_setup");
      return err;
    }

    if ((err = gpio_in_get(leds->gpio_options, &input_bits))) {
      LOG_ERROR("gpio_in_get");
      return err;
    }
  }

  LOG_DEBUG("gpio input_pins=" GPIO_PINS_FMT " input_bits=" GPIO_PINS_FMT,
    GPIO_PINS_ARGS(input_pins),
    GPIO_PINS_ARGS(input_bits)
  );

  for (unsigned i = 0; i < leds->count; i++) {
    struct user_led *led = &leds->leds[i];

    led->input_bit = input_bits & GPIO_PINS(led->options.gpio_pin);
  }

  return 0;
}

int user_leds_gpio_output(struct user_leds *leds)
{
  gpio_pins_t output_pins = 0, output_bits = 0;
  int err;

  for (unsigned i = 0; i < leds->count; i++) {
    struct user_led *led = &leds->leds[i];

    if (led->output_state && !led->input_state) {
      output_pins |= GPIO_PINS(led->options.gpio_pin);
    }

    if (led->output_bit) {
      output_bits |= GPIO_PINS(led->options.gpio_pin);
    }
  }

  LOG_DEBUG("gpio output_pins=" GPIO_PINS_FMT " output_bits=" GPIO_PINS_FMT,
    GPIO_PINS_ARGS(output_pins),
    GPIO_PINS_ARGS(output_bits)
  );

  // update outputs
  if ((err = gpio_out_setup(leds->gpio_options, output_pins))) {
    LOG_ERROR("gpio_out_setup");
    return err;
  }

  if ((err = gpio_out_set(leds->gpio_options, output_bits))) {
    LOG_ERROR("gpio_out_set");
    return err;
  }

  return 0;
}
