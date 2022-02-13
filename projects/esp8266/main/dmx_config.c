#include "dmx.h"
#include "dmx_config.h"

#include <artnet.h>
#include <gpio_out.h>
#include <uart.h>

#include <sdkconfig.h>

struct dmx_config dmx_config = {
  .uart     = -1,
  .mtbp_min = DMX_UART_MTBP_MIN,
};

struct dmx_output_config dmx_output_configs[DMX_OUTPUT_COUNT] = {
  [0] = {
    .gpio_mode         = -1,
  },
  [1] = {
    .gpio_mode        = -1,
  },
};

const struct config_enum dmx_uart_enum[] = {
 { "OFF",   -1              },
#if CONFIG_ESP_CONSOLE_UART_NUM != 0
 { "UART0", UART_0          },
#endif
#if CONFIG_ESP_CONSOLE_UART_NUM != 1
 { "UART1", UART_1          },
#endif
 {}
};

const struct config_enum dmx_gpio_mode_enum[] = {
 { "OFF",  DMX_GPIO_MODE_OFF    },
 { "LOW",  DMX_GPIO_MODE_LOW    },
 { "HIGH", DMX_GPIO_MODE_HIGH   },
 {}
};

const struct configtab dmx_configtab[] = {
 { CONFIG_TYPE_BOOL, "enabled",
    .bool_type = { .value = &dmx_config.enabled },
 },
 { CONFIG_TYPE_ENUM, "uart",
   .description = "Multiplex between multiple active-high/low GPIO-controlled outputs",
   .enum_type = { .value = &dmx_config.uart, .values = dmx_uart_enum },
 },
 { CONFIG_TYPE_UINT16, "mtbp_min",
    .description = (
      "Minimum mark-time-between-packets (us, approximately).\n"
      "Values ~128us or above are recommended.\n"
      "Values below 32us will drastically increase DMX input -> UART RX interrupt overhead, and may cause WiFi connections to fail."
      "DMX input may fail with UART RX break desynchronization errors if this is too high.\n"
    ),
    .uint16_type = { .value = &dmx_config.mtbp_min, .max = DMX_UART_MTBP_MAX },
 },
 { CONFIG_TYPE_BOOL, "input_enabled",
    .description = "Start DMX input on UART.",
    .bool_type = { .value = &dmx_config.input_enabled },
 },
 { CONFIG_TYPE_BOOL, "input_artnet_enabled",
    .description = "Configure Art-NET input port.",
    .bool_type = { .value = &dmx_config.input_artnet_enabled },
 },
 { CONFIG_TYPE_UINT16, "input_artnet_universe",
    .description = "Input to universe (0-15) within [artnet] net/subnet.",
    .uint16_type = { .value = &dmx_config.input_artnet_universe, .max = ARTNET_UNIVERSE_MAX },
 },
 {}
};

#define DMX_OUTPUT_CONFIGTAB dmx_output_configtab0
#define DMX_OUTPUT_CONFIG dmx_output_configs[0]
#include "dmx_output_configtab.i"
#undef DMX_OUTPUT_CONFIGTAB
#undef DMX_OUTPUT_CONFIG

#define DMX_OUTPUT_CONFIGTAB dmx_output_configtab1
#define DMX_OUTPUT_CONFIG dmx_output_configs[1]
#include "dmx_output_configtab.i"
#undef DMX_OUTPUT_CONFIGTAB
#undef DMX_OUTPUT_CONFIG
