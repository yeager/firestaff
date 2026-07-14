#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" != 1 ]]; then
    printf 'usage: %s MEDNAFEN_BINARY\n' "$0" >&2
    exit 2
fi

mednafen_bin=$1
if [[ ! -x "$mednafen_bin" ]]; then
    printf 'FAIL: Mednafen binary is unavailable\n' >&2
    exit 1
fi

if [[ "$(uname -s)" != Darwin ]]; then
    printf 'SKIP: SDL2 runtime linkage check is macOS-only\n'
    exit 0
fi
if ! command -v otool >/dev/null 2>&1; then
    printf 'FAIL: otool is required to verify the Mednafen SDL2 runtime\n' >&2
    exit 1
fi

sdl2_path=$(otool -L "$mednafen_bin" | awk '/libSDL2-2\.0\.0\.dylib/ { print $1; exit }')
if [[ -z "$sdl2_path" ]]; then
    printf 'FAIL: Mednafen has no directly linked SDL2 runtime\n' >&2
    exit 1
fi
if [[ "$sdl2_path" == *sdl2-compat* ]]; then
    printf 'BLOCKED: Mednafen links sdl2-compat (%s); use a real SDL2 runtime for authentic Quartz/SDL capture\n' "$sdl2_path" >&2
    exit 1
fi

printf 'PASS: Mednafen links real SDL2 runtime %s\n' "$sdl2_path"
