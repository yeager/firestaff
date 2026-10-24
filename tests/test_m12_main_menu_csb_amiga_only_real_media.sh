#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_CSB_AMIGA_ADF_ARCHIVE:-"$HOME/.firestaff/data/csb/Chaos Strikes Back (FTL).zip"}
scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: Firestaff executable or authentic CSB Amiga ZIP -> ADF archive unavailable'
    exit 77
fi

source "$(dirname "$0")/lib/portable_sha256.sh"
archive_hash_before=$(firestaff_sha256_file "$archive")
mkdir -p "$scratch_root"
menu_home=$(mktemp -d "$scratch_root/m12-csb-amiga-only.XXXXXX")
menu_log="$menu_home/menu.log"
trap 'rc=$?; if ((rc)) && [[ -f "$menu_log" ]]; then cat "$menu_log" >&2; fi; rm -rf "$menu_home"' EXIT

menu_csb_root="$menu_home/.firestaff/data/csb"
mkdir -p "$menu_csb_root"
ln -s "$archive" "$menu_csb_root/$(basename "$archive")"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR
    FIRESTAFF_CONFIG_PATH="$menu_home/config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    HOME="$menu_home" XDG_CONFIG_HOME="$menu_home" APPDATA="$menu_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 1920 --height 1080 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter' \
        --duration 90000
) >"$menu_log" 2>&1

expected_menu_source="$menu_csb_root/$(basename "$archive")::Chaos Strikes Back (FTL) A.adf::Graphics.DAT"
if ! grep -Fq 'startup game=none mode=menu' "$menu_log" ||
   ! grep -Fq 'scan complete' "$menu_log" ||
   ! grep -Fq 'games=dm1:no,csb:yes,dm2:no,nexus:no,theron:no' "$menu_log" ||
   ! grep -Fq 'CSB READY: gameId=csb' "$menu_log" ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Amiga edition=amiga31-en source=$expected_menu_source" "$menu_log"; then
    printf '%s\n' 'FAIL: no-game menu did not discover and launch authentic CSB Amiga media' >&2
    grep -E 'startup game=|scan complete|CSB READY|game-handoff|failed|error' "$menu_log" >&2 || true
    exit 1
fi

if [[ "$archive_hash_before" != "$(firestaff_sha256_file "$archive")" ]]; then
    printf '%s\n' 'FAIL: original CSB archive changed during no-game menu scan and launch' >&2
    exit 1
fi

printf '%s\n' 'PASS: no-game menu discovers authentic CSB Amiga ZIP -> ADF media and hands off to CSB without modifying the archive'
