#!/bin/sh
# Verify the visible Nexus card flow using only pointer input.  The retail
# Saturn CUE/BIN remains in place; this test gives Firestaff its absolute
# source path and checks the native title gate rather than synthesizing a VDP
# capture that is not present in the corpus.
set -eu

firestaff_cli="${1:?Firestaff executable is required}"
build_root="${2:?CTest build directory is required}"
data_root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
cue="$data_root/Dungeon Master Nexus (Japan).cue"

if [ ! -x "$firestaff_cli" ] || [ ! -f "$cue" ]; then
    echo "SKIP: authentic Nexus Saturn CUE or Firestaff executable is unavailable"
    exit 77
fi

# Do not inherit an operator's online credentials or launcher preferences.
# Keep the throwaway HOME inside the build tree, never /tmp or the game-data
# tree, and remove it once the test completes.
test_home="$(mktemp -d "$build_root/firestaff-nexus-menu-mouse.XXXXXX")"
cleanup() { rm -rf "$test_home"; }
trap cleanup EXIT HUP INT TERM

unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS
output="$(HOME="$test_home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
    FIRESTAFF_EXIT_AFTER_LAUNCH=1 SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff_cli" --width 1920 --height 1080 --menu --game nexus \
    --platform saturn --data-dir "$data_root" \
    --script 'wait20,click:700:728,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 3000 2>&1)" || {
    printf '%s\n' "$output" >&2
    exit 1
}

if ! printf '%s\n' "$output" | grep -Fq \
        'NEXUS STARTUP RECEIPT: status=blocked gameId=nexus' ||
   ! printf '%s\n' "$output" | grep -Fq \
        'blocker=title-vdp-capture-required' ||
   ! printf '%s\n' "$output" | grep -Fq 'Nexus: opened disc image ' ||
   ! printf '%s\n' "$output" | grep -Fq 'TITLE.CG/4bpp-atlas' ||
   ! printf '%s\n' "$output" | grep -Fq \
        'Nexus V1 engine initialized (source: ISO)'; then
    echo "FAIL: pointer-only Nexus card flow did not retain the real Saturn title route" >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

echo "PASS: Nexus mouse cards select the native Saturn title route in memory"
