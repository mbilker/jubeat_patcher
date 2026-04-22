#define LOG_MODULE "omni-festo-common"

#include "festo.h"

#include "imports/avs2-core/avs.h"

#include "pattern/pattern.h"

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

// force unlock markers "theme"
const struct patch_t marker_unlock_patch_1 {
    .name = "marker unlock patch 1",
    .pattern = { 0x75, 0x47, 0x0F },
    .data = { 0xEB },
    .data_offset = 0,
};
const struct patch_t marker_unlock_patch_2 {
    .name = "marker unlock patch 2",
    .pattern = { 0x75, 0x2B, 0x0F, 0x28, 0x44, 0x24, 0x40, 0x0F, 0x29, 0x05, 0xD0 },
    .data = { 0xEB },
    .data_offset = 0,
};
const struct patch_t marker_unlock_patch_3 {
    .name = "marker unlock patch 3",
    .pattern = { 0x0F, 0xB7, 0x45, 0xB0, 0x89, 0x41 },
    .data = { 0x31, 0xC0, 0x90, 0x90 },
    .data_offset = 0,
};

// force unlock backgrounds
const struct patch_t background_unlock_patch_1 {
    .name = "background unlock patch 1",
    .pattern = { 0x75, 0x43, 0x0F },
    .data = { 0xEB },
    .data_offset = 0,
};
const struct patch_t background_unlock_patch_2 {
    .name = "background unlock patch 2",
    .pattern = { 0x75, 0x2B, 0x0F, 0x28, 0x44, 0x24, 0x40, 0x0F, 0x29, 0x05, 0xF0 },
    .data = { 0xEB },
    .data_offset = 0,
};
const struct patch_t background_unlock_patch_3 {
    .name = "background unlock patch 3",
    .pattern = { 0x0F, 0xB7, 0x45, 0xB0, 0x89, 0x04 },
    .data = { 0x31, 0xC0, 0x90, 0x90 },
    .data_offset = 0,
};

// clang-format on

#ifdef D3_GUARD_ENABLE
struct d3_guard_state {
    bool captured;
    uint32_t *pixel_pool_anchor;
    uint32_t *pixel_pool_ceiling;
    uint32_t *pixel_pool_watermark;
    uint32_t pixel_pool_initial;
    uint32_t *tex_handle_table_cap;
    void **tex_data_handle_table;
    uint32_t *tex_handle_lower_clamp;
};

static d3_guard_state g_d3_guard = {};
#endif

// d3_initialize calls this at its very end, so we use the lucky uniqueness
// of the size parameter to overwrite the texture memory + texture limit
// after it's called
static void *__cdecl mem_set(void *s, int c, size_t n)
{
    // Hey so you think these offsets need changing? well here's how to find
    // 'em. First find d3_initialize which is super easy - the function that
    // calls it prints a message, naming it. Next up are the buffer address and
    // buffer size variables. The initialisation inlines a lot of copies into
    // xmmwords which makes it irritating, but here's an example decompilation
    // with the key variables labelled - keep in mind that 'limit' was
    // initially mis-identified as an xmmword and I had to turn it into 4 DWORDs
    // to fix the decompile.
    //   buf = dword_102C0B38 + 0x200000;
    //   dword_109C3334 = 28 * limit + dword_102C0B38 + 0x200000;
    //   buf_2 = dword_102C0B38 + 0x200000 + 28 * limit + 4 * dword_109C3200;
    //   dword_109B88D8 = buf_2;
    //   dword_109B88DC = buf_2;
    //   dword_109B88E8 = buf_2 - dword_109B88CC;
    //   mem_set(&s, 0, 1296);
    //
    // Per-build offsets (all relative to s):
    //   festo (2018-05-24): s = 0x109b8310
    //     d3_pixel_pool_anchor    = s + 0x5BC
    //     d3_pixel_pool_ceiling   = s + 0x5D4
    //     d3_pixel_pool_watermark = s + 0x5D8
    //     d3_tex_config.limit     = s + 0xAEE8  (u16)
    //     d3_tex_config.clamp_cap = s + 0xAEEC
    //     d3_tex_handle_table_cap = s + 0xAEF0
    //     d3_tex_handle_table     = s + 0xB024
    //     d3_tex_buffer_ptr       = s - 0x6F3598
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

#ifdef D3_GUARD_ENABLE
        g_d3_guard.pixel_pool_anchor = reinterpret_cast<uint32_t *>(s_ptr + 0x5BC);
        g_d3_guard.pixel_pool_ceiling = reinterpret_cast<uint32_t *>(s_ptr + 0x5D4);
        g_d3_guard.pixel_pool_watermark = reinterpret_cast<uint32_t *>(s_ptr + 0x5D8);
        g_d3_guard.pixel_pool_initial = *g_d3_guard.pixel_pool_ceiling;
        g_d3_guard.tex_handle_lower_clamp = reinterpret_cast<uint32_t *>(s_ptr + 0xAEEC);
        g_d3_guard.tex_handle_table_cap = reinterpret_cast<uint32_t *>(s_ptr + 0xAEF0);
        g_d3_guard.tex_data_handle_table = reinterpret_cast<void **>(s_ptr + 0xB024);
        g_d3_guard.captured = true;

        log_body_info(
            "ultimate",
            "d3 guard captured: pixel pool initial=%u bytes (%.1f MiB), "
            "handle cap=%u slots, clamp floor=%u",
            g_d3_guard.pixel_pool_initial,
            g_d3_guard.pixel_pool_initial / 1048576.0,
            *g_d3_guard.tex_handle_table_cap,
            *g_d3_guard.tex_handle_lower_clamp);
#endif
    }

    return memset(s, c, n);
}

