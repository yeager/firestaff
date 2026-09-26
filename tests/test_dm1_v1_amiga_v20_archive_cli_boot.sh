#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_amiga_v20_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_AMIGA_V20_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Amiga_EN_Version-20.zip"}
expected_md5=6a2f135b53c2220f0251fa103e2a6e7e
selected_media="$archive::Dungeon Master v2.0 (1988)(FTL).zip::Dungeon Master v2.0 (1988)(FTL).adf"
selected_save="$archive::Dungeon Master v2.0 (1988)(FTL)[save disk].zip::Dungeon Master v2.0 (1988)(FTL)[save disk].adf::DMGAMEG.DAT"

# This preservation package is ZIP -> ZIP -> ADF, all of which Firestaff
# reads through its native bounded-memory readers.  Keep the real-media
# regression independent of the diagnostic-only external archive opt-in: a
# developer shell must not accidentally hide a runtime dependency here.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga 2.0 preservation archive is not staged'
    exit 77
fi

# Native virtual-media ingestion is read-only.  Retain the supplied outer
# archive hash across both normal boot and save-resume routes.
archive_hash_before=$(sha256sum "$archive")

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq "dataDir=$archive" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform amiga --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

# The ordinary v2.0 save disk is ZIP -> ADF inside the same preservation
# package. Exercise its real F0435 session through direct CLI, without
# extracting or rewriting any member. These tuple values are read from the
# authenticated DMGAMEG.DAT body, not a generated fixture. The M12 menu path
# below separately verifies that the selected nested ADF remains the launch
# owner; boot-probe deliberately cannot be combined with --menu.
probe_save_resume() {
    probe "$@" --save "$selected_save" --boot-probe \
        --boot-probe-expect-runtime --boot-probe-expect-level-loaded 1 \
        --boot-probe-expect-map 0 --boot-probe-expect-party 4,15,2 \
        --boot-probe-expect-champions 4 \
        --boot-probe-expect-runtime-tick-min 292 \
        --boot-probe-expect-runtime-tick-max 292 --duration 0
}
probe_save_resume --game dm1 --platform amiga --data-dir "$archive"

# Apply one source-backed input per fresh resume.  The right-strafe is
# particularly important: this exact original session crosses from map 0 to
# map 1, so it exercises native F0435 party/world adoption, map lookup and
# stair transition rather than merely proving that the ADF can be parsed.
# All expected tuples below are observed from the authenticated save, never
# inferred from a synthetic dungeon or generated save state.
probe_saved_input() {
    local input=$1
    local map=$2
    local party=$3
    probe --game dm1 --platform amiga --data-dir "$archive" \
        --save "$selected_save" --boot-probe --boot-probe-expect-runtime \
        --boot-probe-expect-level-loaded 1 --boot-probe-expect-map "$map" \
        --boot-probe-expect-party "$party" --boot-probe-expect-champions 4 \
        --boot-probe-expect-runtime-tick-min 293 \
        --boot-probe-expect-runtime-tick-max 293 \
        --script "$input" --duration 0
}
probe_saved_input up 0 4,16,2
probe_saved_input down 0 4,14,2
probe_saved_input left 0 4,15,1
probe_saved_input right 0 4,15,3
probe_saved_input strafe-left 0 4,15,2
probe_saved_input strafe-right 1 3,1,0
probe_saved_input action 0 4,15,2

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform amiga --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$selected_media" <<<"$menu_output" ||
   ! grep -Fq 'handoff=amiga-img2' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 Amiga 2.0 start menu did not bind its selected IMG2 disk media' >&2
    exit 1
fi

# The ordinary New Game route above proves the selected virtual disk owns the
# launch. Also drive M12's Quick Resume row with the authenticated save-disk
# member and require the exact F0435 pose that the direct CLI path restored.
# This closes the menu-to-save handoff without manufacturing or extracting a
# save file.
resume_probe_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$resume_probe_root"
resume_probe="$resume_probe_root/dm1-amiga-save-menu-runtime-$$.json"
cleanup_resume_probe() {
    rm -f "$resume_probe"
}
trap cleanup_resume_probe EXIT HUP INT TERM
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$resume_probe" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" \
    --save "$selected_save" --script enter,enter,enter --duration 3000 \
    >/dev/null 2>&1
python3 - "$resume_probe" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["active"] != 1 or startup["startupActive"] != 0 or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 4, 15, 2, 4) or
        probe["gameTick"] != 292):
    raise SystemExit(f"FAIL: authentic DM1 Amiga M12 Quick Resume failed: {probe}")
print("PASS: authentic DM1 Amiga M12 Quick Resume restored the saved runtime pose")
PY

# Amiga is the first card on the platform picker's second row.  The nested
# ZIP -> ADF route must remain launchable with pointer input alone.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform amiga \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:410:679,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 Amiga 2.0 up input did not reach native movement' >&2
    exit 1
fi

[[ "$archive_hash_before" == "$(sha256sum "$archive")" ]]

printf '%s\n' 'PASS: authentic DM1 Amiga 2.0 ZIP -> ZIP -> ADF reaches CLI, menu, and native movement runtime'
