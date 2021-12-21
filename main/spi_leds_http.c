#include "spi_leds.h"
#include "http_routes.h"
#include "http_handlers.h"

#include <logging.h>
#include <json.h>

#include <limits.h>
#include <string.h>

static int spi_leds_api_write_object_enabled(struct json_writer *w, int index, struct spi_leds_state *state)
{
  const struct spi_leds_options *options = spi_leds_options(state->spi_leds);

  return (
        JSON_WRITE_MEMBER_STRING(w, "interface", config_enum_to_string(spi_leds_interface_enum, options->interface))
    ||  JSON_WRITE_MEMBER_STRING(w, "protocol", config_enum_to_string(spi_leds_protocol_enum, options->protocol))
    ||  JSON_WRITE_MEMBER_STRING(w, "color_parameter", config_enum_to_string(spi_leds_color_parameter_enum, spi_leds_color_parameter_for_protocol(options->protocol)))
    ||  JSON_WRITE_MEMBER_UINT(w, "count", options->count)
    ||  JSON_WRITE_MEMBER_UINT(w, "active", state->active)
  );
}

static int spi_leds_api_write_object(struct json_writer *w, int id, struct spi_leds_state *state)
{
  bool enabled = state->config->enabled && state->spi_leds;

  return (
        JSON_WRITE_MEMBER_UINT(w, "id", id)
    ||  JSON_WRITE_MEMBER_BOOL(w, "enabled", enabled)
    ||  (enabled ? spi_leds_api_write_object_enabled(w, id, state) : 0)
  );
}

static int spi_leds_api_write_array(struct json_writer *w)
{
  int err;

  for (int i = 0; i < SPI_LEDS_COUNT; i++)
  {
    struct spi_leds_state *state = &spi_leds_states[i];

    if ((err = JSON_WRITE_OBJECT(w, spi_leds_api_write_object(w, i, state)))) {
      return err;
    }
  }

  return 0;
}

static int spi_leds_api_write(struct json_writer *w, void *ctx)
{
  return JSON_WRITE_ARRAY(w, spi_leds_api_write_array(w));
}

int spi_leds_api_get(struct http_request *request, struct http_response *response, void *ctx)
{
  int err;

  if ((err = http_request_headers(request, NULL))) {
    LOG_WARN("http_request_headers");
    return err;
  }

  if ((err = write_http_response_json(response, spi_leds_api_write, NULL))) {
    LOG_WARN("write_http_response_json -> spi_leds_api_write");
    return err;
  }

  return 0;
}


/* POST /api/leds */
struct spi_leds_api_req {
  struct spi_leds_state *state;
};

int spi_leds_api_color_parse(struct spi_led_color *color, enum spi_leds_protocol protocol, const char *value)
{
  int rgb;
  int parameter = spi_leds_default_color_parameter_for_protocol(protocol);

  if (!value) {
    return HTTP_UNPROCESSABLE_ENTITY;
  }

  if (sscanf(value, "%x.%x", &rgb, &parameter) <= 0) {
    return HTTP_UNPROCESSABLE_ENTITY;
  }

  if (parameter < 0 || parameter > UINT8_MAX) {
    return HTTP_UNPROCESSABLE_ENTITY;
  }

  color->r = (rgb >> 16) & 0xFF;
  color->g = (rgb >>  8) & 0xFF;
  color->b = (rgb >>  0) & 0xFF;
  color->parameter = parameter;

  return 0;
}

int spi_leds_api_state_parse(struct spi_leds_api_req *req, const char *key, const char *value)
{
  struct spi_leds *spi_leds = NULL;
  enum spi_leds_protocol protocol = 0;
  struct spi_led_color color;
  unsigned index;
  int ret;

  // XXX: will only update last state
  if (strcmp(key, "id") == 0) {
    int id;

    if (sscanf(value, "%d", &id) <= 0) {
      return HTTP_UNPROCESSABLE_ENTITY;
    } else if (id < 0 || id >= SPI_LEDS_COUNT) {
      return HTTP_UNPROCESSABLE_ENTITY;
    } else {
      req->state = &spi_leds_states[id];
    }

    return 0;

  } else if (!req->state) {
    LOG_WARN("missing id= in request");
    return HTTP_UNPROCESSABLE_ENTITY;

  } else if (!req->state->spi_leds) {
    LOG_WARN("disabled id= in request");
    return HTTP_UNPROCESSABLE_ENTITY;
  } else {
    spi_leds = req->state->spi_leds;
    protocol = spi_leds_protocol(req->state->spi_leds);
  }

  if (strcmp(key, "all") == 0) {
    if ((ret = spi_leds_api_color_parse(&color, protocol, value))) {
      return ret;
    }

    return spi_leds_set_all(spi_leds, color);

  } else if (sscanf(key, "%u", &index) > 0) {
    if ((ret = spi_leds_api_color_parse(&color, protocol, value))) {
      return ret;
    }

    return spi_leds_set(spi_leds, index, color);

  } else {
    return HTTP_UNPROCESSABLE_ENTITY;
  }
}

