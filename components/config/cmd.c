#include "config.h"

#include <cmd.h>
#include <logging.h>
#include <stdio.h>

#include <sdkconfig.h>

#if CONFIG_LOG_COLORS
  #define CLI_FMT_RESET "\033[0m"

  #define CLI_FMT_COLOR(code)   "\033[0;" code "m"
  #define CLI_FMT_COLOR_YELLOW  "33"
  #define CLI_FMT_COLOR_BLUE    "34"
  #define CLI_FMT_COLOR_MAGENTA "35"
  #define CLI_FMT_COLOR_CYAN    "36"
  #define CLI_FMT_COLOR_DEFAULT "39"

  #define CLI_FMT_COMMENT   CLI_FMT_COLOR(CLI_FMT_COLOR_BLUE)
  #define CLI_FMT_SECTION   CLI_FMT_COLOR(CLI_FMT_COLOR_MAGENTA)
  #define CLI_FMT_NAME      CLI_FMT_COLOR(CLI_FMT_COLOR_CYAN)
  #define CLI_FMT_SEP       CLI_FMT_COLOR(CLI_FMT_COLOR_YELLOW)
  #define CLI_FMT_VALUE     CLI_FMT_COLOR(CLI_FMT_COLOR_DEFAULT)
#else
  #define CLI_FMT_RESET ""
  #define CLI_FMT_COMMENT ""
  #define CLI_FMT_SECTION ""
  #define CLI_FMT_NAME ""
  #define CLI_FMT_SEP ""
  #define CLI_FMT_VALUE ""
#endif

static void print_comment(const char *comment)
{
  bool start = true;

  // support linebreaks for multi-line output
  for (const char *c = comment; *c; c++) {
    if (start) {
      printf(CLI_FMT_COMMENT "#\t");
    }

    putchar(*c);

    if (*c == '\n') {
      start = true;
    } else {
      start = false;
    }
  }

  if (!start) {
    printf(CLI_FMT_RESET "\n");
  }
}

static void print_configtab(const struct configmod *mod, const struct configtab *tab)
{
  unsigned count = configtab_count(tab);

  if (tab->count) {
    printf(CLI_FMT_COMMENT "# %s[%u/%u] = ", tab->name, count, tab->size);
  } else {
    printf(CLI_FMT_COMMENT "# %s = ", tab->name);
  }

  switch(tab->type) {
    case CONFIG_TYPE_UINT16:
      printf("<UINT16>[0..%u]", tab->uint16_type.max ? tab->uint16_type.max : UINT16_MAX);
      break;

    case CONFIG_TYPE_STRING:
      printf("<STRING>[%u]", tab->string_type.size);
      break;

    case CONFIG_TYPE_BOOL:
      printf("true | false");
      break;

    case CONFIG_TYPE_ENUM:
      for (const struct config_enum *e = tab->enum_type.values; e->name; e++) {
        if (e == tab->enum_type.values) {
          printf("%s", e->name);
        } else {
          printf(" | %s", e->name);
        }
      }
      break;

    default:
      printf("???");
      break;
  }

  printf(CLI_FMT_RESET "\n");

  if (tab->description) {
    print_comment(tab->description);
  }

  for (unsigned index = 0; index < count; index++) {
    if (tab->secret) {
      printf(CLI_FMT_NAME "%s" CLI_FMT_SEP " = " CLI_FMT_VALUE "***" CLI_FMT_RESET "\n", tab->name);
    } else {
      printf(CLI_FMT_NAME "%s" CLI_FMT_SEP " = " CLI_FMT_VALUE, tab->name);

      config_print(mod, tab, index, stdout);

      printf(CLI_FMT_RESET "\n");
    }
  }
}

static void print_configmod(const struct configmod *mod, const struct configtab *table)
{
  if (mod->description) {
    print_comment(mod->description);
  }

  if (mod->tables_count) {
    int index = -1;

    for (int i = 0; i < mod->tables_count; i++) {
      if (mod->tables[i] == table) {
        index = i;
      }
    }

    if (index >= 0) {
      printf(CLI_FMT_SEP "[" CLI_FMT_SECTION "%s%d" CLI_FMT_SEP "]" CLI_FMT_RESET "\n", mod->name, index + 1);
    } else {
      printf(CLI_FMT_SEP "[" CLI_FMT_SECTION "%s???" CLI_FMT_SEP "]" CLI_FMT_RESET "\n", mod->name);
    }

  } else {
    printf(CLI_FMT_SEP "[" CLI_FMT_SECTION "%s" CLI_FMT_SEP "]" CLI_FMT_RESET "\n", mod->name);
  }

  for (const struct configtab *tab = table; tab->type && tab->name; tab++) {
    print_configtab(mod, tab);
  }

  printf("\n");
}

static void print_config(const struct config *config)
{
  for (const struct configmod *mod = config->modules; mod->name; mod++) {
    if (mod->tables_count) {
      for (int i = 0; i < mod->tables_count; i++) {
        print_configmod(mod, mod->tables[i]);
      }
    } else {
      print_configmod(mod, mod->table);
    }
  }
}

