#pragma once

#include "util/patch.h"

#define U32_TO_CONST_BYTES_LE(x) \
    static_cast<uint8_t>((x) & 0xff), \
    static_cast<uint8_t>(((x) >> 8) & 0xff), \
    static_cast<uint8_t>(((x) >> 16) & 0xff), \
    static_cast<uint8_t>(((x) >> 24) & 0xff)

void festo_apply_common_patches(
    HANDLE process,
    HMODULE jubeat_handle,
    const MODULEINFO &jubeat_info,
    const MODULEINFO &music_db_info);
