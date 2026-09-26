#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

app=${1:?usage: test_dm1_v1_fmtowns_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
expected_md5=c10c512f63461ebe79b5ac365115b61b
expected_edm_md5=c27e7b984df9753912c3375dc121919f
expected_japanese_md5=edf47d7da5de8184604d6d80477ef01f
expected_jdm_md5=acfbcfa5d65032a4bcabc8d5ea062dcc

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 FM Towns archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq 'platformHandoff=fmtowns-tmenu-edm' <<<"$output" &&
    grep -Fq 'fmtownsProgram=EDM.EXP' <<<"$output" &&
    grep -Fq "fmtownsProgramMd5=$expected_edm_md5" <<<"$output" &&
    grep -Fq 'fmtownsMenuSelectsProgram=1' <<<"$output" &&
    grep -Fq 'dm1FmtownsMenuFontLoaded=1' <<<"$output" &&
    grep -Fq 'dm1FmtownsCddaTrack=5' <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

# "fmtowns" is accepted alongside the canonical hyphenated spelling.
probe --game dm1 --platform fmtowns --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --game dm1 --platform fm-towns --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

japanese_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform fm-towns --dm1-fmtowns-ja --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
    printf '%s\n' "$japanese_output" >&2
    exit 1
}
grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$japanese_output"
grep -Fq "assetMd5=$expected_japanese_md5" <<<"$japanese_output"
grep -Fq 'platformHandoff=fmtowns-tmenu-jdm' <<<"$japanese_output"
grep -Fq 'fmtownsProgram=JDM.EXP' <<<"$japanese_output"
grep -Fq "fmtownsProgramMd5=$expected_jdm_md5" <<<"$japanese_output"
grep -Fq 'fmtownsMenuSelectsProgram=1' <<<"$japanese_output"
grep -Fq 'dm1FmtownsMenuFontLoaded=1' <<<"$japanese_output"
grep -Fq 'dm1FmtownsCddaTrack=5' <<<"$japanese_output"
grep -Fq 'phase=dm1-runtime' <<<"$japanese_output"
grep -Fq 'levelLoaded=1' <<<"$japanese_output"

japanese_menu_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform fm-towns --dm1-fmtowns-ja \
    --data-dir "$archive" --script enter,enter,enter \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
    printf '%s\n' "$japanese_menu_output" >&2
    exit 1
}
grep -Fq "assetMd5=$expected_japanese_md5" <<<"$japanese_menu_output"
grep -Fq 'platformHandoff=fmtowns-tmenu-jdm' <<<"$japanese_menu_output"
grep -Fq 'fmtownsProgram=JDM.EXP' <<<"$japanese_menu_output"
grep -Fq "fmtownsProgramMd5=$expected_jdm_md5" <<<"$japanese_menu_output"
grep -Fq 'dm1FmtownsCddaTrack=5' <<<"$japanese_menu_output"
grep -Fq 'dm1FmtownsMenuFontLoaded=1' <<<"$japanese_menu_output"
grep -Fq 'phase=dm1-runtime' <<<"$japanese_menu_output"
grep -Fq 'levelLoaded=1' <<<"$japanese_menu_output"

# The public launcher is also fully pointer-navigable.  Use the native
# 1920x1080 card canvas explicitly so scripted physical coordinates do not
# depend on a CI host's default window size.  The waits model separate input
# frames; no keyboard event selects the game, platform, or presentation.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform fm-towns \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# The pointer-only check above stops at the launch receipt. Also keep the
# ordinary M12 handoff alive through the original EDM/JDM title and Entrance
# so a selected card cannot pass while its first runtime frame remains
# unreachable. The dummy display's bounded headless Entrance input is the
# test-only path used after the authentic title sequence.
expect_menu_runtime() {
    local language=$1 probe_root probe_file
    local -a edition_args=(--width 1920 --height 1080 --menu --game dm1
                          --platform fm-towns)
    if [[ $language == ja ]]; then
        edition_args+=(--dm1-fmtowns-ja)
    fi
    probe_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
    mkdir -p "$probe_root"
    probe_file="$probe_root/dm1-fmtowns-menu-runtime-$language-$$.json"
    FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe_file" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        "${edition_args[@]}" --data-dir "$archive" \
        --script 'wait20,click:700:262,wait20,click:410:405,wait20,click:450:405,wait20' \
        --duration 10000 >/dev/null 2>&1
    python3 - "$probe_file" <<'PY'
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
    raise SystemExit(f"FAIL: authentic DM1 FM Towns M12 {sys.argv[1]} did not reach its first runtime frame: {probe}")
print("PASS: authentic DM1 FM Towns M12 reached the first runtime frame")
PY
    rm -f "$probe_file"
}
expect_menu_runtime en
expect_menu_runtime ja

