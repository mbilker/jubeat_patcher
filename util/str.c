#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "str.h"

size_t str_format(char *buf, size_t nchars, const char *fmt, ...)
{
    va_list ap;
    size_t result;

    va_start(ap, fmt);
    result = str_vformat(buf, nchars, fmt, ap);
    va_end(ap);

    return result;
}

size_t str_vformat(char *buf, size_t nchars, const char *fmt, va_list ap)
{
    int result;

    result = _vsnprintf(buf, nchars, fmt, ap);

    if (result >= (int) nchars || result < 0) {
        abort();
    }

    return (size_t) result;
}
