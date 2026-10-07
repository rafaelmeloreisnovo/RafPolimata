#ifndef RAFBBS_RUNID_CORE_H
#define RAFBBS_RUNID_CORE_H

#include "rafbbs_types.h"

#define RAFBBS_RUN_ID_LEN 15u
#define RAFBBS_RUN_ID_CAP 16u

typedef struct {
    RafI32 year;
    RafU32 month;
    RafU32 day;
    RafU32 hour;
    RafU32 minute;
    RafU32 second;
    RafU32 valid;
} RafCivilStamp;

static inline RafU32 raf_civil_is_leap(RafI32 year)
{
    RafI32 y4;
    RafI32 y100;
    RafI32 y400;

    if (year < 0)
        return 0u;
    y4 = year;
    while (y4 >= 4)
        y4 -= 4;
    if (y4 != 0)
        return 0u;

    y100 = year;
    while (y100 >= 100)
        y100 -= 100;
    if (y100 != 0)
        return 1u;

    y400 = year;
    while (y400 >= 400)
        y400 -= 400;
    return (RafU32)(y400 == 0);
}

static inline RafU32 raf_civil_days_in_month(RafI32 year, RafU32 month)
{
    if (month == 2u)
        return raf_civil_is_leap(year) != 0u ? 29u : 28u;
    if (month == 4u || month == 6u || month == 9u || month == 11u)
        return 30u;
    if (month >= 1u && month <= 12u)
        return 31u;
    return 0u;
}

static inline RafU32 raf_civil_stamp_valid(const RafCivilStamp *stamp)
{
    RafU32 max_day;
    if (stamp == (const RafCivilStamp *)0 || stamp->valid == 0u)
        return 0u;
    if (stamp->year < 0 || stamp->year > 9999)
        return 0u;
    max_day = raf_civil_days_in_month(stamp->year, stamp->month);
    if (max_day == 0u || stamp->day < 1u || stamp->day > max_day)
        return 0u;
    if (stamp->hour > 23u || stamp->minute > 59u || stamp->second > 60u)
        return 0u;
    return 1u;
}

static inline void raf_runid_put2(char *dst, RafU32 value)
{
    RafU32 tens = 0u;
    while (value >= 10u) {
        value -= 10u;
        ++tens;
    }
    dst[0] = (char)('0' + tens);
    dst[1] = (char)('0' + value);
}

static inline void raf_runid_put4(char *dst, RafU32 value)
{
    RafU32 thousands = 0u;
    RafU32 hundreds = 0u;
    RafU32 tens = 0u;

    while (value >= 1000u) {
        value -= 1000u;
        ++thousands;
    }
    while (value >= 100u) {
        value -= 100u;
        ++hundreds;
    }
    while (value >= 10u) {
        value -= 10u;
        ++tens;
    }

    dst[0] = (char)('0' + thousands);
    dst[1] = (char)('0' + hundreds);
    dst[2] = (char)('0' + tens);
    dst[3] = (char)('0' + value);
}

static inline RafU32 raf_runid_render(
    char *dst,
    RafU32 cap,
    const RafCivilStamp *stamp
)
{
    if (dst == (char *)0 || cap < RAFBBS_RUN_ID_CAP)
        return 0u;
    dst[0] = 0;
    if (raf_civil_stamp_valid(stamp) == 0u)
        return 0u;

    raf_runid_put4(dst, (RafU32)stamp->year);
    raf_runid_put2(dst + 4u, stamp->month);
    raf_runid_put2(dst + 6u, stamp->day);
    dst[8] = '-';
    raf_runid_put2(dst + 9u, stamp->hour);
    raf_runid_put2(dst + 11u, stamp->minute);
    raf_runid_put2(dst + 13u, stamp->second);
    dst[15] = 0;
    return 1u;
}

#endif
