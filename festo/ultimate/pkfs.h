#pragma once

#ifdef __cplusplus
extern "C" {
#endif

size_t __cdecl pkfs_avs_strlcpy(char *dst, const char *src, size_t dst_size);
size_t __cdecl pkfs_avs_strlen(const char *str);
int __cdecl pkfs_avs_snprintf(char *dst, size_t dst_size, const char *fmt, ...);

#ifdef __cplusplus
};
#endif
