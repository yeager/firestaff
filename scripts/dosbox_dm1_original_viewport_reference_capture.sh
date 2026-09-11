#!/usr/bin/env bash
# Capture and normalize original DM1 PC 3.4 viewport references for the
# Firestaff deterministic in-game route.
#
# Honest scope: this script automates the reusable pieces (DOSBox Staging raw
# framebuffer screenshots, 224x136 viewport cropping, SHA-256 manifest).  It
# deliberately requires an explicit, validated original input route before
# --run, because guessing the DM1 menu/game keystrokes would create false
# reference evidence.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_STAGE="${DM1_ORIGINAL_STAGE_DIR:-${REPO}/verification-screens/dm1-dosbox-capture/DungeonMasterPC34}"
OUT_DIR="${OUT_DIR:-${REPO}/verification-screens/pass70-original-dm1-viewports}"
# Original DOS reference sessions are captured with DOSBox-X.  Do not silently
# select a different DOSBox implementation: its title/menu timing and SDL
# input delivery are part of the evidence boundary.
DOSBOX="${DOSBOX:-$(command -v dosbox-x 2>/dev/null || true)}"
DOSBOX_OUTPUT="${DM1_DOSBOX_OUTPUT:-opengl}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
NEW_FILE_TIMEOUT_MS="${NEW_FILE_TIMEOUT_MS:-2500}"
ROUTE_EVENTS="${DM1_ORIGINAL_ROUTE_EVENTS:-}"
EXPECTED_SHOTS="${DM1_ORIGINAL_EXPECTED_SHOTS:-6}"
SCREENSHOT_HOTKEY="${DM1_DOSBOX_SCREENSHOT_HOTKEY:-cmd-f5}"
case "${EXPECTED_SHOTS}" in
    single|single-row|single-transcript-row|pass625|pass626) EXPECTED_SHOTS_COUNT=1 ;;
    ''|*[!0-9]*)
        echo "ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be a positive integer or single-transcript-row" >&2
        exit 2
        ;;
    *)
        EXPECTED_SHOTS_COUNT="${EXPECTED_SHOTS}"
        if [[ "${EXPECTED_SHOTS_COUNT}" -le 0 ]]; then
            echo "ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be positive" >&2
            exit 2
        fi
        ;;
esac
SKIP_STARTUP_SELECTOR="${DM1_ROUTE_SKIP_STARTUP_SELECTOR:-0}"
ORIGINAL_PROGRAM="${DM1_ORIGINAL_PROGRAM:-DM VGA}"
CONF="${OUT_DIR}/dosbox-original-viewports.conf"
LOG="${OUT_DIR}/dosbox-original-viewports.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/original_viewport_route_keys.swift"
KEY_HELPER_XDOTOOL="${OUT_DIR}/original_viewport_route_keys_xdotool.sh"
KEY_LOG="${OUT_DIR}/original-viewpoint-route-keys.log"
SHOT_LABEL_MANIFEST="${OUT_DIR}/original_viewport_shot_labels.tsv"
RAW_MANIFEST="${OUT_DIR}/raw_manifest.tsv"
RAW_HEALTH_MANIFEST="${OUT_DIR}/raw_frame_health.json"
ROUTE_PLAN_MANIFEST="${OUT_DIR}/original_viewport_route_plan.json"
CROP_MANIFEST="${OUT_DIR}/original_viewport_224x136_manifest.tsv"
CROP_DIR="${OUT_DIR}/viewport_224x136"
SIZE_LOG="${OUT_DIR}/artifact-sizes.txt"
PASS513_SCAFFOLD="${OUT_DIR}/pass513_i34e_route_key_transcript_scaffold.json"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_original_viewport_reference_capture.sh [--prepare|--dry-run|--preflight-route|--run|--normalize-only|--print-pass94-diagnostic|--print-pass435-hoc-route]

Modes:
  --prepare                  write DOSBox config and Swift key helper only (default)
  --dry-run                  show blockers/plan, no launch
  --preflight-route          validate route shape and host injector, no DOSBox launch
  --run                      launch DOSBox, post an explicit route, capture raw frames, normalize crops
  --normalize-only           crop/hash existing image*.png raw screenshots in OUT_DIR
  --print-pass94-diagnostic  print the pass94 entrance-click diagnostic command and audit expectations
  --print-pass435-hoc-route  print the current pass435 Hall-of-Champions route candidate

Required for --run:
  DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot:party_hud right wait:300 shot up wait:300 shot:spell_panel ...'

Supported route tokens:
  shot, shot:<label>, wait:<ms>, click:<x>,<y>, press:<x>,<y>, release, enter, esc, space, up, down,
  left, right, one, two, three, four, five, six, f1-f4, kp0-kp9,
  kpenter, ctrl-s, a-z, 0-9, rclick:<x>,<y>

Labeled shot tokens:
  shot:<label> is equivalent to shot for capture input, and records the label
  in original_viewport_shot_labels.tsv during normalization. Labels are
  lowercase route semantics such as shot:party_hud, shot:spell_panel, and
  shot:inventory_panel. They are evidence metadata only; they do not claim
  pixel parity or validate that the original runtime reached that state.