#ifdef D3_GUARD_ENABLE
void festo_d3_guard_check(const char *context)
{
    if (!g_d3_guard.captured)
        return;

    // d3_pixel_pool_watermark is peak cumulative upload offset — monotonically
    // non-decreasing. d3_pixel_pool_ceiling is a constant placement anchor, not
    // a running budget; watermark reaching it means the next upload overflows.
    uint32_t pool_peak = *g_d3_guard.pixel_pool_watermark;
    uint32_t pool_cap = g_d3_guard.pixel_pool_initial;
    uint32_t pool_free = (pool_peak < pool_cap) ? (pool_cap - pool_peak) : 0;
    unsigned pool_pct = static_cast<unsigned>((static_cast<uint64_t>(pool_peak) * 100) / pool_cap);

    if (pool_free < 0x100000) {
        log_body_warning(
            "d3-guard",
            "[%s] PIXEL POOL CRITICAL: peak 0x%x / cap 0x%x (%u%% used, %u bytes free). "
            "Next upload larger than remaining free bytes will corrupt memory "
            "before d3_pixel_pool_anchor.",
            context,
            pool_peak,
            pool_cap,
            pool_pct,
            pool_free);
    } else if (pool_pct >= 80) {
        log_body_warning(
            "d3-guard",
            "[%s] pixel pool %u%% used (peak 0x%x / cap 0x%x, %u bytes free)",
            context,
            pool_pct,
            pool_peak,
            pool_cap,
            pool_free);
    } else {
        log_body_info(
            "d3-guard",
            "[%s] pixel pool %u%% used (peak 0x%x / cap 0x%x, %u bytes free)",
            context,
            pool_pct,
            pool_peak,
            pool_cap,
            pool_free);
    }

    // Post-boot, [lo, hi) is expected to be fully occupied by stock texpacks —
    // full is steady state, not an error.
    {
        uint32_t lo = *g_d3_guard.tex_handle_lower_clamp;
        uint32_t hi = *g_d3_guard.tex_handle_table_cap;
        if (hi > lo && hi <= 256) {
            unsigned used_slots = 0;
            for (uint32_t i = lo; i < hi; i++) {
                if (g_d3_guard.tex_data_handle_table[i] != nullptr) {
                    used_slots++;
                }
            }
            log_body_info(
                "d3-guard",
                "[%s] handle table %u/%u slots populated (valid range [%u,%u))",
                context,
                used_slots,
                static_cast<unsigned>(hi - lo),
                lo,
                hi);
        }
    }
}
#endif // D3_GUARD_ENABLE

uint8_t *find_pattern_checked(
    const char *name,
    const MODULEINFO &module_info,
    const std::vector<uint8_t> &data,
    const std::vector<char> &_mask)
{
    log_info("Looking for %s", name);
    const char *mask = _mask.size() ? _mask.data() : static_cast<const char *>(NULL);

    auto addr = find_pattern(
        reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
        module_info.SizeOfImage,
        data.data(),
        reinterpret_cast<const bool *>(mask),
        data.size());

    log_assert(addr);

    log_info("Found at %p", addr);

    return addr;
}

void festo_apply_common_patches(
    HANDLE process,
    HMODULE jubeat_handle,
    const MODULEINFO &jubeat_info,
    const MODULEINFO &music_db_info)
{
    // jubeat.dll
    do_patch(process, jubeat_info, marker_unlock_patch_1);
    do_patch(process, jubeat_info, marker_unlock_patch_2);
    do_patch(process, jubeat_info, marker_unlock_patch_3);
    do_patch(process, jubeat_info, background_unlock_patch_1);
    do_patch(process, jubeat_info, background_unlock_patch_2);
    do_patch(process, jubeat_info, background_unlock_patch_3);

    // music_db.dll
    do_patch(process, music_db_info, song_unlock_patch);

    // increase d3 texture memory so all our added banners don't crash the game
    hook_iat_ordinal(
        process, jubeat_handle, "avs2-core.dll", 0xF4, reinterpret_cast<void *>(mem_set));
}
