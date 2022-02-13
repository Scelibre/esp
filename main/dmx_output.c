#include "dmx.h"
#include "dmx_config.h"
#include "dmx_state.h"
#include "artnet_state.h"
#include "user.h"
#include "tasks.h"

#include <artnet.h>
#include <dmx_output.h>
#include <logging.h>

struct dmx_output_state dmx_output_states[DMX_OUTPUT_COUNT];

int init_dmx_output(struct dmx_output_state *state, int index, const struct dmx_output_config *config)
{
  int err;

  state->index = index;

  if (!config->enabled) {
    LOG_INFO("dmx-output%d: disabled", index + 1);
    return 0;
  }

  // dmx output
  struct dmx_output_options options;

  LOG_INFO("dmx-output%d: enabled", index + 1);

  if ((err = config_dmx_output_gpio(state, config, &options))) {
    LOG_ERROR("dmx-output%d: config_dmx_output_gpio", index + 1);
    return err;
  }

  if ((err = dmx_output_new(&state->dmx_output, options))) {
    LOG_ERROR("dmx_output_new");
    return err;
  }

  // artnet
  if (config->artnet_enabled) {
    LOG_INFO("dmx-output%d: artnet universe=%u", index + 1, config->artnet_universe);

    if (!(state->artnet_dmx = calloc(1, sizeof(*state->artnet_dmx)))) {
      LOG_ERROR("calloc: artnet_dmx");
      return -1;
    }

    if (!(state->artnet_queue = xQueueCreate(1, sizeof(struct artnet_dmx)))) {
      LOG_ERROR("xQueueCreate");
      return -1;
    }
  }

  return 0;
}

int init_dmx_outputs()
{
  int err;

  for (int i = 0; i < DMX_OUTPUT_COUNT; i++) {
    const struct dmx_output_config *config = &dmx_output_configs[i];
    struct dmx_output_state *state = &dmx_output_states[i];

    if ((err = init_dmx_output(state, i, config))) {
      LOG_ERROR("dmx%d: init_dmx_output", i + 1);
      return err;
    }
  }

  return 0;
}

int output_dmx(struct dmx_output_state *state, void *data, size_t len)
{
  int err;

  if (!state->dmx_output) {
    LOG_WARN("disabled");
    return -1;
  }

  user_activity(USER_ACTIVITY_DMX_OUTPUT);

  if ((err = open_dmx_output_uart(state->dmx_output))) {
    if (err > 0) {
      LOG_WARN("dmx_output_open: UART not setup, DMX not running");
      return 1;
    } else {
      LOG_ERROR("dmx_output_open");
      return err;
    }
  } else if ((err = dmx_output_write(state->dmx_output, DMX_CMD_DIMMER, data, len)) < 0) {
    LOG_ERROR("dmx_output_write");
    goto error;
  }

error:
  if (dmx_output_close(state->dmx_output)) {
    LOG_WARN("dmx_output_close");
  }

  return err;
}

void dmx_output_main(void *ctx)
{
  struct dmx_output_state *state = ctx;
  int err;

  for (;;) {
    uint32_t notify_bits;

    // wait for output/sync, or next test frame
    xTaskNotifyWait(0, ARTNET_OUTPUT_TASK_INDEX_BITS | ARTNET_OUTPUT_TASK_FLAG_BITS, &notify_bits, portMAX_DELAY);

    LOG_DEBUG("notify index=%04x: sync=%d test=%d",
      (notify_bits & ARTNET_OUTPUT_TASK_INDEX_BITS),
      !!(notify_bits & ARTNET_OUTPUT_TASK_SYNC_BIT),
      !!(notify_bits & ARTNET_OUTPUT_TASK_TEST_BIT)
    );

    // only one index=0 output
    if (notify_bits & ARTNET_OUTPUT_TASK_INDEX_BITS) {
      if (!xQueueReceive(state->artnet_queue, state->artnet_dmx, 0)) {
        LOG_WARN("xQueueReceive");
        continue;
      }

      if ((err = output_dmx(state, state->artnet_dmx->data, state->artnet_dmx->len))) {
        LOG_WARN("dmx-output%d: output_dmx", state->index + 1);
        continue;
      }
    }
  }
}

int start_dmx_output(struct dmx_output_state *state, const struct dmx_output_config *config)
{
  char task_name[configMAX_TASK_NAME_LEN];
  struct artnet_output_options options = {
    .port     = (enum artnet_port) (state->index), // use dmx%d index as output port number
    .index    = 0,
    .address  = config->artnet_universe, // net/subnet set by add_artnet_output()
  };
  int err;

  if (snprintf(task_name, sizeof(task_name), DMX_OUTPUT_TASK_NAME_FMT, state->index + 1) >= sizeof(task_name)) {
    LOG_ERROR("snprintf: task_name overflow");
    return -1;
  }

  if (xTaskCreate(&dmx_output_main, task_name, DMX_OUTPUT_TASK_STACK, state, DMX_OUTPUT_TASK_PRIORITY, &state->artnet_task) <= 0) {
    LOG_ERROR("xTaskCreate");
    return -1;
  } else {
    LOG_DEBUG("task=%p", state->artnet_task);

    options.task = state->artnet_task;
  }

  if ((err = add_artnet_output(options, state->artnet_queue))) {
    LOG_ERROR("add_artnet_output");
    return err;
  }

  return 0;
}

int start_dmx_outputs()
{
  int err;

  for (int i = 0; i < DMX_OUTPUT_COUNT; i++) {
    const struct dmx_output_config *config = &dmx_output_configs[i];
    struct dmx_output_state *state = &dmx_output_states[i];

    if (!config->enabled) {
      continue;
    }

    if (config->artnet_enabled) {
      if ((err = start_dmx_output(state, config))) {
        LOG_ERROR("dmx-output%d: start_dmx_output", i + 1);
        return err;
      }
    }
  }

  // TODO
  return 0;
}
