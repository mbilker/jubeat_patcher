#ifdef VERBOSE
#define LOG_MODULE "pattern"
#endif

#include <stdio.h>

#include "pattern.h"

#ifdef VERBOSE
#include "util/log.h"

char *to_hex(const uint8_t *data, size_t data_len) {
    char *output = (char *) malloc(data_len * 3);

    char *current = output;
    for (size_t i = 0; i < data_len; i++) {
        current += sprintf(current, "%02x ", data[i]);
    }

    *(current - 1) = '\0';
    *current = '\0';

    return output;
}
#endif

uint8_t *find_pattern(uint8_t *data, size_t data_size, const uint8_t *pattern, const bool *pattern_mask, size_t pattern_size) {
    size_t i, j;
    bool pattern_found;

    for (i = 0; i < data_size - pattern_size; i++) {
        pattern_found = true;

        if (pattern_mask == NULL) {
            for (j = 0; j < pattern_size; j++) {
                if (data[i + j] != pattern[j]) {
                    pattern_found = false;
                    break;
                }
            }
        } else {
            for (j = 0; j < pattern_size; j++) {
                if (pattern_mask[j] && data[i + j] != pattern[j]) {
                    pattern_found = false;
                    break;
                }
            }
        }

        if (pattern_found) {
#ifdef VERBOSE
            log_info("pattern found at index %x size %d", i, pattern_size);
#endif

            return &data[i];
        }
    }

    return NULL;
}
