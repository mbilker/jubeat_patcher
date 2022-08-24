#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t str_format(char *buf, size_t nchars, const char *fmt, ...);
size_t str_vformat(char *buf, size_t nchars, const char *fmt, va_list ap);

#ifdef __cplusplus
};
#endif

inline bool str_eq(const char *lhs, const char *rhs)
{
    if (lhs == NULL || rhs == NULL) {
        return lhs == rhs;
    }

    return strcmp(lhs, rhs) == 0;
}

inline bool str_ieq(const char *lhs, const char *rhs)
{
    if (lhs == NULL || rhs == NULL) {
        return lhs == rhs;
    }

    return stricmp(lhs, rhs) == 0;
}