Outputs:
  raw screenshots: ${OUT_DIR}/image*.png (raw 320x200; exact DOSBox 640x400 2x and 720x400 aspect-corrected captures are normalized to 320x200)
  crops:           ${CROP_DIR}/*.ppm and *.png
  raw health:      ${RAW_HEALTH_MANIFEST}

Optional environment:
  DM1_ORIGINAL_STAGE_DIR=/path/to/DM1-PC34-tree-with-DM.EXE
                    override the default staged tree path
                    Defaults to verification-screens/dm1-dosbox-capture/DungeonMasterPC34
                    under the repository. No machine-specific hidden stage is selected.
  DM1_ROUTE_SKIP_STARTUP_SELECTOR=1
                    skip legacy graphics/sound/input selector keystrokes when
                    the DOSBox config launches 'DM VGA' directly
  DM1_ORIGINAL_PROGRAM='DM -vv -sn -pm'
                    override autoexec launch command; recommended for bypassing
                    the original selector and entering VGA/no-sound/mouse mode.
                    Use -pm for routes containing source-space click tokens.
                    directly. 'DM VGA' remains the default for legacy runs.
  DM1_ORIGINAL_EXPECTED_SHOTS=6
                    required raw screenshot count. Legacy overlay routes use 6.
                    Use 1, or 'single-transcript-row', for the pass625/pass626
                    C002 turn-redraw transcript row so black-frame/rawshot
                    health can be checked before transcript writing.
                    The capture script must contain exactly 6 shot or shot:<label> tokens
                    for the legacy pass70 overlay route; this matches the
                    raw_manifest health gate and the pass84 classifier
                    nonDuplicate expectation.
  DM1_DOSBOX_SCREENSHOT_HOTKEY=cmd-f5
                    macOS Swift route injector screenshot accelerator. Use
                    ctrl-f5 for CLI DOSBox Staging builds whose mapper does not
                    respond to Cmd+F5. Linux/xdotool uses Ctrl+F5.
  DM1_DOSBOX_CAPTURE_BACKEND=host
                    Verification-only fallback for an emulator whose own
                    screenshot writer is unstable. Captures the X11 DOSBox
                    window with scrot, crops its 4:3 content rectangle, and
                    reduces it with nearest-neighbour to original 320x200.
  DM1_DOSBOX_OUTPUT=surface
                    select DOSBox-X's software presentation backend for an
                    original capture when an OpenGL/X11 resize is unstable.
                    The default remains opengl; this affects only the
                    external capture harness, never Firestaff runtime.
  DM1_DOSBOX_INPUT_MODE=global
                    Linux/X11 verification fallback for DOSBox-X builds that
                    stop accepting xdotool --window events after the Entrance
                    handoff. Emits focused root-device XTest input instead;
                    it affects only this external original-capture harness.
                    Keep the default window mode for held C071 Eye/C070 Mouth
                    captures: the original panel redraw is tied to the SDL
                    window's pressed-button state on the verified PC 3.4 path.
  DM1_DOSBOX_MOUSE_HOLD_MS=60
                    duration of each injected mouse press. The dungeon loop
                    samples button state asynchronously, so an XTest click
                    with an immediate release is intentionally not used.
  click:<x>,<y>    posts one serialized left-click in original 320x200 game
                    coordinates. Use waits around clicks; ReDMCSB BUG0_73 shows
                    mixed mouse/keyboard commands can be lost when packed tightly.
  rclick:<x>,<y>   posts one serialized right-click in original 320x200 game
                    coordinates. This is needed for source-owned inventory close/
                    toggle routes such as C011/C083, not a parity claim by itself.
  press:<x>,<y>    holds the left button at an original-space coordinate until
                    a later release token. Use this to capture transient source
                    states such as C071 Eye and C070 Mouth while they are held.
  release          releases a preceding press token after any requested shot.
  manifest:        ${CROP_MANIFEST}
  shot labels:     ${SHOT_LABEL_MANIFEST}
  pass513 scaffold:${PASS513_SCAFFOLD}

Linux/N2 note:
  On Linux, run --run under an X server, for example:
    DOSBOX=/usr/bin/dosbox-x xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run
  The script uses xdotool as the route injector when Swift/CGEvent is absent.

Honesty note:
  The route string must be validated against the original runtime state that
  corresponds to Firestaff's run_capture_screenshots.sh sequence.  This script
  will not invent that route. Use DM1_ORIGINAL_PROGRAM='DM -vv -sn -pm' to
  bypass the text selector; audit raw captures before accepting references.
EOF
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --preflight-route) mode="preflight-route"; shift ;;
        --run) mode="run"; shift ;;
        --normalize-only) mode="normalize-only"; shift ;;
        --print-pass94-diagnostic) mode="print-pass94-diagnostic"; shift ;;
        --print-pass435-hoc-route) mode="print-pass435-hoc-route"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

print_pass94_diagnostic() {
    cat <<EOF
# Pass 94 original entrance-click diagnostic (manual/original-route unblock only).
# This is not parity evidence. It should answer whether click:260,50 leaves the entrance menu.
# The PC 3.4 title-to-entrance fade needs at least 2.5 seconds on the
# authenticated DOSBox-X route; a shorter delay sends the click into black.

OUT_DIR=\$PWD/verification-screens/pass94-hall-map-enter-diagnostic \\
DM1_ORIGINAL_STAGE_DIR=\$PWD/verification-screens/dm1-dosbox-capture/DungeonMasterPC34 \\
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pm' \\
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\
WAIT_BEFORE_INPUT_MS=5000 \\
NEW_FILE_TIMEOUT_MS=6000 \\
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:2500 shot:pre_enter_menu click:260,50 wait:1800 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' \\
scripts/dosbox_dm1_original_viewport_reference_capture.sh --run

# Expected route labels in original_viewport_shot_labels.tsv:
#   01 title
#   02 pre_enter_menu
#   03 after_enter_click
#   04 forward_1
#   05 forward_2
#   06 left_turn_probe

# Expected classifier outcome if the entrance click worked:
#   title_or_menu, entrance_menu, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay, dungeon_gameplay
python3 tools/pass80_original_frame_classifier.py \\
  verification-screens/pass94-hall-map-enter-diagnostic \\
  --expected pass94-diagnostic \\
  --fail-on-duplicates

# Failure signal to preserve as a blocker, not promote:
#   after_enter_click == entrance_menu means click:260,50 did not leave the menu.
#   wall_closeup/title_or_menu/non_graphics_blocker in shots 04-06 means the movement probe is not usable gameplay evidence.
EOF
}

print_pass435_hoc_route() {
    cat <<EOF
# Pass435 DM1 original Hall-of-Champions C407 diagnostic.
# This is a reproducible source-start checkpoint, not a promotion route.
# The old candidate clicked a supposed C127 portrait while the original was
# still in Entrance. Authentic 2026-09-08 DOSBox-X captures prove that C407
# becomes active only after a six-second door wait and leads to the no-party
# Hall start. Derive the later start-position-to-C127 movement path from the
# original map before attempting resurrection or inventory capture.

OUT_DIR=\$PWD/verification-screens/pass376-original-route \\
DM1_ORIGINAL_STAGE_DIR=\$PWD/verification-screens/dm1-dosbox-capture/DungeonMasterPC34 \\
DOSBOX=/usr/bin/dosbox-x \\
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pm' \\
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\
WAIT_BEFORE_INPUT_MS=3000 \\
NEW_FILE_TIMEOUT_MS=6000 \\
DM1_ORIGINAL_EXPECTED_SHOTS=2 \\
DM1_DOSBOX_CAPTURE_BACKEND=host \\
DM1_DOSBOX_INPUT_MODE=global \\
DM1_ORIGINAL_ROUTE_EVENTS='wait:9000 enter wait:6000 shot:entrance_stable click:260,50 wait:3000 shot:hall_start' \\
xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run

python3 tools/pass80_original_frame_classifier.py \\
  verification-screens/pass376-original-route \\
  --expected pass435-c407 \\
  --fail-on-duplicates

python3 tools/pass86_original_viewport_crop_manifest.py \\
  verification-screens/pass376-original-route \\
  --out-dir verification-screens/pass376-original-dm1-viewports
python3 tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py

# Expected diagnostic classes: entrance_menu, dungeon_gameplay.
# This only establishes the original C407 handoff. It must not be used as
# portrait, party, inventory, overlay, or pixel-parity evidence.
EOF
}

need_stage() {
    if [[ ! -f "${SRC_STAGE}/DM.EXE" ]]; then
        echo "ERROR: staged DM1 PC 3.4 tree missing: ${SRC_STAGE}" >&2
        echo "Run scripts/dosbox_dm1_capture.sh first, or set DM1_ORIGINAL_STAGE_DIR to an existing DM1 PC 3.4 tree. If staging fails, the missing input is original-games/Game,Dungeon_Master,DOS,Software.7z." >&2
        exit 3
    fi
}

need_image_tool() {
    # Prefer Pillow for deterministic crop normalization. Some ImageMagick 6
    # convert builds reject extensionless PPM outputs used below, while the
    # Pillow path writes both PPM and PNG explicitly.
    if python3 - <<PY >/dev/null 2>&1
from PIL import Image
PY
    then
        echo pillow
    elif command -v magick >/dev/null 2>&1; then
        echo magick
    elif command -v convert >/dev/null 2>&1; then
        echo convert
    else
        echo "ERROR: ImageMagick (magick/convert) or Python Pillow is required for PNG->PPM viewport crop normalization." >&2
        exit 4
    fi
}

validate_route_shape() {
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        return 0
    fi
    python3 - "${ROUTE_EVENTS}" "${EXPECTED_SHOTS}" <<'PY'
import re, sys
route = sys.argv[1].split()
expected_raw = sys.argv[2].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
if expected <= 0:
    raise SystemExit("ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be positive")
allowed = set("shot capture screenshot enter return esc escape space up down left right one two three four five six zero".split())
allowed |= set("abcdefghijklmnopqrstuvwxyz") | set("0123456789") | {f"kp{i}" for i in range(10)} | {f"f{i}" for i in range(1, 5)} | {"kpenter", "ctrl-s"}
diagnostic_only = {"title", "pre_enter_menu", "after_enter_click", "forward_1", "forward_2", "left_turn_probe"}
shots = 0
labeled_shots = 0
labels = []
for token in route:
    low = token.lower()
    if low in {"shot", "capture", "screenshot"}:
        shots += 1
        labels.append("")
        continue
    if low.startswith("shot:"):
        label = low.split(":", 1)[1]
        if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", label):
            raise SystemExit(f"ERROR: invalid shot label: {token}")
        shots += 1
        labeled_shots += 1
        labels.append(label)
        continue
    if low.startswith("wait:"):
        if not re.fullmatch(r"wait:[0-9]+", low):
            raise SystemExit(f"ERROR: invalid wait token: {token}")
        continue
    if low.startswith("click:") or low.startswith("rclick:") or low.startswith("press:"):
        m = re.fullmatch(r"(?:r?click|press):([0-9]{1,3}),([0-9]{1,3})", low)
        if not m:
            raise SystemExit(f"ERROR: invalid click token: {token}")
        x, y = map(int, m.groups())
        if not (0 <= x < 320 and 0 <= y < 200):
            raise SystemExit(f"ERROR: click token outside original 320x200 frame: {token}")
        continue
    if low == "release":
        continue
    if low not in allowed:
        raise SystemExit(f"ERROR: unknown route token: {token}")
if shots != expected:
    raise SystemExit(f"ERROR: DM1_ORIGINAL_ROUTE_EVENTS must contain exactly {expected} shot or shot:<label> tokens, found {shots}")
print(f"[pass-70] route shape OK: {len(route)} tokens, {shots} shots, {labeled_shots} labeled")
if labeled_shots:
    pretty = ", ".join(f"{idx + 1:02d}:{label or '(unlabeled)'}" for idx, label in enumerate(labels))
    print(f"[pass-70] shot label plan: {pretty}")
if expected == 1:
    # DOSBox-X can exit immediately after writing a raw screenshot on some
    # X11 hosts.  A one-frame route is therefore also a useful, honest way to
    # capture a late original state: replay the complete source route and put
    # the only screenshot last.  The label remains capture-channel metadata;
    # no arbitrary one-frame result can satisfy a paired transcript or pixel
    # parity gate.  Keep the existing C002 row warning when that special label
    # is used, but do not reject other explicitly named diagnostic states.
    if not labels or not labels[0]:
        raise SystemExit("ERROR: one-frame capture routes must use an explicit shot:<label>")
    if labels[0] == "02_turn_right_west_1_3":
        if not any(token.lower() in {"right", "kp6"} for token in route):
            raise SystemExit("ERROR: single transcript-row capture must include right or kp6 before the shot")
        print("[pass-70] single transcript-row route locked to pass625/pass626 C002 turn-redraw target")
    else:
        print("[pass-70] single named route is capture-channel evidence only; it cannot satisfy transcript or overlay readiness")
diagnostic_hits = [label for label in labels if label in diagnostic_only]
if diagnostic_hits:
    print("[pass-70] diagnostic-only labels present: " + ", ".join(diagnostic_hits))
    print("[pass-70] note: diagnostic labels are for manual/pass94 routing only; pass84 overlay readiness requires party_hud, blank, blank, spell_panel, blank, inventory_panel")
PY
}

write_route_plan_manifest() {
    local injector="${1:-unknown}"
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        return 0
    fi
    mkdir -p "${OUT_DIR}"
    python3 - "${ROUTE_EVENTS}" "${EXPECTED_SHOTS}" "${injector}" "${SCREENSHOT_HOTKEY}" "${ROUTE_PLAN_MANIFEST}" <<'PY'
from __future__ import annotations
import json
import re
import sys
from pathlib import Path

route = sys.argv[1].split()
expected_raw = sys.argv[2].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
injector = sys.argv[3]
screenshot_hotkey = sys.argv[4]
out = Path(sys.argv[5])

rows = []
total_wait_ms = 0
shot_labels = []
for idx, token in enumerate(route, 1):
    low = token.lower()
    kind = "key"
    detail = low
    if low in {"shot", "capture", "screenshot"}:
        kind = "shot"
        detail = ""
        shot_labels.append("")
    elif low.startswith("shot:"):
        kind = "shot"
        detail = low.split(":", 1)[1]
        shot_labels.append(detail)
    elif low.startswith("wait:"):
        kind = "wait"
        detail = low.split(":", 1)[1]
        total_wait_ms += int(detail)
    elif low.startswith("click:") or low.startswith("rclick:") or low.startswith("press:"):
        kind = "click"
    rows.append({"index": idx, "token": token, "kind": kind, "detail": detail})

payload = {
    "schema": "dm1_original_route_plan.v1",
    "honesty": "Route injector plan only. This does not launch DOSBox or claim runtime semantics.",
    "injector": injector,
    "screenshotHotkey": screenshot_hotkey,
    "tokenCount": len(route),
    "expectedShots": expected,
    "shotCount": len(shot_labels),
    "shotLabels": shot_labels,
    "totalWaitMs": total_wait_ms,
    "tokens": rows,
    "pass": len(shot_labels) == expected and all(
        (label == "" or re.fullmatch(r"[a-z0-9][a-z0-9_-]*", label))
        for label in shot_labels
    ),
}
out.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
print(f"[pass-70] wrote route plan: {out}")
PY
}

select_route_injector() {
    local uname_s
    local swift_path
    uname_s="$(uname -s 2>/dev/null || true)"
    swift_path="$(command -v swift 2>/dev/null || true)"
    if [[ "${uname_s}" == "Darwin" && -n "${swift_path}" && -x "${swift_path}" ]]; then
        echo "swift"
        return 0
    fi
    if command -v xdotool >/dev/null 2>&1; then
        echo "xdotool"
        return 0
    fi
    return 1
}

focus_dosbox_for_route() {
    local pid="${1:-}"
    if [[ "$(uname -s 2>/dev/null || true)" != "Darwin" ]]; then
        return 0
    fi
    if ! command -v osascript >/dev/null 2>&1; then
        return 0
    fi

    # Best-effort focus only. The evidence gate remains the raw screenshot
    # count/health check below, not the fact that macOS accepted activation.
    osascript -e 'tell application "DOSBox Staging" to activate' >/dev/null 2>&1 || true
    if [[ -n "${pid}" ]]; then
        osascript -e 'tell application "System Events" to set frontmost of first process whose unix id is '"${pid}"' to true' >/dev/null 2>&1 || true
    fi
}

preflight_route() {
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        echo "ERROR: DM1_ORIGINAL_ROUTE_EVENTS is required for --preflight-route" >&2
        return 5
    fi
    write_helpers
    validate_route_shape

    local injector
    if ! injector="$(select_route_injector)"; then
        echo "ERROR: no supported route injector found; install Swift on macOS or xdotool on X11/Linux" >&2
        return 6
    fi
    if [[ "${injector}" == "xdotool" && -z "${DISPLAY:-}" ]]; then
        echo "ERROR: xdotool route injector selected but DISPLAY is not set; run the capture under an X server such as xvfb-run -a" >&2
        return 6
    fi

    echo "[pass-70] selected route injector: ${injector}"
    write_route_plan_manifest "${injector}"
    echo "[pass-70] route preflight OK"
}

write_helpers() {
    mkdir -p "${OUT_DIR}" "${CROP_DIR}"
    cat > "${CONF}" <<EOF
[sdl]
fullscreen=false
output=${DOSBOX_OUTPUT}

[dosbox]
# Capture runs are non-interactive and are terminated by this harness.  This
# is a [dosbox] setting in current DOSBox-X, not an [sdl] setting.
quit warning=false
machine=svga_paradise
memsize=4
captures=${OUT_DIR}

[cpu]
core=normal
cputype=386
# DOSBox-X consumes cycles, not the legacy cpu_cycles spelling.  Without this
# it falls back to automatic maximum speed and invalidates timing captures.
cycles=fixed 3000
cycleup=500
cycledown=500

[render]
aspect=false
integer_scaling=false

[mixer]
nosound=true

[speaker]
pcspeaker=false
tandy=off

[capture]
capture_dir=${OUT_DIR}
default_image_capture_formats=raw

[autoexec]
mount c "${SRC_STAGE}"
c:
${ORIGINAL_PROGRAM}
EOF

    cat > "${KEY_HELPER}" <<'SWIFT'
import Foundation
import CoreGraphics
import ApplicationServices

if CommandLine.arguments.count != 4 {
    fputs("usage: original_viewport_route_keys.swift PID ROUTE_EVENTS SKIP_STARTUP_SELECTOR\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}
let route = CommandLine.arguments[2].split(separator: " ").map(String.init)
let skipStartupSelector = CommandLine.arguments[3] == "1"
let source = CGEventSource(stateID: .hidSystemState)
let screenshotHotkey = ProcessInfo.processInfo.environment["DM1_DOSBOX_SCREENSHOT_HOTKEY"]?.lowercased() ?? "cmd-f5"

let keycodes: [String: CGKeyCode] = [
    "a": 0, "s": 1, "d": 2, "f": 3, "h": 4, "g": 5, "z": 6, "x": 7, "c": 8, "v": 9,
    "b": 11, "q": 12, "w": 13, "e": 14, "r": 15, "y": 16, "t": 17,
    "one": 18, "1": 18, "two": 19, "2": 19, "three": 20, "3": 20, "four": 21, "4": 21,
    "six": 22, "6": 22, "five": 23, "5": 23, "zero": 29, "0": 29,
    "o": 31, "u": 32, "i": 34, "p": 35, "l": 37, "j": 38, "k": 40,
    "n": 45, "m": 46,
    "enter": 36, "return": 36, "space": 49, "esc": 53, "escape": 53,
    "f1": 122, "f2": 120, "f3": 99, "f4": 118,
    "left": 123, "right": 124, "down": 125, "up": 126,
    "kp1": 83, "kp2": 84, "kp3": 85, "kp4": 86, "kp5": 87, "kp6": 88,
    "kp7": 89, "kp8": 91, "kp9": 92, "kp0": 82, "kpenter": 76
]

func post(_ key: CGKeyCode, _ down: Bool, flags: CGEventFlags = []) {
    guard let event = CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: down) else { return }
    event.flags = flags
    event.postToPid(pid)
}
func tap(_ key: CGKeyCode, _ delayUs: useconds_t = 120_000) {
    post(key, true)
    usleep(20_000)
    post(key, false)
    usleep(delayUs)
}
func ctrlS() {
    post(59, true, flags: .maskControl) // Control
    usleep(20_000)
    post(1, true, flags: .maskControl)  // S
    usleep(20_000)
    post(1, false, flags: .maskControl)
    usleep(20_000)
    post(59, false)
    usleep(120_000)
}
func cmdF5() {
    let modifierKey: CGKeyCode
    let modifierFlags: CGEventFlags
    if screenshotHotkey == "ctrl-f5" || screenshotHotkey == "control-f5" {
        modifierKey = 59                    // Control
        modifierFlags = .maskControl
    } else {
        modifierKey = 55                    // Command
        modifierFlags = .maskCommand
    }
    print("screenshot-hotkey \(screenshotHotkey)")
    post(modifierKey, true, flags: modifierFlags)
    usleep(20_000)
    post(96, true, flags: modifierFlags)   // F5
    usleep(20_000)
    post(96, false, flags: modifierFlags)
    usleep(20_000)
    post(modifierKey, false)
    usleep(180_000)
}

func dosboxWindowBounds() -> CGRect? {
    let opts: CGWindowListOption = [.optionOnScreenOnly, .excludeDesktopElements]
    guard let windows = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else { return nil }
    for window in windows {
        guard let ownerPid = window[kCGWindowOwnerPID as String] as? pid_t, ownerPid == pid else { continue }
        guard let boundsDict = window[kCGWindowBounds as String] as? [String: Any] else { continue }
        guard
            let x = boundsDict["X"] as? CGFloat,
            let y = boundsDict["Y"] as? CGFloat,
            let w = boundsDict["Width"] as? CGFloat,
            let h = boundsDict["Height"] as? CGFloat,
            w > 0, h > 0
        else { continue }
        return CGRect(x: x, y: y, width: w, height: h)
    }
    return nil
}

var heldMousePoint: CGPoint? = nil
func clickOriginalFrame(x: Int, y: Int, button: String = "left", releaseAfter: Bool = true) {
    guard let bounds = dosboxWindowBounds() else {
        fputs("could not find DOSBox window bounds for click:\(x),\(y)\n", stderr)
        exit(3)
    }
    // Map DM1's raw 320x200 coordinate space into the visible DOSBox content.
    // DOSBox Staging may letterbox/pillarbox depending on the current mode; use
    // a centered aspect-fit rectangle so clicks stay anchored to original pixels.
    let contentAspect = 320.0 / 200.0
    var contentW = Double(bounds.width)
    var contentH = contentW / contentAspect
    if contentH > Double(bounds.height) {
        contentH = Double(bounds.height)
        contentW = contentH * contentAspect
    }
    let left = Double(bounds.minX) + (Double(bounds.width) - contentW) / 2.0
    let top = Double(bounds.minY) + (Double(bounds.height) - contentH) / 2.0
    let px = left + ((Double(x) + 0.5) / 320.0) * contentW
    let py = top + ((Double(y) + 0.5) / 200.0) * contentH
    let point = CGPoint(x: px, y: py)
    let cgButton: CGMouseButton = (button == "right") ? .right : .left
    let downType: CGEventType = (button == "right") ? .rightMouseDown : .leftMouseDown
    let upType: CGEventType = (button == "right") ? .rightMouseUp : .leftMouseUp
    guard let down = CGEvent(mouseEventSource: source, mouseType: downType, mouseCursorPosition: point, mouseButton: cgButton) else { return }
    down.postToPid(pid)
    if !releaseAfter {
        heldMousePoint = point
        print("left-press-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height))")
        return
    }
    usleep(45_000)
    guard let up = CGEvent(mouseEventSource: source, mouseType: upType, mouseCursorPosition: point, mouseButton: cgButton) else { return }
    up.postToPid(pid)
    print("\(button)-click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height))")
    usleep(180_000)
}

func releaseOriginalFrameButton() {
    guard let point = heldMousePoint,
          let up = CGEvent(mouseEventSource: source, mouseType: .leftMouseUp, mouseCursorPosition: point, mouseButton: .left) else { return }
    up.postToPid(pid)
    heldMousePoint = nil
    print("left-release")
    usleep(180_000)
}

// Original PC 3.4 startup selector: graphics=1, sound=1, input=1.
// The generated DOSBox config launches 'DM VGA' directly, which bypasses that
// selector.  In that state, posting the legacy selector keys would hit the
// title/game screens and shift the whole capture route.
if !skipStartupSelector {
    for _ in 0..<3 {
        tap(18) // '1'
        tap(36) // Return
    }
}

for token in route {
    let lowerToken = token.lowercased()
    let started = Date().timeIntervalSince1970
    print("route-token-start \(token) \(started)")
    if lowerToken == "shot" || lowerToken == "capture" || lowerToken == "screenshot" || lowerToken.hasPrefix("shot:") {
        cmdF5()
    } else if lowerToken.hasPrefix("wait:") {
        let msText = String(lowerToken.dropFirst("wait:".count))
        guard let ms = UInt32(msText) else {
            fputs("invalid wait token: \(token)\n", stderr)
            exit(2)
        }
        usleep(ms * 1000)
    } else if lowerToken.hasPrefix("click:") || lowerToken.hasPrefix("rclick:") || lowerToken.hasPrefix("press:") {
        let isRightClick = lowerToken.hasPrefix("rclick:")
        let isPress = lowerToken.hasPrefix("press:")
        let prefix = isRightClick ? "rclick:" : (isPress ? "press:" : "click:")
        let coords = lowerToken.dropFirst(prefix.count).split(separator: ",")
        guard coords.count == 2, let x = Int(coords[0]), let y = Int(coords[1]), x >= 0, x < 320, y >= 0, y < 200 else {
            fputs("invalid click token: \(token)\n", stderr)
            exit(2)
        }
        clickOriginalFrame(x: x, y: y, button: isRightClick ? "right" : "left", releaseAfter: !isPress)
    } else if lowerToken == "release" {
        releaseOriginalFrameButton()
    } else if lowerToken == "ctrl-s" {
        ctrlS()
    } else if let key = keycodes[lowerToken] {
        tap(key)
    } else {
        fputs("unknown route token: \(token)\n", stderr)
        exit(2)
    }
    let finished = Date().timeIntervalSince1970
    print("route-token-done \(token) \(finished)")
}
print("route-complete \(Date().timeIntervalSince1970)")
SWIFT
    cat > "${KEY_HELPER_XDOTOOL}" <<'SH'
#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "usage: original_viewport_route_keys_xdotool.sh PID ROUTE_EVENTS SKIP_STARTUP_SELECTOR" >&2
    exit 2
fi

pid="$1"
route_events="$2"
skip_startup_selector="$3"
screenshot_hotkey="${DM1_DOSBOX_SCREENSHOT_HOTKEY:-ctrl+F5}"
capture_backend="${DM1_DOSBOX_CAPTURE_BACKEND:-emulator}"
capture_dir="${DM1_DOSBOX_CAPTURE_OUT_DIR:-.}"
input_mode="${DM1_DOSBOX_INPUT_MODE:-window}"
mouse_hold_ms="${DM1_DOSBOX_MOUSE_HOLD_MS:-60}"
host_capture_index=0

if [[ -z "${DISPLAY:-}" ]]; then
    echo "ERROR: DISPLAY is not set; run DOSBox under an X server, e.g. xvfb-run -a ... --run" >&2
    exit 6
fi

find_dosbox_window() {
    # Do not use xdotool --sync here: DOSBox-X may destroy and recreate its
    # SDL window while control transfers from Entrance to the dungeon loop.
    # --sync then waits forever for an already-gone window and silently turns
    # an authentic route into a truncated capture.  A bounded retry keeps the
    # result diagnostic: an absent replacement window is a hard route error.
    local candidate attempt
    for attempt in $(seq 1 50); do
        candidate="$(xdotool search --pid "$pid" 2>/dev/null | head -n 1 || true)"
        if [[ -n "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
        sleep 0.10
    done
    return 1
}

window="$(find_dosbox_window || true)"
if [[ -z "$window" ]]; then
    echo "ERROR: could not find DOSBox X window for pid $pid" >&2
    exit 3
fi
xdotool windowactivate --sync "$window" >/dev/null 2>&1 || true
xdotool windowfocus --sync "$window" >/dev/null 2>&1 || true

# DOSBox-X recreates its SDL window while transferring from Entrance into the
# game loop on some Linux/X11 builds.  Route keys in ``global`` mode continue
# to reach the focused SDL application, but host screenshots and mouse
# geometry must never retain the now-invalid pre-transfer window ID.
refresh_window() {
    if ! xdotool getwindowgeometry --shell "$window" >/dev/null 2>&1; then
        window="$(find_dosbox_window || true)"
        if [[ -z "$window" ]]; then
            echo "ERROR: DOSBox X11 window disappeared and could not be reacquired for pid $pid" >&2
            exit 3
        fi
        echo "dosbox-window-reacquired $window"
    fi
    # A global XTest event follows the X input focus, not ``window``.  Long
    # original routes include host mouse motion and screenshot shortcuts, so
    # explicitly restore focus before every such action rather than assuming
    # the initial activation survived the previous token.
    xdotool windowactivate --sync "$window" >/dev/null 2>&1 || true
    xdotool windowfocus --sync "$window" >/dev/null 2>&1 || true
}

case "$input_mode" in
    window|global) ;;
    *)
        echo "ERROR: DM1_DOSBOX_INPUT_MODE must be 'window' or 'global', got '$input_mode'" >&2
        exit 2
        ;;
esac
if [[ ! "$mouse_hold_ms" =~ ^[0-9]+$ ]] || [[ "$mouse_hold_ms" -lt 1 ]]; then
    echo "ERROR: DM1_DOSBOX_MOUSE_HOLD_MS must be a positive millisecond count, got '$mouse_hold_ms'" >&2
    exit 2
fi

tap_key() {
    local key="$1"
    # DOSBox-X accepts XTest events addressed at its window for title/Entrance,
    # but some SDL input paths stop consuming those targeted events once C407
    # transfers into the game loop.  ``global`` deliberately emits the same
    # focused X11 event at the root input device, like physical keyboard input.
    # It is capture tooling only; Firestaff never invokes xdotool at runtime.
    if [[ "$input_mode" == "global" ]]; then
        refresh_window
        xdotool key "$key"
    else
        xdotool key --window "$window" "$key"
    fi
    sleep 0.12
}

shot() {
    refresh_window
    if [[ "$capture_backend" == "host" ]]; then
        local host_raw host_out
        host_capture_index=$((host_capture_index + 1))
        host_raw="${capture_dir}/host-window-${host_capture_index}.png"
        host_out="${capture_dir}/host-${host_capture_index}.png"
        scrot --window "$window" --overwrite --silent "$host_raw" || true
        # Some SDL/Xvfb combinations expose a live window to xdotool while
        # scrot's per-window path still returns an all-black pixmap.  That is
        # not original evidence.  Detect that narrow host-capture failure and
        # retry the same X11 window through ImageMagick's XGetImage backend.
        # The later raw-frame health gate remains authoritative; this fallback
        # merely avoids turning a known capture-backend defect into a false
        # negative route result.
        if ! python3 - "$host_raw" <<'PY'
from pathlib import Path
from PIL import Image
import sys

path = Path(sys.argv[1])
try:
    im = Image.open(path).convert("RGB")
except Exception:
    raise SystemExit(1)
colors = im.getcolors(maxcolors=257)
if not colors or len(colors) < 2:
    raise SystemExit(1)
if all(pixel == (0, 0, 0) for _, pixel in colors):
    raise SystemExit(1)
# A stale SDL surface can contain only the DOSBox-X menu/title strip at the
# top.  Inspect the lower 80 percent where the 4:3 game canvas belongs; a
# genuine HoC frame has substantial non-black content there, whereas that
# strip-only failure has none.
canvas = im.crop((0, im.height // 5, im.width, im.height))
if not any(pixel != (0, 0, 0) for pixel in canvas.get_flattened_data()):
    raise SystemExit(1)
PY
        then
            if command -v import >/dev/null 2>&1; then
                echo "host-capture-scrot-blank-retrying-import window=$window" >&2
                import -window "$window" "$host_raw"
            else
                echo "ERROR: scrot returned a blank host capture and ImageMagick import is unavailable" >&2
                exit 9
            fi
        fi
        python3 - "$host_raw" "$host_out" <<'PY'
from pathlib import Path
from PIL import Image
import sys
src, dst = map(Path, sys.argv[1:])
im = Image.open(src).convert("RGB")
width, height = im.size
content_width = width
content_height = round(content_width * 200 / 320)
if content_height > height:
    content_height = height
    content_width = round(content_height * 320 / 200)
left = (width - content_width) // 2
# scrot's X11 window capture includes DOSBox-X's menu chrome above (not below)
# the emulated canvas.  The actual DOS canvas is the trailing 4:3 rectangle;
# centering would leak that host UI into a purported original frame.
top = height - content_height
resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
im.crop((left, top, left + content_width, top + content_height)).resize(
    (320, 200), resample).save(dst)
if __import__("os").environ.get("DM1_DOSBOX_KEEP_HOST_CAPTURE") != "1":
    src.unlink()
PY
        return
    fi
    # DOSBox 0.74 on Linux uses Ctrl+F5 for screenshots. DOSBox Staging accepts
    # the same accelerator, while DOSBox-X uses its F12+P host-key sequence.
    # The caller/backend selection is injected through the environment.
    # Screenshot capture is a DOSBox-X host binding.  Once Entrance has
    # transferred ownership to the SDL game loop, a targeted XTest event can
    # lose its window beneath the binding itself.  Match the physical-style
    # global route-input mode for this host binding as well.
    if [[ "$input_mode" == "global" ]]; then
        refresh_window
        xdotool key "$screenshot_hotkey"
    else
        xdotool key --window "$window" "$screenshot_hotkey"
    fi
    sleep 0.18
}

click_original_frame() {
    local x="$1" y="$2" button="${3:-1}" release_after="${4:-1}"
    local geom gx gy gw gh px py
    refresh_window
    geom="$(xdotool getwindowgeometry --shell "$window")"
    eval "$geom"
    gx="$X"; gy="$Y"; gw="$WIDTH"; gh="$HEIGHT"
    read -r px py < <(python3 - "$gw" "$gh" "$x" "$y" <<'PY'
import sys
gw, gh, x, y = map(float, sys.argv[1:])
content_aspect = 320.0 / 200.0
content_w = gw
content_h = content_w / content_aspect
if content_h > gh:
    content_h = gh
    content_w = content_h * content_aspect
left = (gw - content_w) / 2.0
# DOSBox-X places its optional menu chrome above the emulated canvas.  The
# usable 4:3 rectangle therefore ends at the window's bottom edge; centering
# it shifts every original-space click upward when that chrome is present.
top = gh - content_h
px = left + ((x + 0.5) / 320.0) * content_w
py = top + ((y + 0.5) / 200.0) * content_h
print(int(round(px)), int(round(py)))
PY
)
    if [[ "$input_mode" == "global" ]]; then
        # Physical-style X11 input must use desktop coordinates.  This is the
        # counterpart to the root-level keyboard path above.
        xdotool mousemove "$((gx + px))" "$((gy + py))"
    else
        # xdotool --window coordinates are relative to the target window.  Do
        # not add the absolute X/Y origin here; doing so can click outside the
        # DOSBox client under Xvfb when the window is offset from 0,0.
        xdotool mousemove --window "$window" "$px" "$py"
    fi
    # ``xdotool click`` can collapse press/release into one host timeslice.
    # The Entrance menu observes it, but the timed dungeon loop can miss it.
    # Keep the button down across several original frames; this mirrors the
    # existing CGEvent injector's explicit 45ms interval.
    xdotool mousedown "$button"
    if [[ "$release_after" != "1" ]]; then
        echo "left-press-mapped ${x},${y} -> ${input_mode} ${px},${py} window=${gw}x${gh} origin=${gx},${gy}"
        return
    fi
    sleep "$(python3 - "$mouse_hold_ms" <<'PY'
import sys
print(int(sys.argv[1]) / 1000.0)
PY
)"
    xdotool mouseup "$button"
    local button_name=left
    if [[ "$button" == "3" ]]; then button_name=right; fi
    echo "${button_name}-click-mapped ${x},${y} -> ${input_mode} ${px},${py} window=${gw}x${gh} origin=${gx},${gy}"
    sleep 0.18
}

release_original_frame_button() {
    # C071/C070 are transient while the source sees the button down.  The
    # route deliberately takes any screenshot before this release token.
    xdotool mouseup 1
    echo "left-release"
    sleep 0.18
}

key_for_token() {
    case "$1" in
        enter|return) echo Return ;;
        esc|escape) echo Escape ;;
        space) echo space ;;
        up) echo Up ;;
        down) echo Down ;;
        left) echo Left ;;
        right) echo Right ;;
        one|1) echo 1 ;;
        two|2) echo 2 ;;
        three|3) echo 3 ;;
        four|4) echo 4 ;;
        five|5) echo 5 ;;
        six|6) echo 6 ;;
        zero|0) echo 0 ;;
        f1) echo F1 ;;
        f2) echo F2 ;;
        f3) echo F3 ;;
        f4) echo F4 ;;
        kp0) echo KP_Insert ;;
        kp1) echo KP_End ;;
        kp2) echo KP_Down ;;
        kp3) echo KP_Next ;;
        kp4) echo KP_Left ;;
        kp5) echo KP_Begin ;;
        kp6) echo KP_Right ;;
        kp7) echo KP_Home ;;
        kp8) echo KP_Up ;;
        kp9) echo KP_Prior ;;
        kpenter) echo KP_Enter ;;
        ctrl-s) echo ctrl+s ;;
        [a-z]) echo "$1" ;;
        *) return 1 ;;
    esac
}

if [[ "$skip_startup_selector" != "1" ]]; then
    for _ in 1 2 3; do
        tap_key 1
        tap_key Return
    done
fi

for token in $route_events; do
    low="${token,,}"
    echo "route-token-start $token $(python3 - <<'PY'
import time
print(f"{time.time():.6f}")
PY
)"
    case "$low" in
        shot|capture|screenshot|shot:*) shot ;;
        wait:*) sleep "$(python3 - "$low" <<'PY'
import sys
t = sys.argv[1]
print(int(t.split(':', 1)[1]) / 1000.0)
PY
)" ;;
        click:*|rclick:*|press:*)
            if [[ "$low" == rclick:* ]]; then
                coords="${low#rclick:}"
                click_original_frame "${coords%,*}" "${coords#*,}" 3
            elif [[ "$low" == press:* ]]; then
                coords="${low#press:}"
                click_original_frame "${coords%,*}" "${coords#*,}" 1 0
            else
                coords="${low#click:}"
                click_original_frame "${coords%,*}" "${coords#*,}" 1
            fi
            ;;
        release) release_original_frame_button ;;
        *)
            key="$(key_for_token "$low")" || { echo "unknown route token: $token" >&2; exit 2; }
            tap_key "$key"
            ;;
    esac
    echo "route-token-done $token $(python3 - <<'PY'
import time
print(f"{time.time():.6f}")
PY
)"
done
echo "route-complete $(python3 - <<'PY'
import time
print(f"{time.time():.6f}")
PY
)"
SH
    chmod +x "${KEY_HELPER_XDOTOOL}"
    echo "[pass-70] wrote ${CONF}"
    echo "[pass-70] wrote ${KEY_HELPER}"
    echo "[pass-70] wrote ${KEY_HELPER_XDOTOOL}"
}

normalize_existing() {
    local image_tool
    image_tool="$(need_image_tool)"
    mkdir -p "${CROP_DIR}"
    rm -f "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" "${CROP_MANIFEST}" "${SHOT_LABEL_MANIFEST}"

    python3 - "${OUT_DIR}" "${RAW_MANIFEST}" "${EXPECTED_SHOTS}" <<'PY'
from __future__ import annotations
from pathlib import Path
from datetime import datetime, timezone
import hashlib, struct, sys
out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
expected_raw = sys.argv[3].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
if expected <= 0:
    raise SystemExit("ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be positive")
paths = sorted(out.glob("image*.png"))
if not paths:
    # DOSBox 0.74 names screenshots after the running program/screen instead of
    # imageNNNN.png. Normalize raw 320x200, exact 640x400 2x, and
    # DOSBox-X's 720x400 aspect-corrected captures into the stable
    # image000N-raw.png names expected by the downstream pass70/pass84 tools.
    candidates = sorted(
        [p for p in out.glob("*.png") if p.parent == out and not p.name.startswith("image")
         and not p.name.startswith("host-window-")],
        key=lambda p: (p.stat().st_mtime_ns, p.name),
    )
    if len(candidates) == expected:
        normalized = []
        for idx, src in enumerate(candidates, 1):
            dst = out / f"image{idx:04d}-raw.png"
            dst.write_bytes(src.read_bytes())
            normalized.append(dst)
        paths = normalized
if not paths:
    raise SystemExit(f"ERROR: no DOSBox raw screenshots found under {out}/image*.png")
if len(paths) != expected:
    raise SystemExit(f"ERROR: expected exactly {expected} DOSBox raw screenshots under {out}/image*.png, found {len(paths)}")
with manifest.open("w") as f:
    f.write("index\tpath\tmtime_epoch_ns\tmtime_iso\tsha256\tsize_bytes\twidth\theight\n")
    for i, path in enumerate(paths):
        data = path.read_bytes()
        if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
            raise SystemExit(f"ERROR: not a PNG with IHDR: {path}")
        w, h = struct.unpack(">II", data[16:24])
        if (w, h) in {(640, 400), (720, 400)}:
            try:
                from PIL import Image
            except Exception as exc:
                raise SystemExit(
                    f"ERROR: {path} is a {w}x{h} DOSBox scaled capture; "
                    f"Python Pillow is required to normalize it to original 320x200: {exc}")
            im = Image.open(path).convert("RGB")
            resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
            im.resize((320, 200), resample).save(path)
            data = path.read_bytes()
            if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
                raise SystemExit(f"ERROR: normalized 2x capture is not a PNG with IHDR: {path}")
            w, h = struct.unpack(">II", data[16:24])
        if (w, h) != (320, 200):
            raise SystemExit(f"ERROR: expected raw screenshot 320x200, got {w}x{h} for {path}")
        st = path.stat()
        iso = datetime.fromtimestamp(st.st_mtime_ns / 1_000_000_000, timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")
        f.write(f"{i:02d}\t{path}\t{st.st_mtime_ns}\t{iso}\t{hashlib.sha256(data).hexdigest()}\t{st.st_size}\t{w}\t{h}\n")
PY

    python3 - "${OUT_DIR}" "${RAW_HEALTH_MANIFEST}" "${image_tool}" "${EXPECTED_SHOTS}" <<'PY'
from __future__ import annotations
from pathlib import Path
import hashlib
import json
import subprocess
import sys

out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
image_tool = sys.argv[3]
expected_raw = sys.argv[4].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
if expected <= 0:
    raise SystemExit("ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be positive")

def ppm_pixels(data: bytes, path: Path) -> tuple[tuple[int, int], list[tuple[int, int, int]]]:
    tokens: list[bytes] = []
    i = 0
    n = len(data)
    while len(tokens) < 4 and i < n:
        while i < n and data[i] in b" \t\r\n":
            i += 1
        if i < n and data[i] == ord("#"):
            while i < n and data[i] not in b"\r\n":
                i += 1
            continue
        start = i
        while i < n and data[i] not in b" \t\r\n":
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4 or tokens[0] != b"P6" or tokens[3] != b"255":
        raise SystemExit(f"ERROR: ImageMagick did not produce binary PPM for {path}")
    while i < n and data[i] in b" \t\r\n":
        i += 1
    width = int(tokens[1])
    height = int(tokens[2])
    raw = data[i:]
    if len(raw) != width * height * 3:
        raise SystemExit(f"ERROR: PPM pixel payload size mismatch for {path}")
    pixels = [(raw[j], raw[j + 1], raw[j + 2]) for j in range(0, len(raw), 3)]
    return (width, height), pixels

def load_pixels(path: Path) -> tuple[tuple[int, int], list[tuple[int, int, int]]]:
    if image_tool == "pillow":
        from PIL import Image
        im = Image.open(path).convert("RGB")
        return im.size, list(im.get_flattened_data())
    data = subprocess.check_output([image_tool, str(path), "ppm:-"])
    return ppm_pixels(data, path)

paths = sorted(out.glob("image*.png"))
rows = []
problems = []
for idx, path in enumerate(paths, 1):
    dims, pixels = load_pixels(path)
    total = len(pixels)
    nonblack = sum(1 for rgb in pixels if rgb != (0, 0, 0))
    # A host capture can contain only a DOSBox-X title/menu strip at the top.
    # Treat that as blank even though its overall non-black ratio is nonzero:
    # the original 320x200 frame must contain pixels below the upper host UI.
    lower_pixels = [
        rgb for y in range(dims[1] // 5, dims[1])
        for rgb in pixels[y * dims[0]:(y + 1) * dims[0]]
    ]
    lower_nonblack = sum(1 for rgb in lower_pixels if rgb != (0, 0, 0))
    unique = len(set(pixels))
    data = path.read_bytes()
    row = {
        "index": idx,
        "path": str(path),
        "width": dims[0],
        "height": dims[1],
        "sizeBytes": path.stat().st_size,
        "sha256": hashlib.sha256(data).hexdigest(),
        "nonblackRatio": round(nonblack / total, 6),
        "lowerCanvasNonblackRatio": round(lower_nonblack / len(lower_pixels), 6),
        "uniqueColors": unique,
    }
    if dims != (320, 200):
        problems.append(f"{path.name}: rawshot dimensions are {dims[0]}x{dims[1]}, expected 320x200")
    if row["nonblackRatio"] <= 0.005 or unique <= 1:
        problems.append(f"{path.name}: black/blank rawshot candidate nonblack={row['nonblackRatio']} uniqueColors={unique}")
    if row["lowerCanvasNonblackRatio"] <= 0.005:
        problems.append(f"{path.name}: no meaningful canvas content below host UI lowerCanvasNonblack={row['lowerCanvasNonblackRatio']}")
    rows.append(row)
payload = {
    "schema": "dm1_original_raw_frame_health.v1",
    "attemptDir": str(out),
    "expectedCaptureCount": expected,
    "captureCount": len(rows),
    "honesty": "Rawshot health gate only. Passing this gate does not claim route semantics or pixel parity.",
    "captures": rows,
    "problems": problems,
    "pass": len(rows) == expected and not problems,
}
manifest.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n")
if not payload["pass"]:
    print(f"ERROR: raw screenshot health gate failed; see {manifest}", file=sys.stderr)
    for problem in problems:
        print(f"ERROR: {problem}", file=sys.stderr)
    raise SystemExit(9)
print(f"[pass-70] raw screenshot health OK: {manifest}")
PY

    local labels=(
        01_ingame_start_original_viewport_224x136
        02_ingame_turn_right_original_viewport_224x136
        03_ingame_move_forward_original_viewport_224x136
        04_ingame_spell_panel_original_viewport_224x136
        05_ingame_after_cast_original_viewport_224x136
        06_ingame_inventory_panel_original_viewport_224x136
    )
    local route_shot_labels=()
    if [[ -n "${ROUTE_EVENTS}" ]]; then
        local route_labels_tmp
        route_labels_tmp="$(mktemp "${OUT_DIR}/route-shot-labels.XXXXXX")"
        python3 - "${ROUTE_EVENTS}" > "${route_labels_tmp}" <<'PY'
import sys
for token in sys.argv[1].split():
    low = token.lower()
    if low in {"shot", "capture", "screenshot"}:
        print("")
    elif low.startswith("shot:"):
        print(low.split(":", 1)[1])
PY
        while IFS= read -r route_label || [[ -n "$route_label" ]]; do
            route_shot_labels+=("$route_label")
        done < "${route_labels_tmp}"
        rm -f "${route_labels_tmp}"
        if [[ ${#route_shot_labels[@]} -ne ${EXPECTED_SHOTS_COUNT} ]]; then
            echo "ERROR: expected exactly ${EXPECTED_SHOTS_COUNT} route shot labels, found ${#route_shot_labels[@]}" >&2
            exit 8
        fi
    fi
    printf 'index\tfilename\troute_label\troute_token\n' > "${SHOT_LABEL_MANIFEST}"
    local i=0 src legacy_label label route_label route_token ppm png
    while IFS= read -r src; do
        if [[ $i -ge ${#labels[@]} ]]; then
            break
        fi
        legacy_label="${labels[$i]}"
        route_label="${route_shot_labels[$i]:-}"
        if [[ -n "$route_label" ]]; then
            route_token="shot:${route_label}"
            label="$(python3 - "$((i + 1))" "$route_label" <<'PY'
import re
import sys
idx = int(sys.argv[1])
route_label = sys.argv[2]
stem = re.sub(r"[^a-z0-9_-]+", "_", route_label.lower()).strip("_")
if not stem:
    raise SystemExit("ERROR: empty normalized route label")
print(f"{idx:02d}_{stem}_original_viewport_224x136")
PY
)"
        else
            route_token="shot"
            label="${legacy_label}"
        fi
        ppm="${CROP_DIR}/${label}.ppm"
        png="${CROP_DIR}/${label}.png"
        if [[ "${image_tool}" == "pillow" ]]; then
            python3 - "$src" "$ppm" "$png" <<'PY'
from pathlib import Path
from PIL import Image
import sys
src, ppm, png = map(Path, sys.argv[1:4])
im = Image.open(src).convert("RGB")
if im.size != (320, 200):
    raise SystemExit(f"ERROR: expected raw screenshot 320x200, got {im.size[0]}x{im.size[1]} for {src}")
crop = im.crop((0, 33, 224, 169))
crop.save(ppm)
crop.save(png)
PY
        else
            "${image_tool}" "$src" -crop 224x136+0+33 +repage "$ppm"
            "${image_tool}" "$ppm" "$png" 2>/dev/null || true
        fi
        printf '%02d\t%s\t%s\t%s\n' "$((i + 1))" "${label}.ppm" "$route_label" "$route_token" >> "${SHOT_LABEL_MANIFEST}"
        i=$((i + 1))
    done < <(find "${OUT_DIR}" -maxdepth 1 -type f -name 'image*.png' | sort)

    python3 - "${CROP_DIR}" "${CROP_MANIFEST}" "${EXPECTED_SHOTS}" <<'PY'
from __future__ import annotations
from pathlib import Path
import hashlib, sys

def ppm_dims(data: bytes, path: Path) -> tuple[int, int]:
    tokens: list[bytes] = []
    i = 0
    n = len(data)
    while len(tokens) < 4 and i < n:
        while i < n and data[i] in b" \t\r\n":
            i += 1
        if i < n and data[i] == ord('#'):
            while i < n and data[i] not in b"\r\n":
                i += 1
            continue
        start = i
        while i < n and data[i] not in b" \t\r\n":
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4 or tokens[0] != b"P6" or tokens[3] != b"255":
        raise SystemExit(f"ERROR: not a binary PPM with maxval 255: {path}")
    return int(tokens[1]), int(tokens[2])

crop_dir = Path(sys.argv[1])
manifest = Path(sys.argv[2])
expected_raw = sys.argv[3].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
if expected <= 0:
    raise SystemExit("ERROR: DM1_ORIGINAL_EXPECTED_SHOTS must be positive")
paths = sorted(crop_dir.glob("*.ppm"))
if len(paths) != expected:
    raise SystemExit(f"ERROR: expected exactly {expected} normalized viewport PPM crops, found {len(paths)} in {crop_dir}")
with manifest.open("w") as f:
    f.write("kind\tfilename\twidth\theight\tbytes\tsha256\n")
    for path in paths:
        data = path.read_bytes()
        width, height = ppm_dims(data, path)
        if (width, height) != (224, 136):
            raise SystemExit(f"ERROR: wrong crop geometry for {path}: {width}x{height}")
        f.write(f"original_viewport_224x136\t{path.name}\t{width}\t{height}\t{len(data)}\t{hashlib.sha256(data).hexdigest()}\n")
PY
    ls -lh "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" "${CROP_MANIFEST}" "${SHOT_LABEL_MANIFEST}" "${CROP_DIR}"/* | tee "${SIZE_LOG}"
    if [[ -n "${ROUTE_EVENTS}" ]]; then
        python3 "${REPO}/tools/pass513_i34e_route_transcript_scaffold.py" \
            --repo-root "${REPO}" \
            --route-events "${ROUTE_EVENTS}" \
            --raw-manifest "${RAW_MANIFEST}" \
            --crop-manifest "${CROP_MANIFEST}" \
            --shot-labels "${SHOT_LABEL_MANIFEST}" \
            --out "${PASS513_SCAFFOLD}"
    fi
    echo "[pass-70] normalized original viewport crops: ${CROP_MANIFEST}"
}

case "$mode" in
    print-pass94-diagnostic)
        print_pass94_diagnostic
        exit 0
        ;;
    print-pass435-hoc-route)
        print_pass435_hoc_route
        exit 0
        ;;
    prepare)
        need_stage
        write_helpers
        exit 0
        ;;
    dry-run)
        if [[ ! -f "${SRC_STAGE}/DM.EXE" ]]; then
            echo "[blocked] staged DM1 tree missing: ${SRC_STAGE}"
            echo "          next: scripts/dosbox_dm1_capture.sh"
        fi
        write_helpers
        if [[ -z "${ROUTE_EVENTS}" ]]; then
            echo "[blocked] DM1_ORIGINAL_ROUTE_EVENTS is not set. Do not guess; validate the exact original keystroke route first."
        else
            echo "[pass-70] route events: ${ROUTE_EVENTS}"
            validate_route_shape
            write_route_plan_manifest "dry-run"
        fi
        echo "[pass-70] normalize command after raw screenshots exist: scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only"
        echo "[pass-70] normalize-only now writes ${RAW_HEALTH_MANIFEST} and fails black/blank rawshots before cropping"
        exit 0
        ;;
    preflight-route)
        preflight_route
        exit $?
        ;;
    normalize-only)
        normalize_existing
        exit 0
        ;;
    run)
        need_stage
        if [[ -z "${ROUTE_EVENTS}" ]]; then
            echo "ERROR: DM1_ORIGINAL_ROUTE_EVENTS is required for --run; refusing to guess original route/state." >&2
            echo "Example shape only (not validated): DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot:party_hud right wait:300 shot up wait:300 shot:spell_panel wait:300 shot wait:300 shot:inventory_panel'" >&2
            exit 5
        fi
        validate_route_shape
        if [[ ! -x "$DOSBOX" ]]; then
            echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
            exit 7
        fi
        write_helpers
        injector="$(select_route_injector || true)"
        if [[ "$injector" == "swift" ]]; then
            route_injector=(swift "$KEY_HELPER")
        elif [[ "$injector" == "xdotool" ]]; then
            if [[ -z "${DISPLAY:-}" ]]; then
                echo "ERROR: xdotool route injector selected but DISPLAY is not set; run under an X server such as xvfb-run -a" >&2
                exit 6
            fi
            route_injector=("$KEY_HELPER_XDOTOOL")
        else
            echo "ERROR: no supported route injector found; install Swift on macOS or xdotool on X11/Linux" >&2
            exit 6
        fi
        case "${DM1_DOSBOX_CAPTURE_BACKEND:-emulator}" in
            emulator) ;;
            host)
                if ! command -v scrot >/dev/null 2>&1; then
                    echo "ERROR: DM1_DOSBOX_CAPTURE_BACKEND=host requires scrot" >&2
                    exit 6
                fi
                export DM1_DOSBOX_CAPTURE_OUT_DIR="${OUT_DIR}"
                ;;
            *)
                echo "ERROR: unsupported DM1_DOSBOX_CAPTURE_BACKEND=${DM1_DOSBOX_CAPTURE_BACKEND}" >&2
                exit 6
                ;;
        esac
        # DOSBox-X on Linux maps screenshot capture to the host-key sequence
        # F12+P; Ctrl+F5 copies DOS text there. Vanilla DOSBox continues to
        # use Ctrl+F5. Keep an explicit caller override authoritative.
        if [[ -z "${DM1_DOSBOX_SCREENSHOT_HOTKEY:-}" &&
              "$(basename "$DOSBOX")" == "dosbox-x" ]]; then
            export DM1_DOSBOX_SCREENSHOT_HOTKEY="F12+p"
            SCREENSHOT_HOTKEY="$DM1_DOSBOX_SCREENSHOT_HOTKEY"
        fi
        write_route_plan_manifest "$injector"
        rm -f "${LOG}" "${PID_FILE}" "${KEY_LOG}" "${RAW_MANIFEST}" "${RAW_HEALTH_MANIFEST}" "${CROP_MANIFEST}" "${SIZE_LOG}"
        # DOSBox 0.74 names screenshots after the active program/window
        # (selector_NNN.png, fires_NNN.png) instead of imageNNNN.png.  Remove
        # stale top-level captures before a run so normalize_existing can map
        # exactly this run's six raw frames into stable image000N-raw.png names.
        rm -f "${OUT_DIR}"/*.png "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png
        # DOSBox-X requires the [dosbox] section name when setting this
        # space-containing option from its command line.
        "$DOSBOX" -exit -set "dosbox quit warning=false" -conf "$CONF" >"$LOG" 2>&1 &
        pid=$!
        echo "$pid" > "$PID_FILE"
        focus_dosbox_for_route "$pid"
        cleanup() {
            osascript -e 'tell application "DOSBox Staging" to quit' >/dev/null 2>&1 || true
            kill "$pid" >/dev/null 2>&1 || true
        }
        trap cleanup EXIT
        sleep "$(python3 - <<PY
print(${WAIT_BEFORE_INPUT_MS}/1000)
PY
)"
        focus_dosbox_for_route "$pid"
        if ! "${route_injector[@]}" "$pid" "$ROUTE_EVENTS" "$SKIP_STARTUP_SELECTOR" >"$KEY_LOG" 2>&1; then
            echo "ERROR: route injector failed; see ${KEY_LOG}" >&2
            tail -40 "$KEY_LOG" >&2 || true
            exit 8
        fi
        python3 - "$OUT_DIR" "$NEW_FILE_TIMEOUT_MS" "$EXPECTED_SHOTS" <<'PY'
from pathlib import Path
import sys, time
out = Path(sys.argv[1])
timeout = int(sys.argv[2]) / 1000.0
expected_raw = sys.argv[3].strip().lower()
expected = 1 if expected_raw in {"single", "single-row", "single-transcript-row", "pass625", "pass626"} else int(expected_raw)
start = time.monotonic()
png_iend = b"\x00\x00\x00\x00IEND\xaeB`\x82"

def complete_png(path: Path) -> bool:
    """Do not terminate DOSBox while its screenshot writer is still active."""
    try:
        data = path.read_bytes()
    except OSError:
        return False
    if len(data) < 45 or not data.startswith(b"\x89PNG\r\n\x1a\n"):
        return False
    # DOSBox-X appends its ``raw1`` capture trailer after a complete PNG
    # stream.  The PNG standard permits trailing bytes, and Pillow/file both
    # correctly accept these captures.  Looking for a complete IEND chunk
    # rather than requiring it to be the final bytes rejects partial writes
    # without discarding authentic DOSBox-X output.
    return data.find(png_iend) != -1

while time.monotonic() - start < timeout:
    images = sorted(out.glob("image*.png"))
    fallbacks = sorted(p for p in out.glob("*.png")
                       if p.parent == out and not p.name.startswith("image")
                       and not p.name.startswith("host-window-"))
    candidates = images if images else fallbacks
    if len(candidates) >= expected and all(complete_png(path) for path in candidates):
        break
    time.sleep(0.025)
image_count = len(list(out.glob("image*.png")))
fallback_count = len([p for p in out.glob("*.png") if p.parent == out
                      and not p.name.startswith("image")
                      and not p.name.startswith("host-window-")])
captured = max(image_count, fallback_count)
images = sorted(out.glob("image*.png"))
fallbacks = sorted(p for p in out.glob("*.png")
                   if p.parent == out and not p.name.startswith("image")
                   and not p.name.startswith("host-window-"))
candidates = images if images else fallbacks
if captured < expected or not all(complete_png(path) for path in candidates):
    raise SystemExit(
        f"ERROR: DOSBox produced {captured}/{expected} complete raw screenshots in {out}; "
        "refusing to normalize or promote a still-writing capture route. "
        "On Linux/DOSBox builds whose Ctrl+F5 writer is unavailable, rerun "
        "with DM1_DOSBOX_CAPTURE_BACKEND=host (and an X11 display); this "
        "captures the original emulator window but still requires semantic "
        "and duplicate-frame validation."
    )
PY
        normalize_existing
        ;;
esac
