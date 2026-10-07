#ifndef RAFBBS_TUI_CORE_H
#define RAFBBS_TUI_CORE_H

typedef enum {
    RAF_TUI_QUIT = 0,
    RAF_TUI_LIST = 1,
    RAF_TUI_FILES = 2,
    RAF_TUI_RUN = 3
} RafTuiAction;

typedef struct {
    RafTuiAction action;
    const char *pipeline_id;
} RafTuiRoute;

static inline RafTuiRoute raf_tui_route_char(char input)
{
    RafTuiRoute route;
    route.action = RAF_TUI_RUN;
    route.pipeline_id = "encoders";

    if (input == 'q' || input == 'Q') {
        route.action = RAF_TUI_QUIT;
        route.pipeline_id = (const char *)0;
        return route;
    }
    if (input == 'l' || input == 'L') {
        route.action = RAF_TUI_LIST;
        route.pipeline_id = (const char *)0;
        return route;
    }
    if (input == 'f' || input == 'F') {
        route.action = RAF_TUI_FILES;
        route.pipeline_id = (const char *)0;
        return route;
    }
    if (input == '2') {
        route.pipeline_id = "roundtrip";
        return route;
    }
    if (input == '3') {
        route.pipeline_id = "apkc_validate";
        return route;
    }

    return route;
}

#endif
