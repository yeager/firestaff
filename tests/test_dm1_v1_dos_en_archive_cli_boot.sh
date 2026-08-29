#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

app=${1:?usage: test_dm1_v1_dos_en_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_DOS_EN_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN.zip"}
expected_graphics_md5=fa6b1aa29e191418713bf2cda93d962e

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 English DOS archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_graphics_md5" <<<"$output" &&
    grep -Fq "dataDir=$archive::dungeon-master/dmaster/DATA/GRAPHICS.DAT" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

# The source remains in its distribution ZIP.  The native DOS IMG3 path must
# bind its members through virtual archive paths, never a staged extraction.
probe --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform pc --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive::dungeon-master/dmaster/DATA/GRAPHICS.DAT" <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic English DOS ZIP start menu did not bind the native IMG3 route' >&2
    exit 1
fi

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script enter,enter,enter,up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic English DOS ZIP did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic English DM1 DOS ZIP reaches CLI, start menu, and native movement in memory'
