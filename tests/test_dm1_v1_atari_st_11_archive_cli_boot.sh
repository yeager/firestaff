#!/usr/bin/env bash
set -euo pipefail

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
source_archive=${FIRESTAFF_DM1_ATARI_ST_11_SOURCE:-"$HOME/.firestaff/data/dm1/Game,Dungeon_Master,Atari_ST,Software.7z"}
extractor=${FIRESTAFF_7ZZ:-7zz}
member='Floppy Disks STX/Dungeon Master for Atati ST v1.1 (English).stx'
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
   ! grep -Fq 'assetMd5=5095a13692702235d2e74f6b2b1367a9' <<<"$probe" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$probe" ||
   ! grep -Fq 'levelLoaded=1' <<<"$probe"; then
    printf '%s\n' "$probe" >&2
    echo 'FAIL: authentic Atari ST 1.1 CLI boot did not select its authenticated graphics' >&2
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
    echo 'FAIL: authentic Atari ST 1.1 media did not reach the M12 start-menu handoff' >&2
    exit 1
fi

echo 'PASS: authentic DM1 Atari ST 1.1 source reaches CLI runtime and M12 menu handoff'
