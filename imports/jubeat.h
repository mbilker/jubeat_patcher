#pragma once

#include <stdbool.h>

bool __declspec(dllimport) jb_dll_entry_init(char *, void *);
bool __declspec(dllimport) jb_dll_entry_main(void);
