#ifndef RAFBBS_CLI_CORE_H
#define RAFBBS_CLI_CORE_H

#include "rafbbs_types.h"

typedef enum {
    RAF_CLI_HELP = 0,
    RAF_CLI_LIST = 1,
    RAF_CLI_RUN = 2,
    RAF_CLI_LOGS = 3,
    RAF_CLI_MANIFEST = 4,
    RAF_CLI_FILES = 5,
    RAF_CLI_INVALID = 6
} RafCliAction;

typedef struct {
    RafCliAction action;
    const char *argument;
} RafCliRoute;

static inline RafU32
raf_cli_text_equal(const char *a, const char *b)
{
    if (a == (const char *)0 || b == (const char *)0)
        return 0u;

    while (*a != 0 && *b != 0) {
        if (*a != *b)
            return 0u;
        a++;
        b++;
    }

    return (RafU32)(*a == *b);
}

static inline RafCliRoute
raf_cli_route(RafU32 argc, const char *const *argv)
{
    RafCliRoute route;
    const char *command;

    route.action = RAF_CLI_HELP;
    route.argument = (const char *)0;

    if (argc <= 1u || argv == (const char *const *)0)
        return route;

    command = argv[1];

    if (raf_cli_text_equal(command, "--help"))
        return route;

    if (raf_cli_text_equal(command, "list")) {
        route.action = RAF_CLI_LIST;
        return route;
    }

    if (raf_cli_text_equal(command, "run")) {
        route.action = RAF_CLI_INVALID;
        if (argc > 2u && argv[2] != (const char *)0) {
            route.action = RAF_CLI_RUN;
            route.argument = argv[2];
        }
        return route;
    }

    if (raf_cli_text_equal(command, "logs")) {
        route.action = RAF_CLI_LOGS;
        return route;
    }

    if (raf_cli_text_equal(command, "manifest")) {
        route.action = RAF_CLI_MANIFEST;
        return route;
    }

    if (raf_cli_text_equal(command, "files")) {
        route.action = RAF_CLI_FILES;
        return route;
    }

    route.action = RAF_CLI_INVALID;
    return route;
}

#endif
