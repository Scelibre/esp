#pragma once

#include "leds.h"
#include "custom_leds.h"

void custom_leds_dimmer(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter);
void custom_leds_strobe(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter);
void custom_leds_flash_special(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter);
void custom_leds_continuous_special(struct leds_color color, const uint8_t value, const uint16_t tick, const uint16_t index, size_t len, enum leds_parameter_type parameter);

void custom_leds_program_off(struct leds_color color, const uint8_t speed, const uint8_t color_value, const uint16_t tick, const unsigned index, size_t len, enum leds_parameter_type parameter);
void custom_leds_program_full(struct leds_color color, const uint8_t speed, const uint8_t color_value, const uint16_t tick, const unsigned index, size_t len, enum leds_parameter_type parameter);

extern const struct custom_leds_dmx_channel_executor *custom_leds_dmx_channel_executors[CUSTOM_LEDS_DMX_CHANNEL_COUNT];
extern const struct custom_leds_program *custom_leds_programs[CUSTOM_LEDS_PROGRAM_TYPE_COUNT];

void custom_leds_color_set(struct leds_color color, const uint8_t value);

void leds_process_format_custom(struct leds *leds, const uint8_t *data, size_t len, struct leds_format_params params);