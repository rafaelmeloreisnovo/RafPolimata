#ifndef RAFBBS_THEME_H
#define RAFBBS_THEME_H
#include "rafbbs_status.h"
#define RAF_ANSI_RESET "\033[0m"
#define RAF_ANSI_GREEN "\033[32m"
#define RAF_ANSI_RED "\033[31m"
#define RAF_ANSI_CYAN "\033[36m"
#define RAF_ANSI_YELLOW "\033[33m"
#define RAF_ANSI_MAGENTA "\033[35m"
#define RAF_ANSI_BLUE "\033[34m"
#define RAF_ANSI_BOLD "\033[1m"

static const char *raf_status_color(RafStatus s) {
    switch (s) {
        case RAF_PASS:
        case RAF_DONE: return RAF_ANSI_GREEN;
        case RAF_FAIL: return RAF_ANSI_RED;
        case RAF_STEP:
        case RAF_HASH: return RAF_ANSI_CYAN;
        case RAF_WARN: return RAF_ANSI_YELLOW;
        case RAF_TOKEN_VAZIO: return RAF_ANSI_MAGENTA;
        case RAF_AUDIT: return RAF_ANSI_BLUE;
        default: return "";
    }
}
#endif
