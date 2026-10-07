#include "rafbbs_tui_core.h"

static int rafbbs_tui_text_equal(const char *a, const char *b)
{
    unsigned i = 0u;
    if (a == (const char *)0 || b == (const char *)0)
        return a == b;
    while (a[i] != 0 && b[i] != 0) {
        if (a[i] != b[i])
            return 0;
        ++i;
    }
    return a[i] == b[i];
}

int rafbbs_tui_core_test(void)
{
    RafTuiRoute route;

    route = raf_tui_route_char('q');
    if (route.action != RAF_TUI_QUIT || route.pipeline_id != (const char *)0)
        return 1;

    route = raf_tui_route_char('L');
    if (route.action != RAF_TUI_LIST || route.pipeline_id != (const char *)0)
        return 2;

    route = raf_tui_route_char('f');
    if (route.action != RAF_TUI_FILES || route.pipeline_id != (const char *)0)
        return 3;

    route = raf_tui_route_char('1');
    if (route.action != RAF_TUI_RUN ||
        !rafbbs_tui_text_equal(route.pipeline_id, "encoders"))
        return 4;

    route = raf_tui_route_char('2');
    if (route.action != RAF_TUI_RUN ||
        !rafbbs_tui_text_equal(route.pipeline_id, "roundtrip"))
        return 5;

    route = raf_tui_route_char('3');
    if (route.action != RAF_TUI_RUN ||
        !rafbbs_tui_text_equal(route.pipeline_id, "apkc_validate"))
        return 6;

    route = raf_tui_route_char('\n');
    if (route.action != RAF_TUI_RUN ||
        !rafbbs_tui_text_equal(route.pipeline_id, "encoders"))
        return 7;

    route = raf_tui_route_char('x');
    if (route.action != RAF_TUI_RUN ||
        !rafbbs_tui_text_equal(route.pipeline_id, "encoders"))
        return 8;

    return 0;
}

#ifdef RAFBBS_TUI_CORE_TEST_MAIN
int main(void)
{
    return rafbbs_tui_core_test();
}
#endif
