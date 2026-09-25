#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_ATARI_ST_DE_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_DE_Version-12.zip"}
expected_md5=2bdc5f431f84c0ece738f54dbd787c3b

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic German DM1 Atari ST 1.2 archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq "assetMd5=$expected_md5" <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic German DM1 Atari ST start menu did not bind DMCSB1 source media' >&2
    exit 1
fi

# Follow the same normal M12 -> M11 title/entrance handoff as the English
# Atari ST v1.2 route; a separate boot probe cannot satisfy this receipt.
case "$app" in
    */*) app_dir=${app%/*} ;;
    *) app_dir=. ;;
esac
runtime_probe="$app_dir/dm1-atari-st-de-runtime-$$.json"
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch_root"
capture_dir=$(mktemp -d "$scratch_root/dm1-atari-runtime.XXXXXX")
trap 'rm -f "$runtime_probe"; rm -rf "$capture_dir"' EXIT
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$capture_dir" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" \
    --script 'enter,enter,enter,wait30,enter,wait60,enter' \
    --duration 20000 >/dev/null 2>&1
python3 - "$runtime_probe" "$capture_dir" <<'PY'
import json
import pathlib
import struct
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        startup["levelLoaded"] != 1 or startup["phase"] != "dm1-runtime" or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic German DM1 Atari start menu did not reach its source runtime state: {probe}")
captures = list(pathlib.Path(sys.argv[2]).glob("*.bmp"))
if len(captures) != 1:
    raise SystemExit(f"FAIL: expected one presented Atari runtime frame, got {len(captures)}")
bitmap = captures[0].read_bytes()
if len(bitmap) < 54 or bitmap[:2] != b"BM":
    raise SystemExit("FAIL: Atari runtime capture is not a BMP")
offset = struct.unpack_from("<I", bitmap, 10)[0]
width, signed_height = struct.unpack_from("<Ii", bitmap, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", bitmap, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if (bits != 24 or width < 320 or height < 200 or
        width % 320 != 0 or height % 200 != 0 or
        width // 320 != height // 200 or
        offset + stride * height > len(bitmap)):
    raise SystemExit("FAIL: unexpected Atari runtime capture geometry")
nonblack = 0
colours = set()
for row in range(height):
    start = offset + row * stride
    for column in range(width):
        pixel = bitmap[start + column * 3:start + column * 3 + 3]
        if pixel != b"\0\0\0":
            nonblack += 1
            colours.add(pixel)
scale = width // 320
if nonblack < 10000 * scale * scale or len(colours) < 4:
    raise SystemExit(
        "FAIL: authentic Atari start-menu runtime remains black "
        f"(nonblack={nonblack}, colours={len(colours)})")
print("PASS: authentic German DM1 Atari start menu reached runtime and presented "
      f"nonblack source pixels={nonblack} colours={len(colours)}")
PY

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic German DM1 Atari ST start menu did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic German DM1 Atari ST 1.2 ZIP -> STX reaches CLI, menu, and native movement runtime'
