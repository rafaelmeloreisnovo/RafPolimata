#ifndef RAFBBS_RUNID_POSIX_H
#define RAFBBS_RUNID_POSIX_H

/*
 * Hosted civil-time observation adapter.
 *
 * Local timezone/calendar observation is provider/libc behavior. The adapter
 * converts it into RafCivilStamp; deterministic validation and run-id byte
 * rendering live in rafbbs_runid_core.h.
 */

#include <time.h>
#include "rafbbs_runid_core.h"

static inline RafU32 raf_runid_posix_local(
    char *dst,
    RafU32 cap
)
{
    time_t now;
    struct tm tmv;
    RafCivilStamp stamp;

    if (dst != (char *)0 && cap != 0u)
        dst[0] = 0;

    now = time((time_t *)0);
    if (now == (time_t)-1)
        return 0u;
    if (localtime_r(&now, &tmv) == (struct tm *)0)
        return 0u;

    stamp.year = (RafI32)(tmv.tm_year + 1900);
    stamp.month = (RafU32)(tmv.tm_mon + 1);
    stamp.day = (RafU32)tmv.tm_mday;
    stamp.hour = (RafU32)tmv.tm_hour;
    stamp.minute = (RafU32)tmv.tm_min;
    stamp.second = (RafU32)tmv.tm_sec;
    stamp.valid = 1u;

    return raf_runid_render(dst, cap, &stamp);
}

#endif
