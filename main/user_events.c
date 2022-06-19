#include <user_leds.h>
#include "user_leds.h"
#include "user_leds_config.h"
#include "user_leds_input.h"
#include "user_leds_output.h"

#include "tasks.h"

#include <logging.h>

// reset if held for >5s
#define USER_LEDS_CONFIG_RESET_THRESHOLD (5000 / portTICK_RATE_MS)

xTaskHandle user_events_task;

// TODO: read at boot -> disable config load, enable config mode
// TODO: ignore when pressed at boot
void on_user_config_input(struct user_leds_input input)
{
  switch (input.event) {
    case USER_LEDS_INPUT_PRESS:
      // enter config mode
      LOG_WARN("config");
      override_user_led(input.index, USER_LEDS_PULSE); // feedback
      user_config_mode();
      break;

    case USER_LEDS_INPUT_HOLD:
      if (input.ticks > USER_LEDS_CONFIG_RESET_THRESHOLD) {
        LOG_WARN("config reset");
        user_state(USER_STATE_RESET);
        revert_user_led(input.index);
        user_config_reset();
      }
      break;

    case USER_LEDS_INPUT_RELEASE:
      LOG_WARN("cancel");
      revert_user_led(input.index);

      break;
  }
}

void on_user_test_input(struct user_leds_input input)
{
  switch (input.event) {
    case USER_LEDS_INPUT_PRESS:
      LOG_WARN("start");
      override_user_led(input.index, USER_LEDS_PULSE); // feedback
      user_test_mode();

      break;

    case USER_LEDS_INPUT_HOLD:
      LOG_WARN("continue");

      break;

    case USER_LEDS_INPUT_RELEASE:
      LOG_WARN("cancel");
      revert_user_led(input.index);
      user_test_cancel();

      break;
  }
}

void on_user_input(struct user_leds_input input)
{
  switch (input.index) {
  #if CONFIG_STATUS_LEDS_USER_MODE_TEST
    case USER_LED:
      on_user_test_input(input);
      break;
  #endif

  #if CONFIG_STATUS_LEDS_FLASH_MODE_ACTIVITY_CONFIG || CONFIG_STATUS_LEDS_FLASH_MODE_ALERT_CONFIG
    case FLASH_LED:
      on_user_config_input(input);
      break;
  #endif
  #if CONFIG_STATUS_LEDS_ALERT_MODE_TEST
    case ALERT_LED:
      on_user_test_input(input);
      break;
  #endif
  }
}

int init_user_events()
{
  return 0;
}

void user_events_main(void *arg)
{
  for (;;) {
    struct user_leds_input input;

    if (read_user_leds_input(&input, portMAX_DELAY)) {
      LOG_DEBUG("timeout");
      continue;
    }

    on_user_input(input);
  }
}

int start_user_events()
{
  struct task_options task_options = {
    .main       = user_events_main,
    .name       = USER_EVENTS_TASK_NAME,
    .stack_size = USER_EVENTS_TASK_STACK,
    .arg        = NULL,
    .priority   = USER_EVENTS_TASK_PRIORITY,
    .handle     = &user_events_task,
    .affinity   = USER_EVENTS_TASK_AFFINITY,
  };
  int err;

  if ((err = start_task(task_options))) {
    LOG_ERROR("start_task[%s]", USER_EVENTS_TASK_NAME);
    return err;
  }

  return 0;
}