int spi_leds_api_form(struct http_request *request, struct http_response *response)
{
  struct spi_leds_api_req req = {};
  char *key, *value;
  int err;

  while (!(err = http_request_form(request, &key, &value))) {
    if ((err = spi_leds_api_state_parse(&req, key, value)) < 0) {
      LOG_ERROR("spi_leds_api_state_parse");
      return err;
    } else if (err) {
      LOG_WARN("spi_leds_api_state_parse: %s=%s", key, value ? value : "");
      return err;
    }
  }

  if (err < 0) {
    LOG_ERROR("http_request_form");
    return err;
  }

  if (req.state && req.state->spi_leds) {
    if ((err = update_spi_leds(req.state)) < 0) {
      LOG_ERROR("update_spi_leds");
      return HTTP_INTERNAL_SERVER_ERROR;
    } else if (err) {
      LOG_WARN("update_spi_leds");
      return HTTP_CONFLICT;
    }
  }

  return HTTP_NO_CONTENT;
}

int spi_leds_api_post(struct http_request *request, struct http_response *response, void *ctx)
{
  const struct http_request_headers *headers;
  int err;

  if ((err = http_request_headers(request, &headers))) {
    LOG_WARN("http_request_headers");
    return err;
  }

  switch (headers->content_type) {
    case HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED:
      return spi_leds_api_form(request, response);

    default:
      LOG_WARN("Unknown Content-Type");

      return HTTP_UNSUPPORTED_MEDIA_TYPE;
  }
}

/* GET /api/leds/test */

static int spi_leds_api_write_test_array(struct json_writer *w)
{
  int err;

  for (const struct config_enum *e = spi_leds_test_mode_enum; e->name; e++) {
    if ((err = JSON_WRITE_OBJECT(w,
      JSON_WRITE_MEMBER_STRING(w, "mode", e->name)
    ))) {
      return err;
    }
  }

  return 0;
}

static int spi_leds_api_write_test(struct json_writer *w, void *ctx)
{
  return JSON_WRITE_ARRAY(w, spi_leds_api_write_test_array(w));
}

int spi_leds_api_test_get(struct http_request *request, struct http_response *response, void *ctx)
{
  int err;

  if ((err = http_request_headers(request, NULL))) {
    LOG_WARN("http_request_headers");
    return err;
  }

  if ((err = write_http_response_json(response, spi_leds_api_write_test, NULL))) {
    LOG_WARN("write_http_response_json -> spi_leds_api_write_test");
    return err;
  }

  return 0;
}

/* POST /api/leds/test */
struct spi_leds_api_test_params {
  int id;
  enum spi_leds_test_mode mode;
};

int spi_leds_api_test_params_set(struct spi_leds_api_test_params *params, const char *key, const char *value)
{
  if (strcmp(key, "id") == 0) {
    if (sscanf(value, "%d", &params->id) <= 0) {
      return HTTP_UNPROCESSABLE_ENTITY;
    }
  } else if (strcmp(key, "mode") == 0) {
    int mode;

    if ((mode = config_enum_to_value(spi_leds_test_mode_enum, value)) < 0) {
      return HTTP_UNPROCESSABLE_ENTITY;
    } else {
      params->mode = mode;
    }
  } else {
    return HTTP_UNPROCESSABLE_ENTITY;
  }

  return 0;
}

/* POST /api/config application/x-www-form-urlencoded */
int spi_leds_api_test_read_form_params(struct http_request *request, struct spi_leds_api_test_params *params)
{
  char *key, *value;
  int err;

  while (!(err = http_request_form(request, &key, &value))) {
    if ((err = spi_leds_api_test_params_set(params, key, value))) {
      LOG_WARN("spi_leds_api_test_params_set: %s=%s", key, value ? value : "");
      return err;
    }
  }

  if (err < 0) {
    LOG_ERROR("http_request_form");
    return err;
  }

  return 0;
}

int spi_leds_api_test_post(struct http_request *request, struct http_response *response, void *ctx)
{
  const struct http_request_headers *headers;
  struct spi_leds_api_test_params params = {
    .mode = TEST_MODE_CHASE,
  };
  int err;

  if ((err = http_request_headers(request, &headers))) {
    LOG_WARN("http_request_headers");
    return err;
  }

  switch (headers->content_type) {
    case HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED:
      if ((err = spi_leds_api_test_read_form_params(request, &params))) {
        LOG_WARN("spi_leds_api_test_read_form_params");
        return err;
      }

      break;

    default:
      LOG_WARN("Unknown Content-Type");

      return HTTP_UNSUPPORTED_MEDIA_TYPE;
  }

  // decode
  struct spi_leds_state *state;

  if (params.id < 0 || params.id >= SPI_LEDS_COUNT) {
    LOG_WARN("invalid id=%d", params.id);
    return HTTP_UNPROCESSABLE_ENTITY;
  } else {
    state = &spi_leds_states[params.id];
  }

  // XXX: may block for some time
  if ((err = test_spi_leds(state, params.mode)) < 0) {
    LOG_ERROR("test_spi_leds");
    return HTTP_INTERNAL_SERVER_ERROR;
  } else if (err) {
    LOG_WARN("test_spi_leds");
    return HTTP_CONFLICT;
  }

  return HTTP_NO_CONTENT;
}
