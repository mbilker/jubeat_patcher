#pragma once

#include "util/patch.h"

void festo_apply_common_patches(
    HANDLE process, HMODULE jubeat_module, const MODULEINFO &music_db_info);
