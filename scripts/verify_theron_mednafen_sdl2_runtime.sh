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

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo=$(CDPATH= cd -- "$script_dir/.." && pwd)

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

scratch_root=${THERON_CAPTURE_SCRATCH_ROOT:-"$repo/.codex-scratch"}
mkdir -p "$scratch_root"
probe_home=$(mktemp -d "$scratch_root/firestaff-theron-sdl-version.XXXXXX")
cleanup_probe_home() {
    if [[ -n ${probe_home:-} && -d "$probe_home" ]]; then
        rm -rf -- "$probe_home"
    fi
}
trap cleanup_probe_home EXIT

probe_status=0
probe_output=$(MEDNAFEN_HOME="$probe_home" SDL_VIDEODRIVER=dummy \
    SDL_AUDIODRIVER=dummy "$mednafen_bin" -help 2>&1) || probe_status=$?
compiled_sdl_version=$(printf '%s\n' "$probe_output" | sed -n \
    's/.*Compiled against SDL \([0-9][0-9.]*\)(.*/\1/p' | head -n 1)
running_sdl_version=$(printf '%s\n' "$probe_output" | sed -n \
    's/.*running with SDL \([0-9][0-9.]*\)(.*/\1/p' | head -n 1)
if [[ ! "$compiled_sdl_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ||
      ! "$running_sdl_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    printf 'BLOCKED: Mednafen SDL version probe did not report compiled and runtime versions (exit=%s)\n' \
        "$probe_status" >&2
    exit 1
fi

version_is_at_least() {
    local required=$1 actual=$2
    local required_major required_minor required_patch
    local actual_major actual_minor actual_patch
    IFS=. read -r required_major required_minor required_patch <<< "$required"
    IFS=. read -r actual_major actual_minor actual_patch <<< "$actual"
    (( actual_major > required_major )) && return 0
    (( actual_major < required_major )) && return 1
    (( actual_minor > required_minor )) && return 0
    (( actual_minor < required_minor )) && return 1
    (( actual_patch >= required_patch ))
}

if ! version_is_at_least "$compiled_sdl_version" "$running_sdl_version"; then
    printf 'BLOCKED: Mednafen SDL runtime %s is older than compiled headers %s\n' \
        "$running_sdl_version" "$compiled_sdl_version" >&2
    exit 1
fi
if ! grep -Fq 'Usage:' <<< "$probe_output"; then
    printf 'BLOCKED: Mednafen failed its isolated SDL startup probe (exit=%s)\n' \
        "$probe_status" >&2
    exit 1
fi

printf 'PASS: Mednafen SDL runtime %s satisfies compiled headers %s via isolated startup; linked %s\n' \
    "$running_sdl_version" "$compiled_sdl_version" "$sdl2_path"
