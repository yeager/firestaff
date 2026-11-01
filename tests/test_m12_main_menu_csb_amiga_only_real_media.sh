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
        "$app" --debug --verbose --width 1920 --height 1080 \
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

# A successful handoff does not prove that the ordinary no-game menu can
# finish CSB's Amiga startup and enter its first native runtime frame. Repeat
# the same authentic route without the test-only early-exit switch and record
# the runtime receipt.
runtime_log="$menu_home/runtime.log"
runtime_probe="$menu_home/runtime.json"
(
    unset FIRESTAFF_DATA FIRESTAFF_ORIGINALS_DIR FIRESTAFF_EXIT_AFTER_LAUNCH
    FIRESTAFF_CONFIG_PATH="$menu_home/runtime-config.toml" \
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$runtime_probe" \
    HOME="$menu_home" XDG_CONFIG_HOME="$menu_home" APPDATA="$menu_home" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$app" --debug --width 1920 --height 1080 \
        --script 'wait-scan-complete,wait20,key:down,wait20,key:enter,wait20,key:enter,wait20,key:enter,wait700,click:100:100,wait10,key:enter' \
        --duration 30000
) >"$runtime_log" 2>&1 || {
    cat "$runtime_log" >&2
    exit 1
}

if [[ $(grep -Fc 'launch phase=game-handoff' "$runtime_log") != 1 ]] ||
   ! grep -Fq "launch phase=game-handoff mode=menu game=csb platform=Amiga edition=amiga31-en source=$expected_menu_source" "$runtime_log"; then
    printf '%s\n' 'FAIL: CSB Amiga no-game menu runtime route did not make exactly one authentic handoff' >&2
    grep -E 'startup game=|scan complete|CSB READY|game-handoff|failed|error' "$runtime_log" >&2 || true
    exit 1
fi
if ! grep -Eq 'startup-source-complete game=csb phase=amiga-swsh-ftl profile=ff3872baaed8ee4e83ee3c0684b2eeec frames=[1-9][0-9]* source-vblanks=30 audio=' "$runtime_log"; then
    printf '%s\n' 'FAIL: CSB Amiga 3.1 English did not play its authenticated SWSH intro' >&2
    grep -E 'startup-source-select|amiga-swsh-ftl|swsh-intro|game-handoff|failed|error' "$runtime_log" >&2 || true
    exit 1
fi

python3 - "$runtime_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as source:
    probe = json.load(source)
startup = probe["startup"]
party = probe["party"]
if (probe.get("launchedEver") != 1 or probe.get("active") != 1 or
        probe.get("sourceId") != "csb" or
        startup.get("receiptReady") != 1 or
        startup.get("startupActive") != 0 or
        startup.get("levelLoaded") != 1 or
        startup.get("phase") != "inactive" or
        (party.get("mapIndex"), party.get("mapX"), party.get("mapY"),
         party.get("direction"), party.get("championCount")) != (0, 9, 0, 2, 0) or
        not probe.get("csbViewportHash") or
        probe.get("script") != {"waitFramesRemaining": 0, "pending": 0}):
    raise SystemExit(f"FAIL: CSB Amiga no-game menu missed its authentic first runtime: {probe}")
print("PASS: CSB Amiga-only no-game M12 menu reached the authentic first runtime")
PY

if [[ "$archive_hash_before" != "$(firestaff_sha256_file "$archive")" ]]; then
    printf '%s\n' 'FAIL: original CSB archive changed during no-game menu runtime' >&2
    exit 1
fi

printf '%s\n' 'PASS: no-game menu discovers authentic CSB Amiga ZIP -> ADF media, reaches its first runtime, and preserves the archive'
