#include "config.h"

#include "activity_led.h"
#include "alert_led.h"
#include "artnet.h"
#include "atx_psu.h"
#include "dmx.h"
#include "http.h"
#include "spi_leds_config.h"
#include "spiffs.h"
#include "wifi.h"

#include <config.h>
#include <errno.h>
#include <logging.h>

#define CONFIG_PARTITON_LABEL ("config")
#define CONFIG_BASE_PATH ("/config")
#define CONFIG_MAX_FILES 4

const struct configmod config_modules[] = {
  { "activity_led",
    .description = "Indicate spi-leds/dmx activity using a blinking LED.",
    .table = activity_led_configtab,
  },
  { "alert_led",
    .description = "Indicate errors using a blinking LED.",
    .table = alert_led_configtab,
  },
  { "atx_psu",
    .description = (
      "Control ATX-PSU based on spi-leds output."
      "\n"
      "The ATX-PSU will be powered on when spi-led outputs are active,"
      " and powered off into standby mode if all spi-led outputs are idle (zero-valued)."
    ),
    .table = atx_psu_configtab,
  },
  { "wifi",
    .description = (
      "WiFi station mode, connecting to an SSID with optional PSK."
      "\n"
      "Uses DHCP for IPv4 addressing."
    ),
    .table = wifi_configtab,
  },
  { "http",
    .description = "HTTP API + Web frontend with optional HTTP basic authentication.",
    .table = http_configtab,
  },
  { "artnet",
    .description = (
      "Art-Net receiver on UDP port 6454."
      "\n"
      "Art-Net addresses consist of the net (0-127) + subnet (0-15) + universe (0-15)."
      " All outputs share the same net/subnet, each output uses a different universe."
      " Up to four outputs are supported."
    ),
    .table = artnet_configtab,
  },
  { "spi-leds0",
    .description = (
      "Control LEDs using synchronous (separate clock/data) serial protocols via Art-Net."
      "\n"
      "Multiple serial outputs can be multiplexed from the same SPI driver by using GPIOs to control"
      " an external driver chip with active-high/low output-enable GPIO lines."
    ),
    .table = spi_leds_configtab0,
    .alias = "spi_leds",
  },
  { "spi-leds1",
    .description = (
      "Control LEDs using synchronous (separate clock/data) serial protocols via Art-Net."
      "\n"
      "Multiple serial outputs can be multiplexed from the same SPI driver by using GPIOs to control"
      " an external driver chip with active-high/low output-enable GPIO lines."
    ),
    .table = spi_leds_configtab1,
  },
  { "dmx",
    .description = (
      "DMX output via UART1 -> RS-485 transceiver."
      "\n"
      "Because UART1 TX will spew debug messages reset/flash/boot, avoid DMX glitches by using"
      " a GPIO pin that is kept low during reset/boot to drive the RS-485 transceiver's active-high transmit/output-enable."
    ),
    .table = dmx_configtab,
  },
  {}
};

struct config config = {
  .filename = "/config/boot.ini",

  .modules = config_modules,
};

int init_config()
{
  int err;

  if ((err = init_spiffs_partition_formatted(CONFIG_BASE_PATH, CONFIG_PARTITON_LABEL, CONFIG_MAX_FILES)) < 0) {
    LOG_ERROR("init_spiffs_partition");
  } else if (err) {
    LOG_WARN("No configuration partition available");
    return 0;
  }

  if (config_load(&config)) {
    if (errno == ENOENT) {
      LOG_WARN("No configuration available");
      return 0;
    } else {
      LOG_ERROR("Load config failed");
      return -1;
    }
  }

  LOG_INFO("Config loaded");

  return 0;
}

const struct cmdtab config_cmdtab = {
  .arg      = &config,
  .commands = config_commands,
};
