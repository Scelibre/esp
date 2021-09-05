#pragma once

#include <cli.h>
#include <cmd.h>

struct cli {
  char *buf;
  size_t size;

  struct cmdtab cmdtab;
  struct cmd_eval *cmd_eval;
};
