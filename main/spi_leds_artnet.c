#include "spi_leds.h"
#include "artnet.h"

#include <artnet.h>
#include <logging.h>
#include <spi_leds.h>

#define SPI_LEDS_ARTNET_TASK_NAME_FMT "spi-leds%d"
#define SPI_LEDS_ARTNET_TASK_STACK 1024
#define SPI_LEDS_ARTNET_TASK_PRIORITY (tskIDLE_PRIORITY + 2)

static void spi_leds_set_dmx_rgb(struct spi_leds *spi_leds, struct artnet_dmx *artnet_dmx, unsigned universe_size, unsigned universe_index)
{
  unsigned offset = (universe_size / 3) * universe_index;
  unsigned count = spi_leds_count(spi_leds);
  unsigned len = artnet_dmx->len < universe_size ? artnet_dmx->len : universe_size;

  LOG_DEBUG("universe_size=%u universe_index=%u -> offset=%u count=%u len=%u", universe_size, universe_index, offset, count, artnet_dmx->len);

  for (unsigned i = 0; offset + i < count && len >= (i + 1) * 3; i++) {
    spi_leds_set(spi_leds, offset + i, (struct spi_led_color) {
      .r = artnet_dmx->data[i * 3 + 0],
      .g = artnet_dmx->data[i * 3 + 1],
      .b = artnet_dmx->data[i * 3 + 2],

      .brightness = 255,
    });
  }
}

static void spi_leds_set_dmx_bgr(struct spi_leds *spi_leds, struct artnet_dmx *artnet_dmx, unsigned universe_size, unsigned universe_index)
{
  unsigned offset = (universe_size / 3) * universe_index;
  unsigned count = spi_leds_count(spi_leds);
  unsigned len = artnet_dmx->len < universe_size ? artnet_dmx->len : universe_size;

  LOG_DEBUG("universe_size=%u universe_index=%u -> offset=%u count=%u len=%u", universe_size, universe_index, offset, count, artnet_dmx->len);

  for (unsigned i = 0; offset + i < count && len >= (i + 1) * 3; i++) {
    spi_leds_set(spi_leds, offset + i, (struct spi_led_color) {
      .b = artnet_dmx->data[i * 3 + 0],
      .g = artnet_dmx->data[i * 3 + 1],
      .r = artnet_dmx->data[i * 3 + 2],

      .brightness = 255,
    });
  }
}

static void spi_leds_set_dmx_grb(struct spi_leds *spi_leds, struct artnet_dmx *artnet_dmx, unsigned universe_size, unsigned universe_index)
{
  unsigned offset = (universe_size / 3) * universe_index;
  unsigned count = spi_leds_count(spi_leds);
  unsigned len = artnet_dmx->len < universe_size ? artnet_dmx->len : universe_size;

  LOG_DEBUG("universe_size=%u universe_index=%u -> offset=%u count=%u len=%u", universe_size, universe_index, offset, count, artnet_dmx->len);

  for (unsigned i = 0; offset + i < count && len >= (i + 1) * 3; i++) {
    spi_leds_set(spi_leds, offset + i, (struct spi_led_color) {
      .g = artnet_dmx->data[i * 3 + 0],
      .r = artnet_dmx->data[i * 3 + 1],
      .b = artnet_dmx->data[i * 3 + 2],

      .brightness = 255,
    });
  }
}

static void spi_leds_set_dmx(struct spi_leds *spi_leds, enum spi_leds_artnet_mode mode, struct artnet_dmx *dmx, unsigned universe_size, unsigned universe_index)
{
  switch(mode) {
    case SPI_LEDS_RGB:
      spi_leds_set_dmx_rgb(spi_leds, dmx, universe_size, universe_index);
      break;

    case SPI_LEDS_BGR:
      spi_leds_set_dmx_bgr(spi_leds, dmx, universe_size, universe_index);
      break;

    case SPI_LEDS_GRB:
      spi_leds_set_dmx_grb(spi_leds, dmx, universe_size, universe_index);
      break;

    default:
      LOG_WARN("unknown artnet_mode=%d", mode);
      break;
  }
}

