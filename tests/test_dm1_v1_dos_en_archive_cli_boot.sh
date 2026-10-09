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

native_graphics_path_matches() {
    local output
    output=$(cat)
    grep -Fq "dataDir=$archive::DATA/GRAPHICS.DAT" <<<"$output" ||
    grep -Fq "dataDir=$archive::dungeon-master/dmaster/DATA/GRAPHICS.DAT" <<<"$output"
}

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_graphics_md5" <<<"$output" &&
    native_graphics_path_matches <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

# The source remains in its distribution ZIP.  The native DOS IMG3 path must
# bind its members through virtual archive paths, never a staged extraction.
# "dos" is the public spelling for the PC/DOS source route.
probe --game dm1 --platform dos --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
direct_boot_probe_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
    printf '%s\n' "$direct_boot_probe_output" >&2
    exit 1
}
if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$direct_boot_probe_output" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$direct_boot_probe_output" ||
   ! native_graphics_path_matches <<<"$direct_boot_probe_output" ||
   ! grep -Fq 'dm1StartupHandoffExecuted=1' <<<"$direct_boot_probe_output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$direct_boot_probe_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$direct_boot_probe_output"; then
    printf '%s\n' "$direct_boot_probe_output" >&2
    printf '%s\n' 'FAIL: DM1 direct boot probe did not apply the Hall runtime handoff before its first frame' >&2
    exit 1
fi

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! native_graphics_path_matches <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic English DOS ZIP start menu did not bind the native IMG3 route' >&2
    exit 1
fi

# Verify the normal per-user installation layout with no --data-dir,
# FIRESTAFF_DATA, or explicit platform. Keep the original DOS ZIP in place and
# expose it through an isolated HOME so the scanner must discover it there.
test_scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$test_scratch"
default_home=$(mktemp -d "$test_scratch/dm1-default-data.XXXXXX")
default_data_root="$default_home/.firestaff/data"
default_archive="$default_data_root/dm1/$(basename "$archive")"
default_log="$default_home/menu.log"
mkdir -p "$(dirname "$default_archive")"
if ! ln -s "$archive" "$default_archive" 2>/dev/null; then
    # Fall back to a same-volume hard link on Windows hosts without symlink
    # privileges; this keeps the original game archive in place.
    ln "$archive" "$default_archive"
fi
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    HOME="$default_home" XDG_CONFIG_HOME="$default_home/.config" \
    APPDATA="$default_home" FIRESTAFF_CONFIG_PATH="$default_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 --debug --script enter,enter,enter --duration 1000
) >"$default_log" 2>&1 || {
    cat "$default_log" >&2
    exit 1
}
if ! grep -Fq "DM1 READY: gameId=dm1" "$default_log" ||
   { ! grep -Fq "dataDir=$default_archive::DATA/GRAPHICS.DAT" "$default_log" &&
     ! grep -Fq "dataDir=$default_archive::dungeon-master/dmaster/DATA/GRAPHICS.DAT" "$default_log"; } ||
   ! grep -Fq 'handoff=pc-img3' "$default_log"; then
    printf '%s\n' 'FAIL: DM1 AUTO menu did not discover authentic DOS media from ~/.firestaff/data/dm1 without --data-dir' >&2
    cat "$default_log" >&2
    exit 1
fi
echo 'PASS: DM1 AUTO start menu discovers authentic DOS media from ~/.firestaff/data/dm1 without --data-dir'
find "$default_home" -depth -delete

# Keep the actual M12 -> DOS menu handoff running through the first playable
# frame. The launch receipt above alone cannot prove that the selected source
# reached its native level and party state.
menu_runtime_probe_json="$test_scratch/dm1-dos-menu-runtime-$$.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$menu_runtime_probe_json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" \
    --script enter,enter,enter --duration 10000 >/dev/null 2>&1
python3 - "$menu_runtime_probe_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["phase"] != "dm1-runtime" or startup["active"] != 1 or
        startup["startupActive"] != 0 or startup["levelLoaded"] != 1 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(
        f"FAIL: authentic DM1 English DOS M12 route did not reach the first "
        f"runtime frame: {probe}")
print("PASS: authentic DM1 English DOS start menu reached the first runtime frame")
PY
rm -f "$menu_runtime_probe_json"

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
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

for mode in v1 v20 v21; do
    case "$mode" in v1) mode_index=0;; v20) mode_index=1;; v21) mode_index=2;; esac
    for route in cli menu; do
        if [[ "$route" == menu ]]; then
            mode_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
                SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
                --menu --game dm1 --platform pc --data-dir "$archive" \
                --script enter,enter,enter --presentation-mode "$mode" \
                --duration 1000 2>&1) || {
                printf '%s\n' "$mode_output" >&2; exit 1;
            }
            if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$mode_output" ||
               ! native_graphics_path_matches <<<"$mode_output" ||
               ! grep -Fq 'handoff=pc-img3' <<<"$mode_output"; then
                printf '%s\n' "$mode_output" >&2
                printf 'FAIL: DOS menu did not launch the selected %s route\n' "$mode" >&2
                exit 1
            fi
        else
            mode_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
                --game dm1 --platform pc --data-dir "$archive" \
                --presentation-mode "$mode" --boot-probe \
                --boot-probe-frames 2 --duration 0 2>&1) || {
                printf '%s\n' "$mode_output" >&2; exit 1;
            }
            if ! grep -Fq "presentationMode=$mode_index " <<<"$mode_output" ||
               ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$mode_output" ||
               ! grep -Fq 'phase=dm1-runtime' <<<"$mode_output" ||
               ! grep -Fq 'levelLoaded=1' <<<"$mode_output"; then
                printf '%s\n' "$mode_output" >&2
                printf 'FAIL: DOS CLI did not retain requested %s presentation\n' "$mode" >&2
                exit 1
            fi
        fi
    done
done
printf '%s\n' 'PASS: authentic English DM1 DOS ZIP reaches CLI/menu in Original, Filtered and Upscaled modes, with native movement in memory'
