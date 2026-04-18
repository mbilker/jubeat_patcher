#!/usr/bin/env bash

set -eux

CROSS="${CROSS:-cross-clang-mingw-32.ini}"

meson setup --cross-file "$CROSS" build
meson compile -C build
