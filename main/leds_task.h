#pragma once

#include <leds.h>

// compatible with ARTNET_OUTPUT_EVENT_*_BIT/BITS
#define LEDS_EVENT_SYNC_BIT (ARTNET_OUTPUT_EVENT_SYNC_BIT + 0)
#define LEDS_EVENT_TEST_BIT (ARTNET_OUTPUT_EVENT_SYNC_BIT + 1)

int init_leds_task(struct leds_state *state, const struct leds_config *config);
int start_leds_task(struct leds_state *state, const struct leds_config *config);

void notify_leds_task(struct leds_state *state, EventBits_t bits);
void notify_leds_tasks(EventBits_t bits);
