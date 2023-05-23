#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define CUSTOM_LEDS_PROGRAM_COUNT = 2
#define CUSTOM_LED_DMX_CHANNEL_TOTAL = CUSTOM_LEDS_DMX_CHANNEL_COUNT + CUSTOM_LEDS_PROGRAM_COUNT * CUSTOM_LEDS_PROGRAM_DMX_CHANNEL_COUNT

enum custom_leds_dmx_channel {
  CUSTOM_LEDS_DIMMER,                       // DIMMER (open [0] to close [255])
  CUSTOM_LEDS_STROBE,                       // STROBE (OFF [0], fast [1] to slow [255])
  CUSTOM_LEDS_FLASH_SPECIAL,                // Call a flash special effect
  CUSTOM_LEDS_CONTINUOUS_SPECIAL,           // Continuous special effect

  CUSTOM_LEDS_DMX_CHANNEL_COUNT,
};

enum custom_leds_program_type {
  CUSTOM_LEDS_PROGRAM_TYPE_OFF     = 0x00, // LED OFF
  CUSTOM_LEDS_PROGRAM_TYPE_FULL    = 0x01, // FULL

  CUSTOM_LEDS_PROGRAM_TYPE_COUNT,
};

enum custom_leds_program_dmx_channel {
  CUSTOM_LEDS_PROGRAM_TYPE,
  CUSTOM_LEDS_PROGRAM_SPEED,
  CUSTOM_LEDS_PROGRAM_COLOR,

  CUSTOM_LEDS_PROGRAM_DMX_CHANNEL_COUNT,
}

/*enum custom_leds_flash_special {
  OFF                           = 0x00, // OFF
  FULL                          = 0x01, // FULL
};

enum custom_leds_continuous_special {
  OFF                           = 0x00, // OFF
  STARS                         = 0x01, // White stars
};*/
