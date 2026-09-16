#pragma once
#include <int.h>

struct Time {
    u8 secs;
    u8 mins;
    u8 hours;
    u8 day;
    u8 month;
    u16 years;

    Time() : secs(0), mins(0), hours(0), day(0), month(0), years(0xDEAD) {}
    auto fromCurrTime() -> void {
        // TODO
    }
};
