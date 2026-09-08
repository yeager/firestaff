#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
# Card startup flow: game card -> verified PC card -> Original card.
menu_original=enter,enter,enter

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 PC-34 archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq 'assetMd5=fa6b1aa29e191418713bf2cda93d962e' <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 120 \
    --duration 0
probe --menu --game dm1 --platform pc --data-dir "$archive" --script "$menu_original" \
    --boot-probe --boot-probe-frames 120 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 --platform pc \
    --data-dir "$archive" --script "$menu_original" --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 start menu did not bind IMG3 source media' >&2
    exit 1
fi

# Physical card coordinates use the explicit launcher canvas, making this a
# real mouse-only game -> PC -> Original launch rather than a keyboard alias.
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game dm1 --platform pc \
    --data-dir "$archive" \
    --script 'wait20,click:700:262,wait20,click:934:405,wait20,click:450:405,wait20' \
    --duration 3000 >/dev/null 2>&1

# The authentic PC-3.4 dungeon starts at (map=0,x=1,y=3,dir=2).  A native
# `up` input advances to y=4; this proves the selected archive has reached
# the actual M11 movement route rather than only a title/startup receipt.
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 500 \
    --script up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 up input did not reach native movement' >&2
    exit 1
fi

# Every row begins from a fresh native archive session.  The original PC 3.4
# ZIP supplies the title, dungeon and party state throughout; do not replace
# it with a generated save, an extracted fixture, or a synthetic map.
probe_runtime_input() {
    local input=$1
    local expected_party=$2
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game dm1 --platform pc --data-dir "$archive" \
        --boot-probe --boot-probe-frames 500 --script "$input" --duration 0 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output" &&
    grep -Fq "map=0 party=$expected_party" <<<"$output"
}

probe_runtime_input down 1,3,2
probe_runtime_input left 1,3,1
probe_runtime_input right 1,3,3
probe_runtime_input strafe-left 1,3,2
probe_runtime_input strafe-right 1,3,2
probe_runtime_input action 1,3,2

# Authentic Hall of Champions route, derived from the mounted PC 3.4
# DUNGEON.DAT rather than from a save or a coordinate fixture.  The terminal
# C127 portrait is ordinal 5: after the source movement sequence, its C026
# portrait occupies screen x=96..127/y=68..96 (viewport-local 96..127/35..63).
# A click at its source centre must run REVIVE.C F0280 and append exactly one
# pending candidate.  Keeping this at CLI level catches presentation/input
# scaling regressions that a direct M11-state probe cannot see.
hoc_route='wait5,key:kp5,key:kp5,key:kp5,key:kp5,key:kp5,key:kp1,key:kp1,key:kp1,key:kp2,key:kp2,key:kp2,key:kp2,key:kp2,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp5,key:kp1,key:kp1,key:kp1,key:kp1,key:kp1,key:kp2,key:kp1,key:kp6,key:kp6,wait5,click:112:83,wait5'
hoc_capture_root=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$hoc_capture_root"
hoc_capture_dir=$(mktemp -d "$hoc_capture_root/dm1-hoc-c040.XXXXXX")
cleanup_hoc_capture() { find "$hoc_capture_dir" -depth -delete; }
trap cleanup_hoc_capture EXIT HUP INT TERM
hoc_output=$(FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$hoc_capture_dir" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 --script "$hoc_route" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_output" ||
   ! grep -Fq 'map=0 party=14,3,0 champions=1' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_output" ||
   ! grep -Fq 'dm1HocCandidatePartyIndex=0' <<<"$hoc_output"; then
    printf '%s\n' "$hoc_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 Hall C127 portrait click did not open C040 for ordinal 5' >&2
    exit 1
fi

# State receipts are not presentation proof. C040 is a source-sized 144x73
# overlay at screen (80,85), whose original PC3.4 raster is materially dense.
# A cleared/stale admission leaves this rectangle black even while the input
# state says candidatePanel=1, which is the reported invisible HoC panel bug.
python3 - "$hoc_capture_dir" <<'PY'
import pathlib
import struct
import sys

files = list(pathlib.Path(sys.argv[1]).glob("*.bmp"))
if len(files) != 1:
    raise SystemExit("FAIL: expected one native DM1 C040 presentation capture")
blob = files[0].read_bytes()
if len(blob) < 54 or blob[:2] != b"BM":
    raise SystemExit("FAIL: native DM1 C040 capture is not BMP")
offset = struct.unpack_from("<I", blob, 10)[0]
width, signed_height = struct.unpack_from("<Ii", blob, 18)
height = abs(signed_height)
bits = struct.unpack_from("<H", blob, 28)[0]
stride = ((width * bits + 31) // 32) * 4
if width != 320 or height != 200 or bits != 24 or offset + stride * height > len(blob):
    raise SystemExit("FAIL: invalid native DM1 C040 capture geometry")

# The BMP is BGR. The C040 rectangle is deliberately measured, not compared
# to copyrighted pixels; its source graphic has a stable dense non-black body
# and more than the black/cyan control-strip colours.
pixels = []
for y in range(85, 158):
    row = offset + y * stride
    pixels.extend(tuple(blob[row + x * 3:row + x * 3 + 3]) for x in range(80, 224))
nonblack = sum(pixel != (0, 0, 0) for pixel in pixels)
colours = len(set(pixels))
if nonblack < 5000 or colours < 10:
    raise SystemExit(
        "FAIL: authentic PC-34 C040 was not visibly presented "
        f"(nonblack={nonblack}, colours={colours})")
print(f"PASS: authentic PC-34 C040 visible nonblack={nonblack} colours={colours}")
PY

# The Mac regression was not a source-coordinate failure: a 16:9 window
# letterboxes the 4:3 original page. Exercise the same portrait through the
# physical 1920x1080 point (744,464), which maps to source (112,83) inside
# the 1600x1000 presentation rectangle. This protects the normal SDL pointer
# transform, rather than merely the boot probe's 320x200 convenience route.
hoc_scaled_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 1920 --height 1080 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route%click:112:83,wait5}click:744:464,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_scaled_output" >&2
    exit 1
}
if ! grep -Fq 'window=1920x1080' <<<"$hoc_scaled_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_scaled_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_scaled_output"; then
    printf '%s\n' "$hoc_scaled_output" >&2
    printf '%s\n' 'FAIL: scaled authentic PC-34 Hall portrait click missed C040' >&2
    exit 1
