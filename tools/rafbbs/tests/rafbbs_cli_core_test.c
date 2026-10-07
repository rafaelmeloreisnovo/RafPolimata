#include "rafbbs_cli_core.h"

int rafbbs_cli_core_test(void)
{
    static const char *const help_argv[] = {"rafbbs", "--help"};
    static const char *const list_argv[] = {"rafbbs", "list"};
    static const char *const run_argv[] = {"rafbbs", "run", "encoders"};
    static const char *const bad_run_argv[] = {"rafbbs", "run"};
    static const char *const bad_argv[] = {"rafbbs", "unknown"};
    RafCliRoute route;

    route = raf_cli_route(1u, help_argv);
    if (route.action != RAF_CLI_HELP)
        return 1;

    route = raf_cli_route(2u, help_argv);
    if (route.action != RAF_CLI_HELP)
        return 2;

    route = raf_cli_route(2u, list_argv);
    if (route.action != RAF_CLI_LIST)
        return 3;

    route = raf_cli_route(3u, run_argv);
    if (route.action != RAF_CLI_RUN || route.argument != run_argv[2])
        return 4;

    route = raf_cli_route(2u, bad_run_argv);
    if (route.action != RAF_CLI_INVALID)
        return 5;

    route = raf_cli_route(2u, bad_argv);
    if (route.action != RAF_CLI_INVALID)
        return 6;

    if (raf_cli_text_equal("list", "List"))
        return 7;
    if (!raf_cli_text_equal("manifest", "manifest"))
        return 8;

    return 0;
}

#if defined(RAFBBS_CLI_TEST_MAIN)
int main(void)
{
    return rafbbs_cli_core_test();
}
#endif
