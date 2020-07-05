#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_CHECK_FMT __attribute__(( format(printf, 2, 3) ))

enum property_type {
    PROP_TYPE_node = 1,
    PROP_TYPE_s8 = 2,
    PROP_TYPE_u8 = 3,
    PROP_TYPE_s16 = 4,
    PROP_TYPE_u16 = 5,
    PROP_TYPE_s32 = 6,
    PROP_TYPE_u32 = 7,
    PROP_TYPE_s64 = 8,
    PROP_TYPE_u64 = 9,
    PROP_TYPE_bin = 10,
    PROP_TYPE_str = 11,
    PROP_TYPE_ip4 = 12,
    PROP_TYPE_time = 13,
    PROP_TYPE_float = 14,
    PROP_TYPE_double = 15,
    PROP_TYPE_2s8 = 16,
    PROP_TYPE_2u8 = 17,
    PROP_TYPE_2s16 = 18,
    PROP_TYPE_2u16 = 19,
    PROP_TYPE_2s32 = 20,
    PROP_TYPE_2u32 = 21,
    PROP_TYPE_2s64 = 22,
    PROP_TYPE_2u64 = 23,
    PROP_TYPE_2f = 24,
    PROP_TYPE_2d = 25,
    PROP_TYPE_3s8 = 26,
    PROP_TYPE_3u8 = 27,
    PROP_TYPE_3s16 = 28,
    PROP_TYPE_3u16 = 29,
    PROP_TYPE_3s32 = 30,
    PROP_TYPE_3u32 = 31,
    PROP_TYPE_3s64 = 32,
    PROP_TYPE_3u64 = 33,
    PROP_TYPE_3f = 34,
    PROP_TYPE_3d = 35,
    PROP_TYPE_4s8 = 36,
    PROP_TYPE_4u8 = 37,
    PROP_TYPE_4s16 = 38,
    PROP_TYPE_4u16 = 39,
    PROP_TYPE_4s32 = 40,
    PROP_TYPE_4u32 = 41,
    PROP_TYPE_4s64 = 42,
    PROP_TYPE_4u64 = 43,
    PROP_TYPE_4f = 44,
    PROP_TYPE_4d = 45,
    PROP_TYPE_attr = 46,
    PROP_TYPE_attr_and_node = 47,
    PROP_TYPE_vs8 = 48,
    PROP_TYPE_vu8 = 49,
    PROP_TYPE_vs16 = 50,
    PROP_TYPE_vu16 = 51,
    PROP_TYPE_bool = 52,
    PROP_TYPE_2b = 53,
    PROP_TYPE_3b = 54,
    PROP_TYPE_4b = 55,
    PROP_TYPE_vb = 56,
};

enum prop_traverse_option {
    TRAVERSE_PARENT = 0,
    TRAVERSE_FIRST_CHILD = 1,
    TRAVERSE_FIRST_ATTR = 2,
    TRAVERSE_FIRST_SIBLING = 3,
    TRAVERSE_NEXT_SIBLING = 4,
    TRAVERSE_PREVIOUS_SIBLING = 5,
    TRAVERSE_LAST_SIBLING = 6,
    TRAVERSE_NEXT_SEARCH_RESULT = 7,
    TRAVERSE_PREV_SEARCH_RESULT = 8
};

enum prop_create_flag {
    PROP_XML                  = 0x000,
    PROP_READ                 = 0x001,
    PROP_WRITE                = 0x002,
    PROP_CREATE               = 0x004,
    PROP_BINARY               = 0x008,
    PROP_APPEND               = 0x010,
    PROP_XML_HEADER           = 0x100,
    PROP_DEBUG_VERBOSE        = 0x400,
    PROP_JSON                 = 0x800,
    PROP_BIN_PLAIN_NODE_NAMES = 0x1000,
};

void __declspec(dllimport) log_body_misc(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_info(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_warning(const char *module, const char *fmt, ...) LOG_CHECK_FMT;
void __declspec(dllimport) log_body_fatal(const char *module, const char *fmt, ...) LOG_CHECK_FMT;

size_t __declspec(dllimport) avs_strlcpy(char *dst, const char *src, size_t dst_size);
size_t __declspec(dllimport) avs_snprintf(char *dst, size_t dst_size, const char *fmt, ...);
void __declspec(dllimport) avs_thread_delay(int time, int a2);

void *__declspec(dllimport) property_mem_read(
        void *src_buf,
        size_t src_size,
        unsigned int flags,
        void *buffer,
        uint32_t buffer_size);
void __declspec(dllimport) property_destroy(void *prop);
void __declspec(dllimport) property_clear_error(void *prop);
void *__declspec(dllimport) property_node_traversal(void *prop, enum prop_traverse_option opt);
void *__declspec(dllimport) property_search(void *prop, void *node, const char *path);
int __declspec(dllimport) property_node_refer(
        void* prop,
        void* node,
        const char* path,
        enum property_type type,
        void* dest,
        int size);

#ifdef __cplusplus
};
#endif