fi

# Modern changes the render target, but it must not change the original
# source-coordinate hit route.  Use the same real PC3.4 archive and physical
# 16:9 click; this caught variants where C040 worked only in Original mode.
hoc_modern_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v20 --width 1920 --height 1080 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route%click:112:83,wait5}click:744:464,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_modern_output" >&2
    exit 1
}
if ! grep -Fq 'window=1920x1080' <<<"$hoc_modern_output" ||
   ! grep -Fq 'presentationMode=1' <<<"$hoc_modern_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=1' <<<"$hoc_modern_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=5' <<<"$hoc_modern_output"; then
    printf '%s\n' "$hoc_modern_output" >&2
    printf '%s\n' 'FAIL: Modern scaled authentic PC-34 Hall portrait click missed C040' >&2
    exit 1
fi

# C040 is interactive, not merely a painted modal.  Its RESURRECT control is
# centred at (130,115) in the same source-sized PC viewport.  It must consume
# the pending C127 candidate and close the panel through the normal REVIVE.C
# path.  Keep this separate from the open assertion above so a broken click
# transform cannot be hidden by the first portrait dispatch.
hoc_confirm_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --presentation-mode v1 --width 320 --height 200 \
    --game dm1 --platform pc --data-dir "$archive" \
    --boot-probe --boot-probe-frames 720 \
    --script "${hoc_route},click:130:115,wait5" --duration 0 2>&1) || {
    printf '%s\n' "$hoc_confirm_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'map=0 party=14,3,0 champions=1' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidatePanel=0' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidateOrdinal=-1' <<<"$hoc_confirm_output" ||
   ! grep -Fq 'dm1HocCandidatePartyIndex=-1' <<<"$hoc_confirm_output"; then
    printf '%s\n' "$hoc_confirm_output" >&2
    printf '%s\n' 'FAIL: authentic PC-34 Hall C040 RESURRECT click did not consume ordinal 5' >&2
    exit 1
fi

trap - EXIT HUP INT TERM
cleanup_hoc_capture

printf '%s\n' 'PASS: authentic DM1 PC-34 archive reaches CLI, menu, and complete native input matrix'
