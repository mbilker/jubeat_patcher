#!/usr/bin/env bash

set -eux

CROSS="${CROSS:-cross-clang-mingw-32.ini}"
# if you have a globally installed meson, set UVX=
UVX="${UVX:-uvx}"

WIPE=()
for arg in "$@"; do
    if [[ "$arg" == "--wipe" ]]; then
        WIPE=(--wipe)
    fi
done

$UVX meson setup --cross-file "$CROSS" "${WIPE[@]}" build
$UVX meson compile -C build
