#include "leds.h"
#include "leds_config.h"
#include "leds_state.h"

#include <gpio_out.h>

#include <logging.h>

#if CONFIG_LEDS_GPIO_ENABLED
  // by interface
  struct gpio_out leds_gpio_out[LEDS_INTERFACE_COUNT] = {};

  int init_leds_gpio()
  {
    bool enabled = false;
    bool interfaces_enabled[LEDS_INTERFACE_COUNT] = {};
    int err;

    for (int i = 0; i < LEDS_COUNT; i++)
    {
      const struct leds_config *config = &leds_configs[i];
      enum leds_interface interface = config->interface;

      if (!config->enabled) {
        continue;
      }

      if (interface == LEDS_INTERFACE_NONE) {
        interface = leds_interface_for_protocol(config->protocol);
      }

      if (config->gpio_mode == LEDS_GPIO_MODE_DISABLED) {
        continue;
      }

      //
      enabled = true;
      interfaces_enabled[interface] = true;

      LOG_INFO("leds%d: gpio[%s] mode=%s pin=%d", i + 1,
        config_enum_to_string(leds_interface_enum, interface) ?: "?",
        config_enum_to_string(leds_gpio_mode_enum, config->gpio_mode) ?: "?",
        config->gpio_pin
      );

      leds_gpio_out[interface].pins |= gpio_out_pin(config->gpio_pin);

      if (config->gpio_mode == LEDS_GPIO_MODE_LOW) {
        leds_gpio_out[interface].inverted |= gpio_out_pin(config->gpio_pin);
      }
    }

    if (!enabled) {
      LOG_INFO("leds: gpio not configured");
      return 0;
    }

    for (enum leds_interface interface = 0; interface < LEDS_INTERFACE_COUNT; interface++) {
      if (!interfaces_enabled[interface]) {
        continue;
      }

      LOG_INFO("leds: gpio[%s] -> pins=%08x inverted=%08x",
        config_enum_to_string(leds_interface_enum, interface),
        leds_gpio_out[interface].pins,
        leds_gpio_out[interface].inverted
      );

      if ((err = gpio_out_setup(&leds_gpio_out[interface]))) {
        LOG_ERROR("gpio_out_setup");
        return err;
      }
    }

    return 0;
  }

  int config_leds_gpio(struct leds_state *state, const struct leds_config *config, struct leds_options *options)
  {
    LOG_INFO("leds%d: gpio[%s] mode=%s pin=%d", state->index + 1,
      config_enum_to_string(leds_interface_enum, options->interface),
      config_enum_to_string(leds_gpio_mode_enum, config->gpio_mode),
      config->gpio_pin
    );

    options->gpio_out = &leds_gpio_out[options->interface];
    options->gpio_out_pins = gpio_out_pin(config->gpio_pin);

    return 0;
  }
#endif
