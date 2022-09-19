#define LOG_MODULE "ultimate::sound"

#include <vector>

#include "hook/table.h"

#include "pe/iat.h"

#include "util/log.h"
#include "util/patch.h"

#include "../common/festo.h"

// look for string "se file size sum is over", you'll find it
#define OLD_MAX_SONG_LEN 7864320  // 7.5MiB
#define NEW_MAX_SONG_LEN 15728640 // 15MiB

#define SONG_LEN_EXTRA (NEW_MAX_SONG_LEN - OLD_MAX_SONG_LEN)

// the only call to GFMemCreate
#define OLD_SE_HEAP 60293120
#define NEW_SE_HEAP (OLD_SE_HEAP + SONG_LEN_EXTRA)

#define OLD_LIMIT_CHECK (OLD_SE_HEAP - OLD_MAX_SONG_LEN)
#define NEW_LIMIT_CHECK (NEW_SE_HEAP - NEW_MAX_SONG_LEN)

int (*orig_GFMemCreate)(void *mem, size_t len);
int GFMemCreate(void *mem, size_t len);

static const struct hook_symbol gftools_hooks[] = {
    {
        .name = "GFMemCreate",
        .ordinal = 0,
        .patch = reinterpret_cast<void *>(GFMemCreate),
        .link = reinterpret_cast<void**>(&orig_GFMemCreate),
    },
};

const struct patch_t sound_limit {
    .name = "sound buffer limit",
    // cmp     eax, 52428800
    .pattern = { 0x3D, U32_TO_CONST_BYTES_LE(OLD_LIMIT_CHECK) },
    .data = { U32_TO_CONST_BYTES_LE(NEW_LIMIT_CHECK) },
    .data_offset = 1,
};

// not strictly required but may prevent spurious bug reports from people
const struct patch_t sound_warning {
    .name = "sound buffer warning",
    // cmp     ecx, 7864320
    .pattern = { 0x81, 0xF9, U32_TO_CONST_BYTES_LE(OLD_MAX_SONG_LEN) },
    .data = { U32_TO_CONST_BYTES_LE(NEW_MAX_SONG_LEN) },
    .data_offset = 2,
};

int GFMemCreate(void *mem, uint32_t len) {
    log_info("GFMemCreate buf len %u -> %u", len, NEW_SE_HEAP);

    mem = malloc(NEW_SE_HEAP);

    return orig_GFMemCreate(mem, NEW_SE_HEAP);
}

void hook_sound(HANDLE process)
{
    MODULEINFO sound_info;
    HMODULE sound = GetModuleHandleA("sound.dll");
    log_assert(sound);

    if (!GetModuleInformation(process, sound, &sound_info, sizeof(sound_info))) {
        log_fatal("GetModuleInformation(\"sound.dll\") failed: 0x%08lx", GetLastError());
    }

    iat_hook_table_apply(
        process, sound, "gftools.dll", gftools_hooks, std::size(gftools_hooks));

    do_patch(process, sound_info, sound_limit);
    do_patch(process, sound_info, sound_warning);
}
