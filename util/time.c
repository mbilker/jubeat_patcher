#include "time.h"

#include <windows.h>

static LARGE_INTEGER counter_freq = { .QuadPart = 0 };

uint64_t time_get_counter(void)
{
    LARGE_INTEGER time;

    QueryPerformanceCounter(&time);
    return (uint64_t) time.QuadPart;
}

uint64_t time_get_elapsed_ns(uint64_t counter_delta)
{
    if (counter_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&counter_freq);
    }

    return (counter_delta * 1000) / counter_freq.QuadPart;
}

uint64_t time_get_elapsed_us(uint64_t counter_delta)
{
    if (counter_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&counter_freq);
    }

    return (counter_delta * 1000000) / counter_freq.QuadPart;
}

uint32_t time_get_elapsed_ms(uint64_t counter_delta)
{
    if (counter_freq.QuadPart == 0) {
        QueryPerformanceFrequency(&counter_freq);
    }

    return (counter_delta * 1000000000l) / counter_freq.QuadPart;
}