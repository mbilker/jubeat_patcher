#define LOG_MODULE "omni-festo-common"

#include "festo.h"

#include "imports/avs2-core/avs.h"

#include "pattern/pattern.h"

#include "pe/iat.h"

#include "util/log.h"
#include "util/patch.h"

// clang-format off

// This one is used to read in the .bin files and needs to be larger than the
// actual files - at the default 64MB size loads start failing with about a 32
// meg .bin
#define BUF2_ORIG_SIZE 0x4000000
#define BUF2_NEW_SIZE  0x8000000

const struct patch_t file_loader_patch_1 {
    .name = "file loader patch 1",
    .pattern = { 0x72, 0x22, 0x8D, 0x83, U32_TO_CONST_BYTES_LE(BUF2_ORIG_SIZE - 1) },
    .data = {U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE - 1)},
    .data_offset = 4,
};
const struct patch_t file_loader_patch_2 {
    .name = "file loader patch 2",
    .pattern = { 0xB9, U32_TO_CONST_BYTES_LE(BUF2_ORIG_SIZE - 1) },
    .data = {U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE - 1)},
    .data_offset = 1,
};

const struct patch_t file_loader_patch_3 {
    .name = "file loader patch 3",
    // fn start, push    4000000h
    .pattern = { 0xCC, 0x68, U32_TO_CONST_BYTES_LE(BUF2_ORIG_SIZE) },
    .data = {U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE)},
    .data_offset = 2,
};

const struct patch_t file_loader_patch_4 {
    .name = "file loader patch 4",
    // push    4000000h; push 0
    .pattern = { 0x68, U32_TO_CONST_BYTES_LE(BUF2_ORIG_SIZE), 0x6A },
    .data = {U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE)},
    .data_offset = 1,
};

const struct patch_t file_loader_patch_5 {
    .name = "file loader patch 5",
    // lea     ecx, [esi+4000000h]
    .pattern = { 0x8D, 0x8E, U32_TO_CONST_BYTES_LE(BUF2_ORIG_SIZE) },
    .data = {U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE)},
    .data_offset = 2,
};

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


    if (n == 1296) {
        log_body_info("ultimate", "hooked d3_initialize");

        auto s_ptr = reinterpret_cast<uintptr_t>(s);
        auto limit = reinterpret_cast<uint16_t *>(s_ptr + 0xAEE8);
        auto buf = reinterpret_cast<void **>(s_ptr - 0x6F3598);
        auto buf2 = reinterpret_cast<void **>(s_ptr + 0x5c4);

        const size_t new_sz = 8192 * 28 * 4;
        size_t new_limit = new_sz / 28;
        void *new_buf = malloc(new_sz);
        void *new_buf2 = malloc(BUF2_NEW_SIZE);

        log_body_info("ultimate", "d3 limit %d->%d buf %p->%p buf_2 %p->%p",
            *limit, new_limit, *buf, new_buf, *buf2, new_buf2);

        *buf = new_buf;
        *limit = new_limit;

        *buf2 = new_buf2;
        // buf2 is copied into with no checks and has no specified limit except
        // for the ones in D3FileLoader::Update and similar
    }

    return memset(s, c, n);
}

uint8_t *find_pattern_checked(const char *name, const MODULEINFO &module_info, const std::vector<uint8_t> &data, const std::vector<char> &_mask) {
    log_info("Looking for %s", name);
    const char* mask = _mask.size() ? _mask.data() : static_cast<const char*>(NULL);

    auto addr = find_pattern(
        reinterpret_cast<uint8_t *>(module_info.lpBaseOfDll),
        module_info.SizeOfImage,
        data.data(),
        reinterpret_cast<const bool*>(mask),
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

    // Increase file loading memory so we can use much bigger .bin files.
    // Make the #defines bigger if you get the error: "failed to get file buffer."
    do_patch(process, jubeat_info, file_loader_patch_1);
    do_patch(process, jubeat_info, file_loader_patch_2);
    do_patch(process, jubeat_info, file_loader_patch_3);
    do_patch(process, jubeat_info, file_loader_patch_4);
    do_patch(process, jubeat_info, file_loader_patch_5);

    // Fun fun fun times - the init function we *just* hooked is called at CRT
    // init, which is too late to be hooked here. So call the deinit function,
    // then re-init...

    // void **__stdcall init_file_buf(int a1, int a2)
    // {
    // _DWORD esi2; // esi
    // void **result; // eax
    //
    // file_buf_d3 = 0;
    // file_buf_end = 0;
    // dword_FD85080 = 0;
    // if ( sub_FB1B8B0((unsigned int *)&file_buf_d3, 0x4000000u) )
    uint8_t *file_buf_init_fn_addr = find_pattern_checked(
        "d3_file_buf_init_fn", jubeat_info,
        { 0xCC, 0x68, U32_TO_CONST_BYTES_LE(BUF2_NEW_SIZE) },
        {}
    ) + 1;

    // void __cdecl deinit_file_buf()
    // {
    // _DWORD ecx1; // ecx
    //
    // ecx1 = file_buf_d3;
    // if ( file_buf_d3 )
    // {
    //     if ( (unsigned int)(dword_FD85080 - (_DWORD)file_buf_d3) >= 0x1000 )
    //     {
    uint8_t *file_buf_deinit_fn_addr = find_pattern_checked(
        "d3_file_buf_deinit_fn", jubeat_info,
        { 0xCC, 0x8B, 0x0D, 0x78, 0x50, 0xD8, 0x0F, 0x85, 0xC9, 0x74, 0x4E },
        {    1,    1,    1,    0,    0,    0,    0,    1,    1,    1,    1 }
    ) + 1;
    auto file_buf_init_fn = reinterpret_cast<void **(__stdcall *)(int a1, int a2)>(file_buf_init_fn_addr);
    auto file_buf_deinit_fn = reinterpret_cast<void (__cdecl *)()>(file_buf_deinit_fn_addr);

    file_buf_deinit_fn();
    file_buf_init_fn(0,0);
}
