//#define LOG_MODULE "extend::pkfs"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "imports/avs2-core/avs.h"

//#include "util/log.h"
#include "util/str.h"

size_t __cdecl pkfs_avs_strlcpy(char *dst, const char *src, size_t dst_size)
{
    // log_misc("avs_strlcpy(%p, \"%s\", %u)", dst, src, dst_size);

    // Bail early if the length is not expected
    if (dst_size != 9) {
        return avs_strlcpy(dst, src, dst_size);
    }

    if (src[8] < '0' || src[8] > '9') {
        return avs_strlcpy(dst, src, 9);
    }
    if (src[0] == '0') {
        return avs_strlcpy(dst, &src[1], 9);
    }

    return avs_strlcpy(dst, src, 10);
}

size_t __cdecl pkfs_avs_strlen(const char *str)
{
    size_t len = strlen(str);
    // log_misc("avs_strlen(\"%s\") = %u", str, len);

    if (len >= 8 && len <= 9) {
        len = 8;
    }

    return len;
}

int __cdecl pkfs_avs_snprintf(char *dst, size_t dst_size, const char *fmt, ...)
{
    // log_misc("avs_snprintf(%p, %u, \"%s\")", dst, dst_size, fmt);

    const char *music_id, *str2, *dir, *msc;
    char music_dir[7];

    va_list args;
    va_start(args, fmt);

    if (str_eq(fmt, "/data/imagefs/msc/%s/%s")) {
        music_id = va_arg(args, const char *);
        str2 = va_arg(args, const char *);
        va_end(args);

        if (strlen(music_id) == 9) {
            if (music_id[0] == '/') {
                return snprintf(dst, dst_size, fmt, &music_id[1], &str2[1]);
            } else {
                return snprintf(dst, dst_size, fmt, music_id, &str2[1]);
            }
        } else {
            return snprintf(dst, dst_size, fmt, music_id, str2);
        }
    } else if (str_eq(fmt, "%s/d%c%c%c%c%c%c%c/%s_%s.ifs")) {
        dir = va_arg(args, const char *);
        for (size_t i = 0; i < 7; i++) {
            music_dir[i] = (char) va_arg(args, int);
        }
        music_id = va_arg(args, const char *);
        msc = va_arg(args, const char *);
        va_end(args);

        if (strlen(music_id) == 9) {
            if (music_id[0] == '0') {
                return snprintf(
                    dst,
                    dst_size,
                    fmt,
                    dir,
                    music_id[1],
                    music_id[2],
                    music_id[3],
                    music_id[4],
                    music_id[5],
                    music_id[6],
                    music_id[7],
                    &music_id[1],
                    msc);
            } else {
                return snprintf(
                    dst,
                    dst_size,
                    "%s/d%c%c%c%c%c%c%c%c/%s_%s.ifs",
                    "/data/ifs_pack",
                    music_id[0],
                    music_id[1],
                    music_id[2],
                    music_id[3],
                    music_id[4],
                    music_id[5],
                    music_id[6],
                    music_id[7],
                    music_id,
                    msc);
            }
        } else {
            return snprintf(
                dst,
                dst_size,
                fmt,
                dir,
                music_dir[0],
                music_dir[1],
                music_dir[2],
                music_dir[3],
                music_dir[4],
                music_dir[5],
                music_dir[6],
                music_id,
                msc);
        }
    } else if (str_eq(fmt, "/data/imagefs/%s/%s")) {
        music_id = va_arg(args, const char *);
        str2 = va_arg(args, const char *);
        va_end(args);

        if (strlen(str2) == 9 && str2[0] == '0') {
            return snprintf(dst, dst_size, fmt, music_id, &str2[1]);
        } else {
            return snprintf(dst, dst_size, fmt, music_id, str2);
        }
    } else {
        return vsnprintf(dst, dst_size, fmt, args);
    }
}
