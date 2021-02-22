#define LOG_MODULE "x86"

#include <windows.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "util/mem.h"

#pragma pack(push, 1)

struct addr_relative_replacement {
    uint8_t opcode;
    uint32_t addr_offset;
    uint8_t nop;
};
static_assert(sizeof(struct addr_relative_replacement) == 6, "addr_relative_replacement size");

#pragma pack(pop)

// For changing data segment calls (`FF yy xx xx xx xx`) to relative jumps
// (`E8 xx xx xx xx 90`)
void do_relative_jmp(HANDLE process, void *target, const void *new_addr)
{
    uint32_t offset = ((uintptr_t) new_addr) - ((uintptr_t) target) -
                      sizeof(struct addr_relative_replacement) + 1;

    const struct addr_relative_replacement data = {
        .opcode = 0xE8u,
        .addr_offset = offset,
        .nop = 0x90u,
    };
    memory_write(process, target, &data, sizeof(data));
}

// For changing data segment loads (`8B yy xx xx xx xx`) to absolute pointers
// (`BE xx xx xx xx 90`)
void do_absolute_jmp(HANDLE process, void *target, const uint32_t new_addr)
{
    const struct addr_relative_replacement data = {
        .opcode = 0xBEu,
        .addr_offset = new_addr,
        .nop = 0x90u,
    };
    memory_write(process, target, &data, sizeof(data));
}
