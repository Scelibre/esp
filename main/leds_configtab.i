/* included via leds_config.c */
const struct configtab LEDS_CONFIGTAB[] = {
  { CONFIG_TYPE_BOOL, "enabled",
    .bool_type = { .value = &LEDS_CONFIG.enabled },
  },
  { CONFIG_TYPE_ENUM, "interface",
    .enum_type = { .value = &LEDS_CONFIG.interface, .values = leds_interface_enum },
  },
  { CONFIG_TYPE_ENUM, "protocol",
    .enum_type = { .value = &LEDS_CONFIG.protocol, .values = leds_protocol_enum },
  },
  { CONFIG_TYPE_UINT16, "count",
    .description = "Number of LED pixels",
    .uint16_type = { .value = &LEDS_CONFIG.count, .max = LEDS_COUNT_MAX },
  },
  { CONFIG_TYPE_UINT16, "limit_total",
    .alias = "limit",
    .description = (
      "Apply total power limit for the maximum number of LED pixels at full brightness across all LEDs."
      "Default 0 -> no total limit"
    ),
    .uint16_type = { .value = &LEDS_CONFIG.limit_total, .max = LEDS_COUNT_MAX },
  },
  { CONFIG_TYPE_UINT16, "limit_group",
    .description = (
      "Apply per-group power limit for the maximum number of LED pixels at full brightness within each group of consecutive LEDs."
      "Default 0 -> no per-group limit"
    ),
    .uint16_type = { .value = &LEDS_CONFIG.limit_group, .max = LEDS_COUNT_MAX },
  },
  { CONFIG_TYPE_UINT16, "limit_groups",
    .description = (
      "Number of groups to split consecutive LEDs across for per-group power limiting."
      "Default 0 -> no groups"
    ),
    .uint16_type = { .value = &LEDS_CONFIG.limit_groups, .max = LEDS_LIMIT_GROUPS_MAX },
  },

#if CONFIG_LEDS_SPI_ENABLED
  { CONFIG_TYPE_ENUM, "spi_clock",
    .alias = "spi_rate",
    .description = "Longer cable runs can be noisier, and may need a slower rate to work reliably.",
    .enum_type = { .value = &LEDS_CONFIG.spi_clock, .values = leds_spi_clock_enum, .default_value = SPI_CLOCK_DEFAULT },
  },
# if CONFIG_IDF_TARGET_ESP8266
  { CONFIG_TYPE_UINT16, "spi_delay",
    .alias = "delay",
    .description = "Delay data signal transitions by system clock cycles to offset clock/data transitions and avoid coupling glitches.",
    .uint16_type = { .value = &LEDS_CONFIG.spi_delay, .max = SPI_MODE_MOSI_DELAY_MAX },
  },
# else
  { CONFIG_TYPE_ENUM, "spi_cs_mode",
    .description = "Output SPI Chip Select when transmitting.",
    .enum_type = { .value = &LEDS_CONFIG.spi_cs_mode, .values = leds_spi_cs_mode_enum, .default_value = LEDS_SPI_CS_MODE_DISABLED },
  },
  { CONFIG_TYPE_UINT16, "spi_cs_io",
    .description = "Output SPI Chip Select to IO pin when transmitting.",
    .uint16_type = { .value = &LEDS_CONFIG.spi_cs_io, .max = (SOC_GPIO_PIN_COUNT - 1) },
  },
# endif
#endif

#if CONFIG_LEDS_I2S_ENABLED
  { CONFIG_TYPE_ENUM, "i2s_clock",
    .alias = "i2s_clock",
    .description = "Output I2S bit rate. Only used for protocols with a separate clock/data.",
    .enum_type = { .value = &LEDS_CONFIG.i2s_clock, .values = leds_i2s_clock_enum, .default_value = I2S_CLOCK_DEFAULT },
  },
# if LEDS_I2S_GPIO_PINS_ENABLED
  { CONFIG_TYPE_UINT16, "i2s_clock_pin",
    .description = "Output I2S bit clock to GPIO pin. Only used for protocols with a separate clock/data.",
    .count  = &LEDS_CONFIG.i2s_clock_pin_count, .size = LEDS_I2S_DATA_PINS_SIZE,
    .uint16_type = { .value = LEDS_CONFIG.i2s_clock_pins, .max = (GPIO_NUM_MAX - 1) },
  },
  { CONFIG_TYPE_UINT16, "i2s_data_pin",
    .alias = "i2s_gpio_pin",
    .description = "Output I2S data to GPIO pin.",
    .count  = &LEDS_CONFIG.i2s_data_pin_count, .size = LEDS_I2S_DATA_PINS_SIZE,
    .uint16_type = { .value = LEDS_CONFIG.i2s_data_pins, .max = (GPIO_NUM_MAX - 1) },
  },
  { CONFIG_TYPE_UINT16, "i2s_data_inv_pin",
    .description = "Output inverted I2S data to GPIO pin.",
    .count  = &LEDS_CONFIG.i2s_data_inv_pin_count, .size = LEDS_I2S_DATA_PINS_SIZE,
    .uint16_type = { .value = LEDS_CONFIG.i2s_data_inv_pins, .max = (GPIO_NUM_MAX - 1) },
  },
# endif
#endif

#if CONFIG_LEDS_GPIO_ENABLED
  { CONFIG_TYPE_ENUM, "gpio_mode",
    .description = "Multiplex between multiple active-high/low GPIO-controlled outputs",
    .enum_type = { .value = &LEDS_CONFIG.gpio_mode, .values = leds_gpio_mode_enum, .default_value = LEDS_GPIO_MODE_DISABLED },
  },
  { CONFIG_TYPE_UINT16, "gpio_pin",
    .description = "GPIO pin to activate when transmitting on this output",
    .count = &LEDS_CONFIG.gpio_count, .size = LEDS_GPIO_SIZE,
    .uint16_type = { .value = LEDS_CONFIG.gpio_pin, .max = (GPIO_OUT_PIN_COUNT - 1) },
  },
#endif

  { CONFIG_TYPE_BOOL, "test_enabled",
    .description = "Output test patterns at boot",
    .bool_type = { .value = &LEDS_CONFIG.test_enabled },
  },

  { CONFIG_TYPE_BOOL, "artnet_enabled",
    .bool_type = { .value = &LEDS_CONFIG.artnet_enabled },
  },
  { CONFIG_TYPE_UINT16, "artnet_universe_start",
    .alias       = "artnet_universe",
    .description = "Output from Art-Net DMX universe (0-15) within [artnet] net/subnet.",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_universe_start, .max = ARTNET_UNIVERSE_MAX },
  },
  { CONFIG_TYPE_UINT16, "artnet_universe_count",
    .description = "Output from multiple Art-Net DMX universes, from artnet_universe_start to artnet_universe_step * artnet_universe_count. Default 0 -> single unvierse.",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_universe_count, .max = ARTNET_UNIVERSE_MAX },
  },
  { CONFIG_TYPE_UINT16, "artnet_universe_step",
    .description = "Output from multiple non-consecutive Art-Net DMX universes, at artnet_universe_start + artnet_universe_step",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_universe_step, .max = 4 },
  },
  { CONFIG_TYPE_UINT16, "artnet_dmx_addr",
    .description = "Start at DMX address within Art-Net DMX universe. Default 0 -> first channel, the first channel is at adress 1",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_dmx_addr, .max = ARTNET_DMX_SIZE },
  },
  { CONFIG_TYPE_UINT16, "artnet_dmx_leds",
    .alias       = "artnet_universe_leds",
    .description = "Limit number of LEDs per Art-Net DMX universe. Default 0 -> set all available LEDs.",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_dmx_leds, .max = LEDS_ARTNET_UNIVERSE_LEDS },
  },
  { CONFIG_TYPE_UINT16, "artnet_dmx_timeout",
    .description = "Clear LED outputs after given milliseconds without any DMX updates. Default 0 -> hold output.",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_dmx_timeout },
  },
  { CONFIG_TYPE_ENUM, "artnet_leds_format",
    .alias       = "artnet_mode",
    .description = "LED color format for Art-Net DMX channels",
    .enum_type = { .value = &LEDS_CONFIG.artnet_leds_format, .values = leds_format_enum },
  },
  { CONFIG_TYPE_UINT16, "artnet_leds_segment",
    .description = "Control multiple consecutive LEDs per Art-Net DMX channel. Default 0 -> set individual LEDs",
    .uint16_type = { .value = &LEDS_CONFIG.artnet_leds_segment },
  },

  {}
};
