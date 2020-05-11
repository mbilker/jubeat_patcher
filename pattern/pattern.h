#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VERBOSE
char *to_hex(const uint8_t *data, size_t data_size);
#endif

uint8_t *find_pattern(
        uint8_t *data,
        size_t data_size,
        const uint8_t *pattern,
        const bool *pattern_mask,
        size_t pattern_size);

#ifdef __cplusplus
};
#endif