static void spi_leds_artnet_main(void *ctx)
{
  struct spi_leds_state *state = ctx;

  for (;;) {
    uint32_t notify_bits;
    bool unsync = false;

    if (!xTaskNotifyWait(0, ARTNET_OUTPUT_TASK_INDEX_BITS | ARTNET_OUTPUT_TASK_SYNC_BIT, &notify_bits, portMAX_DELAY)) {
      LOG_WARN("xQueueReceive");
      continue;
    }

    LOG_DEBUG("notify index=%04x sync=%d", (notify_bits & ARTNET_OUTPUT_TASK_INDEX_BITS), !!(notify_bits & ARTNET_OUTPUT_TASK_SYNC_BIT));

    for (uint8_t index = 0; index < state->artnet.universe_count; index++) {
      if (!(notify_bits & 1 << index)) {
        continue;
      }

      if (!xQueueReceive(state->artnet.queues[index], state->artnet.dmx, portMAX_DELAY)) {
        LOG_WARN("xQueueReceive");
        continue;
      }

      spi_leds_set_dmx(state->spi_leds, state->artnet.mode, state->artnet.dmx, state->artnet.universe_size, index);

      if (!state->artnet.dmx->sync_mode) {
        // at least one DMX update is in non-synchronous mode, so force update
        unsync = true;
      }
    }

    if (unsync || (notify_bits & ARTNET_OUTPUT_TASK_SYNC_BIT)) {
      if (update_spi_leds(state)) {
        LOG_WARN("update_spi_leds");
        continue;
      }
    }
  }
}

int init_spi_leds_artnet(struct spi_leds_state *state, unsigned index, const struct spi_leds_config *config)
{
  char task_name[configMAX_TASK_NAME_LEN];

  LOG_INFO("spi-leds%u: mode=%x universe start=%u count=%u step=%u size=%u", index,
    config->artnet_mode,
    config->artnet_universe_start,
    config->artnet_universe_count,
    config->artnet_universe_step,
    config->artnet_universe_size
  );

  if (!(state->artnet.dmx = calloc(1, sizeof(*state->artnet.dmx)))) {
    LOG_ERROR("calloc");
    return -1;
  }
  if (!(state->artnet.queues = calloc(config->artnet_universe_count, sizeof(*state->artnet.queues)))) {
    LOG_ERROR("calloc");
    return -1;
  }

  state->artnet.mode = config->artnet_mode;
  state->artnet.universe_size = config->artnet_universe_size;
  state->artnet.universe_count = config->artnet_universe_count;

  for (uint8_t i = 0; i < config->artnet_universe_count; i++) {
    if (!(state->artnet.queues[i] = xQueueCreate(1, sizeof(struct artnet_dmx)))) {
      LOG_ERROR("xQueueCreate");
      return -1;
    }
  }

  // task
  snprintf(task_name, sizeof(task_name), SPI_LEDS_ARTNET_TASK_NAME_FMT, index);

  if (xTaskCreate(&spi_leds_artnet_main, task_name, SPI_LEDS_ARTNET_TASK_STACK, state, SPI_LEDS_ARTNET_TASK_PRIORITY, &state->artnet.task) <= 0) {
    LOG_ERROR("xTaskCreate");
    return -1;
  } else {
    LOG_DEBUG("task=%p", state->artnet.task);
  }

  for (uint8_t i = 0; i < config->artnet_universe_count; i++) {
    struct artnet_output_options options = {
      .port = (enum artnet_output_port) (index), // use spi-ledsX index as output port
      .index = i,
      .address = config->artnet_universe_start + i * config->artnet_universe_step, // net/subnet is set by add_artnet_output()
      .task = state->artnet.task,
    };

    LOG_INFO("spi-leds%u: artnet output port=%d address=%04x index=%u", index, options.port, options.address, options.index);

    if (add_artnet_output(options, state->artnet.queues[i])) {
      LOG_ERROR("add_artnet_output");
      return -1;
    }
  }

  return 0;
}