expect_gameplay_input() {
    local input=$1 expected_party=$2 gameplay_output
    local language=${3:-en} program=EDM.EXP handoff=fmtowns-tmenu-edm
    local program_md5=$expected_edm_md5 graphics_md5=$expected_md5
    set -- --game dm1 --platform fm-towns --data-dir "$archive"
    if [[ $language == ja ]]; then
        set -- "$@" --dm1-fmtowns-ja
        program=JDM.EXP
        handoff=fmtowns-tmenu-jdm
        program_md5=$expected_jdm_md5
        graphics_md5=$expected_japanese_md5
    fi
    gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" \
        --boot-probe --boot-probe-frames 100 --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$gameplay_output" >&2
        return 1
    }
    # After EDM/JDM's title track 02, the common entrance/HoC handoff
    # selects the authenticated FM Towns entrance map track. Track 02 here
    # would prove that title fell straight through to a party-less dungeon.
    if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
       ! grep -Fq "assetMd5=$graphics_md5" <<<"$gameplay_output" ||
       ! grep -Fq "platformHandoff=$handoff" <<<"$gameplay_output" ||
       ! grep -Fq "fmtownsProgram=$program" <<<"$gameplay_output" ||
       ! grep -Fq "fmtownsProgramMd5=$program_md5" <<<"$gameplay_output" ||
       ! grep -Fq 'fmtownsMenuSelectsProgram=1' <<<"$gameplay_output" ||
       ! grep -Fq 'dm1FmtownsMenuFontLoaded=1' <<<"$gameplay_output" ||
       ! grep -Fq 'dm1StartupHandoffExecuted=1' <<<"$gameplay_output" ||
       ! grep -Fq 'dm1FmtownsCddaPlaying=1' <<<"$gameplay_output" ||
       ! grep -Fq 'dm1FmtownsCddaTrack=5' <<<"$gameplay_output" ||
       ! grep -Fq "map=0 party=$expected_party" <<<"$gameplay_output"; then
        printf '%s\n' "$gameplay_output" >&2
        printf 'FAIL: authentic DM1 FM Towns %s %s input did not reach native runtime\n' "$language" "$input" >&2
        return 1
    fi
}

# Each command starts from the same original-disc session.  This prevents a
# prior movement from changing the map context for the next source-backed
# assertion, while covering the complete public directional input contract.
expect_gameplay_input up           1,4,2
expect_gameplay_input down         1,3,2
expect_gameplay_input left         1,3,1
expect_gameplay_input right        1,3,3
expect_gameplay_input strafe-left  1,3,2
expect_gameplay_input strafe-right 1,3,2
expect_gameplay_input action       1,3,2

# Independently reload the Japanese JDATA route for each input. Its own
# graphics/program fingerprints prevent an English fallback from passing.
expect_gameplay_input up           1,4,2 ja
expect_gameplay_input down         1,3,2 ja
expect_gameplay_input left         1,3,1 ja
expect_gameplay_input right        1,3,3 ja
expect_gameplay_input strafe-left  1,3,2 ja
expect_gameplay_input strafe-right 1,3,2 ja
expect_gameplay_input action       1,3,2 ja

if [[ ${FIRESTAFF_TEST_VISUAL_PARITY:-0} == 1 ]]; then
    # Visual parity is tracked separately from start and input support. Opt
    # in only when the image review is ready to be resumed.
    scratch_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
    mkdir -p "$scratch_root"
    capture_dir=$(mktemp -d "$scratch_root/dm1-fmtowns-live-frame.XXXXXX")
    cleanup_capture() { rm -rf "$capture_dir"; }
    trap cleanup_capture EXIT HUP INT TERM
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$capture_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --game dm1 --platform fm-towns --data-dir "$archive" \
        --script wait300 --duration 5000 >/dev/null 2>&1
    python3 - "$capture_dir" <<'PY'
import pathlib
import struct
import sys

files = list(pathlib.Path(sys.argv[1]).glob("*.bmp"))
if len(files) != 1:
    raise SystemExit("FAIL: expected exactly one DM1 FM Towns presented frame")
blob = files[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: DM1 FM Towns capture is not a BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, height = struct.unpack_from("<Ii", blob, 18)
height = abs(height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if width <= 0 or height <= 0 or bits != 24 or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid DM1 FM Towns capture geometry")
nonblack = 0
colours = set()
for row in range(height):
    start = offset + row * stride
    for column in range(width):
        pixel = blob[start + column * 3:start + column * 3 + 3]
        if pixel != b"\0\0\0":
            nonblack += 1
            colours.add(pixel)
if nonblack < 100000 or len(colours) < 4:
    raise SystemExit(
        "FAIL: DM1 FM Towns live view lacks original F20 scenery "
        f"(nonblack={nonblack}, colours={len(colours)})")
print(f"PASS: DM1 FM Towns live F20 view nonblack={nonblack} colours={len(colours)}")
PY
fi

printf '%s\n' 'PASS: authentic DM1 FM Towns ZIP reaches CLI, menu, TMENU/EDM and TMENU/JDM handoffs, plus native English and Japanese input matrices in memory'
