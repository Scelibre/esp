#pragma once

#include <cmd.h>

extern const struct cmdtab system_cmdtab;

int init_system();

/*
 * Soft system reset.
 */
void system_restart() __attribute__ ((noreturn));
