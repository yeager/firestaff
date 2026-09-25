#!/usr/bin/env bash
set -euo pipefail

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
source_archive=${FIRESTAFF_DM1_ATARI_ST_SOURCE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Atari_ST,Software.7z"}
extractor=${FIRESTAFF_7ZZ:-7zz}
member=${FIRESTAFF_DM1_ATARI_ST_MEMBER:-'Floppy Disks STX/Dungeon Master for Atati ST v1.1 (English).stx'}
expected_graphics_md5=${FIRESTAFF_DM1_ATARI_ST_GRAPHICS_MD5:-5095a13692702235d2e74f6b2b1367a9}
edition=${FIRESTAFF_DM1_ATARI_ST_EDITION:-'1.1'}
if [[ ! -x "$app" || ! -f "$source_archive" ]] || ! command -v "$extractor" >/dev/null 2>&1; then
    printf '%s\n' 'SKIP: authentic DM1 Atari ST 1.1 STX source or 7zz is not staged'
    exit 77
fi

scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch"
stage=$(mktemp -d "$scratch/dm1-atari-st-11.XXXXXX")
trap 'rm -rf "$stage"' EXIT
inner="$stage/Dungeon Master (1987)(FTL)[!].zip"
outer="$stage/Dungeon-Master_Atari-ST_EN.zip"
stx="$stage/Dungeon Master (1987)(FTL)[!].stx"
"$extractor" e -so "$source_archive" "$member" >"$stx" 2>/dev/null
[[ -s "$stx" ]] || { echo 'FAIL: extracted authentic Atari ST 1.1 STX is empty' >&2; exit 1; }
(
    cd "$stage"
    zip -q "$inner" "$(basename "$stx")"
    zip -q "$outer" "$(basename "$inner")"
)

probe=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform atari-st --data-dir "$outer" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
    printf '%s\n' "$probe" >&2
    exit 1
}
if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$probe" ||
   ! grep -Fq "assetMd5=$expected_graphics_md5" <<<"$probe" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$probe" ||
   ! grep -Fq 'levelLoaded=1' <<<"$probe"; then
    printf '%s\n' "$probe" >&2
    echo "FAIL: authentic Atari ST $edition CLI boot did not select its authenticated graphics" >&2
    exit 1
fi

menu=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$outer" \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu" ||
   ! grep -Fq 'GRAPHICS.DAT' <<<"$menu"; then
    printf '%s\n' "$menu" >&2
    echo "FAIL: authentic Atari ST $edition media did not reach the M12 start-menu handoff" >&2
    exit 1
fi

runtime_probe="$stage/runtime.json"
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$outer" \
    --script 'enter,enter,enter,wait30,enter,wait60,enter' \
    --duration 20000 >/dev/null 2>&1
python3 - "$runtime_probe" "$edition" <<'PY'
import json
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
    raise SystemExit(f"FAIL: authentic DM1 Atari ST {sys.argv[2]} menu did not reach its runtime state: {probe}")
print(f"PASS: authentic DM1 Atari ST {sys.argv[2]} M12 menu reached the source runtime state")
PY

echo "PASS: authentic DM1 Atari ST $edition source reaches CLI and normal M12-to-M11 menu runtime"
