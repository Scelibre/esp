#include "cli.h"

#include <logging.h>
#include <cli.h>
#include <esp_log.h>

#include <stdio.h>

// max line size
#define CLI_BUF_SIZE 256

static struct cli *cli;

static int help_cmd(int argc, char **arv, void *ctx)
{
  return cmd_help(cli);
}

static const struct cmd commands[] = {
  { "help",   help_cmd,   .describe = "Show commands" },
  { },
};

int init_cli()
{
  // unbuffered input
  setvbuf(stdin, NULL, _IONBF, 0);

  // line-buffered output
  setvbuf(stdout, NULL, _IOLBF, 0);

  // interactive CLI on stdin/stdout
  if (cli_init(&cli, commands, CLI_BUF_SIZE)) {
    LOG_ERROR("cli_init");
    return -1;
  }

  return 0;
}