int config_cmd_save(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;

  if (config_save(config)) {
    LOG_ERROR("config_save");
    return -CMD_ERR;
  }

  return 0;
}

int config_cmd_load(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;

  if (config_load(config)) {
    LOG_ERROR("config_load");
    return -CMD_ERR;
  }

  return 0;
}

int config_cmd_show(int argc, char **argv, void *ctx)
{
  const struct config *config = ctx;
  const struct configmod *mod;
  const struct configtab *table;
  const char *section;
  int err;

  if (argc == 2) {
    if ((err = cmd_arg_str(argc, argv, 1, &section))) {
      return err;
    }

    if (configmod_lookup(config->modules, section, &mod, &table)) {
      LOG_ERROR("Unkown config section: %s", section);
      return -CMD_ERR_ARGV;
    }

    print_configmod(mod, table);
  } else {
    print_config(config);
  }

  return 0;
}

int config_cmd_get(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;
  int err;

  const struct configmod *mod;
  const struct configtab *tab;
  const char *section, *name;
  char value[CONFIG_VALUE_SIZE];

  if ((err = cmd_arg_str(argc, argv, 1, &section))) {
    return err;
  }
  if ((err = cmd_arg_str(argc, argv, 2, &name))) {
    return err;
  }

  if (config_lookup(config, section, name, &mod, &tab)) {
    LOG_ERROR("Unkown config: %s.%s", section, name);
    return -CMD_ERR_ARGV;
  }

  for (unsigned count = configtab_count(tab), index = 0; index < count; index++) {
    if (config_get(mod, tab, index, value, sizeof(value))) {
      LOG_ERROR("Invalid config %s.%s[%u] value: %s", mod->name, tab->name, index, value);
      return -CMD_ERR;
    }

    printf("%s\n", value);
  }

  return 0;
}

int config_cmd_set(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;
  int err;

  const struct configmod *mod;
  const struct configtab *tab;
  const char *section, *name, *value;

  if ((err = cmd_arg_str(argc, argv, 1, &section))) {
    return err;
  }
  if ((err = cmd_arg_str(argc, argv, 2, &name))) {
    return err;
  }

  if (config_lookup(config, section, name, &mod, &tab)) {
    LOG_ERROR("Unkown config: %s.%s", section, name);
    return -CMD_ERR_ARGV;
  }

  if (tab->count) {
    if (config_clear(mod, tab)) {
      LOG_ERROR("config_clear %s.%s", mod->name, tab->name);
      return -CMD_ERR_ARGV;
    }

    for (unsigned index = 0; 3 + index < argc; index++) {
      if ((err = cmd_arg_str(argc, argv, 3 + index, &value))) {
        return err;
      }

      if (config_set(mod, tab, value)) {
        LOG_ERROR("Invalid config %s.%s value[%u]: %s", mod->name, tab->name, index, value);
        return -CMD_ERR_ARGV;
      }
    }
  } else {
    if ((err = cmd_arg_str(argc, argv, 3, &value))) {
      return err;
    }

    if (config_set(mod, tab, value)) {
      LOG_ERROR("Invalid config %s.%s value: %s", mod->name, tab->name, value);
      return -CMD_ERR_ARGV;
    }
  }

  if (config_save(config)) {
    LOG_ERROR("Failed writing config");
    return -CMD_ERR;
  }

  return 0;
}

int config_cmd_clear(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;
  int err;

  const struct configmod *mod;
  const struct configtab *tab;
  const char *section, *name;

  if ((err = cmd_arg_str(argc, argv, 1, &section))) {
    return err;
  }
  if ((err = cmd_arg_str(argc, argv, 2, &name))) {
    return err;
  }

  if (config_lookup(config, section, name, &mod, &tab)) {
    LOG_ERROR("Unkown config: %s.%s", section, name);
    return -CMD_ERR_ARGV;
  }

  if (config_clear(mod, tab)) {
    LOG_ERROR("config_clear %s.%s", mod->name, tab->name);
    return -CMD_ERR_ARGV;
  }

  if (config_save(config)) {
    LOG_ERROR("Failed writing config");
    return -CMD_ERR;
  }

  return 0;
}

int config_cmd_reset(int argc, char **argv, void *ctx)
{
  struct config *config = ctx;

  if (config_reset(config)) {
    LOG_ERROR("config_reset");
    return -CMD_ERR;
  }

  return 0;
}

const struct cmd config_commands[] = {
  { "save",              config_cmd_save,   .usage = "",                                .describe = "Save config to filesystem"  },
  { "load",              config_cmd_load,   .usage = "",                                .describe = "Load config from filesystem"  },
  { "show",              config_cmd_show,   .usage = "[SECTION]",                       .describe = "Show config settings"  },
  { "get",               config_cmd_get,    .usage = "SECTION NAME",                    .describe = "Get config setting"    },
  { "set",               config_cmd_set,    .usage = "SECTION NAME VALUE [VALUE ...]",  .describe = "Set and write config"  },
  { "clear",             config_cmd_clear,  .usage = "SECTION NAME",                    .describe = "Clear and write config"  },
  { "reset",             config_cmd_reset,                                              .describe = "Remove stored config and reset to defaults" },
  {}
};
