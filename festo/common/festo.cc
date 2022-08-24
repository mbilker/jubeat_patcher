#define LOG_MODULE "omni-festo-common"

#include "festo.h"

#include "imports/avs2-core/avs.h"

#include "pe/iat.h"

#include "util/log.h"
#include "util/patch.h"

// clang-format off

const struct patch_t song_unlock_patch {
    .name = "song unlock patch",
    .pattern = { 0x74, 0x0A, 0x8B, 0x4C },
    .data = { 0x90, 0x90 },
    .data_offset = 0,
};

// clang-format on

// d3_initialize calls this at its very end, so we use the lucky uniqueness
// of the size parameter to overwrite the texture memory + texture limit
// after it's called
static void *__cdecl mem_set(void *s, int c, size_t n)
{
    if (n == 1296) {
        log_body_info("ultimate", "hooked d3_initialize");

        auto s_ptr = reinterpret_cast<uintptr_t>(s);
        auto limit = reinterpret_cast<uint16_t *>(s_ptr + 0xAEE8);
        auto buf = reinterpret_cast<void **>(s_ptr - 0x6F3598);

        const size_t new_sz = 8192 * 28 * 4;
        size_t new_limit = new_sz / 28;
        void *new_buf = malloc(new_sz);

        log_body_info("ultimate", "d3 limit %d->%d buf %p->%p", *limit, new_limit, *buf, new_buf);

        *buf = new_buf;
        *limit = new_limit;
    }

    return memset(s, c, n);
}

void festo_apply_common_patches(
    HANDLE process, HMODULE jubeat_handle, const MODULEINFO &music_db_info)
{
    do_patch(process, music_db_info, song_unlock_patch);

    // increase d3 texture memory so all our added banners don't crash the game
    hook_iat_ordinal(
        process, jubeat_handle, "avs2-core.dll", 0xF4, reinterpret_cast<void *>(mem_set));
}
