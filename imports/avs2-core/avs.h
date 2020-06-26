#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_CHECK_FMT __attribute__(( format(printf, 2, 3) ))

void __declspec(dllimport) log_body_misc(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_info(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_warning(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_fatal(const char *module, const char *fmt, ...) LOG_CHECK_FMT;

size_t __declspec(dllimport) avs_strlcpy(char *dst, const char *src, size_t dst_size);
size_t __declspec(dllimport) avs_snprintf(char *dst, size_t dst_size, const char *fmt, ...);

void *__declspec(dllimport) property_mem_read(
        void *prop,
        void *prop_node,
        unsigned int flags,
        uint8_t *buffer,
        uint32_t buffer_size);

#ifdef __cplusplus
};
#endif
