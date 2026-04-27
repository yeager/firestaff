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
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
NEW_FILE_TIMEOUT_MS="${NEW_FILE_TIMEOUT_MS:-2500}"
ROUTE_EVENTS="${DM1_ORIGINAL_ROUTE_EVENTS:-}"
SKIP_STARTUP_SELECTOR="${DM1_ROUTE_SKIP_STARTUP_SELECTOR:-0}"
ORIGINAL_PROGRAM="${DM1_ORIGINAL_PROGRAM:-DM VGA}"
CONF="${OUT_DIR}/dosbox-original-viewports.conf"
LOG="${OUT_DIR}/dosbox-original-viewports.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/original_viewport_route_keys.swift"
KEY_LOG="${OUT_DIR}/original-viewpoint-route-keys.log"
SHOT_LABEL_MANIFEST="${OUT_DIR}/original_viewport_shot_labels.tsv"
RAW_MANIFEST="${OUT_DIR}/raw_manifest.tsv"
CROP_MANIFEST="${OUT_DIR}/original_viewport_224x136_manifest.tsv"
CROP_DIR="${OUT_DIR}/viewport_224x136"
SIZE_LOG="${OUT_DIR}/artifact-sizes.txt"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_original_viewport_reference_capture.sh [--prepare|--dry-run|--run|--normalize-only|--print-pass94-diagnostic]

Modes:
  --prepare                  write DOSBox config and Swift key helper only (default)
  --dry-run                  show blockers/plan, no launch
  --run                      launch DOSBox, post an explicit route, capture raw frames, normalize crops
  --normalize-only           crop/hash existing image*.png raw screenshots in OUT_DIR
  --print-pass94-diagnostic  print the pass94 entrance-click diagnostic command and audit expectations

Required for --run:
  DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot:party_hud right wait:300 shot up wait:300 shot:spell_panel ...'

Supported route tokens:
  shot, shot:<label>, wait:<ms>, click:<x>,<y>, enter, esc, space, up, down,
  left, right, one, two, three, four, five, six, f1-f4, kp0-kp9,
  kpenter, a-z, 0-9

Labeled shot tokens:
  shot:<label> is equivalent to shot for capture input, and records the label
  in original_viewport_shot_labels.tsv during normalization. Labels are
  lowercase route semantics such as shot:party_hud, shot:spell_panel, and
  shot:inventory_panel. They are evidence metadata only; they do not claim
  pixel parity or validate that the original runtime reached that state.

Outputs:
  raw screenshots: ${OUT_DIR}/image*.png (DOSBox Staging raw 320x200 if capture succeeds)
  crops:           ${CROP_DIR}/*.ppm and *.png

Optional environment:
  DM1_ORIGINAL_STAGE_DIR=/path/to/DM1-PC34-tree-with-DM.EXE
                    override the default staged tree path
  DM1_ROUTE_SKIP_STARTUP_SELECTOR=1
                    skip legacy graphics/sound/input selector keystrokes when
                    the DOSBox config launches 'DM VGA' directly
  DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk'
                    override autoexec launch command; recommended for bypassing
                    the original selector and entering VGA/no-sound/keyboard mode
                    directly. 'DM VGA' remains the default for legacy runs.
  click:<x>,<y>    posts one serialized left-click in original 320x200 game
                    coordinates. Use waits around clicks; ReDMCSB BUG0_73 shows
                    mixed mouse/keyboard commands can be lost when packed tightly.
  manifest:        ${CROP_MANIFEST}
  shot labels:     ${SHOT_LABEL_MANIFEST}

Honesty note:
  The route string must be validated against the original runtime state that
  corresponds to Firestaff's run_capture_screenshots.sh sequence.  This script
  will not invent that route. Use DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' to
  bypass the text selector; audit raw captures before accepting references.
EOF
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --run) mode="run"; shift ;;
        --normalize-only) mode="normalize-only"; shift ;;
        --print-pass94-diagnostic) mode="print-pass94-diagnostic"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

print_pass94_diagnostic() {
    cat <<EOF
# Pass 94 original entrance-click diagnostic (manual/original-route unblock only).
# This is not parity evidence. It should answer whether click:260,50 leaves the entrance menu.

OUT_DIR=\$PWD/verification-screens/pass94-hall-map-enter-diagnostic \\
DM1_ORIGINAL_STAGE_DIR=\$PWD/verification-screens/dm1-dosbox-capture/DungeonMasterPC34 \\
DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \\
DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\
WAIT_BEFORE_INPUT_MS=5000 \\
NEW_FILE_TIMEOUT_MS=6000 \\
DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 shot:title enter wait:1200 shot:pre_enter_menu click:260,50 wait:1200 shot:after_enter_click click:276,140 wait:600 shot:forward_1 click:276,140 wait:600 shot:forward_2 click:246,140 wait:600 shot:left_turn_probe' \\
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

need_stage() {
    if [[ ! -f "${SRC_STAGE}/DM.EXE" ]]; then
        echo "ERROR: staged DM1 PC 3.4 tree missing: ${SRC_STAGE}" >&2
        echo "Run scripts/dosbox_dm1_capture.sh first, or set DM1_ORIGINAL_STAGE_DIR to an existing DM1 PC 3.4 tree. If staging fails, the missing input is original-games/Game,Dungeon_Master,DOS,Software.7z." >&2
        exit 3
    fi
}

need_image_tool() {
    if command -v magick >/dev/null 2>&1; then
        echo magick
    elif command -v convert >/dev/null 2>&1; then
        echo convert
    else
        if python3 - <<'PY' >/dev/null 2>&1
from PIL import Image
PY
        then
            echo pillow
        else
            echo "ERROR: ImageMagick (magick/convert) or Python Pillow is required for PNG->PPM viewport crop normalization." >&2
            exit 4
        fi
    fi
}

validate_route_shape() {
    if [[ -z "${ROUTE_EVENTS}" ]]; then
        return 0
    fi
    python3 - "${ROUTE_EVENTS}" <<'PY'
import re, sys
route = sys.argv[1].split()
allowed = set("shot capture screenshot enter return esc escape space up down left right one two three four five six zero".split())
allowed |= set("abcdefghijklmnopqrstuvwxyz") | set("0123456789") | {f"kp{i}" for i in range(10)} | {f"f{i}" for i in range(1, 5)} | {"kpenter"}
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
    if low.startswith("click:"):
        m = re.fullmatch(r"click:([0-9]{1,3}),([0-9]{1,3})", low)
        if not m:
            raise SystemExit(f"ERROR: invalid click token: {token}")
        x, y = map(int, m.groups())
        if not (0 <= x < 320 and 0 <= y < 200):
            raise SystemExit(f"ERROR: click token outside original 320x200 frame: {token}")
        continue
    if low not in allowed:
        raise SystemExit(f"ERROR: unknown route token: {token}")
if shots != 6:
    raise SystemExit(f"ERROR: DM1_ORIGINAL_ROUTE_EVENTS must contain exactly 6 shot or shot:<label> tokens, found {shots}")
print(f"[pass-70] route shape OK: {len(route)} tokens, {shots} shots, {labeled_shots} labeled")
if labeled_shots:
    pretty = ", ".join(f"{idx + 1:02d}:{label or '(unlabeled)'}" for idx, label in enumerate(labels))
    print(f"[pass-70] shot label plan: {pretty}")
diagnostic_hits = [label for label in labels if label in diagnostic_only]
if diagnostic_hits:
    print("[pass-70] diagnostic-only labels present: " + ", ".join(diagnostic_hits))
    print("[pass-70] note: diagnostic labels are for manual/pass94 routing only; pass84 overlay readiness requires party_hud, blank, blank, spell_panel, blank, inventory_panel")
PY
}

write_helpers() {
    mkdir -p "${OUT_DIR}" "${CROP_DIR}"
    cat > "${CONF}" <<EOF
[sdl]
fullscreen=false
output=opengl

[dosbox]
machine=svga_paradise
memsize=4

[cpu]
core=normal
cputype=386
cpu_cycles=3000

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
func cmdF5() {
    post(55, true, flags: .maskCommand)   // Command
    usleep(20_000)
    post(96, true, flags: .maskCommand)   // F5
    usleep(20_000)
    post(96, false, flags: .maskCommand)
    usleep(20_000)
    post(55, false)
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

func clickOriginalFrame(x: Int, y: Int) {
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
    guard let down = CGEvent(mouseEventSource: source, mouseType: .leftMouseDown, mouseCursorPosition: point, mouseButton: .left),
          let up = CGEvent(mouseEventSource: source, mouseType: .leftMouseUp, mouseCursorPosition: point, mouseButton: .left) else { return }
    down.postToPid(pid)
    usleep(45_000)
    up.postToPid(pid)
    print("click-mapped \(x),\(y) -> \(Int(px)),\(Int(py)) window=\(Int(bounds.width))x\(Int(bounds.height))")
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
    print("route-token \(token)")
    if lowerToken == "shot" || lowerToken == "capture" || lowerToken == "screenshot" || lowerToken.hasPrefix("shot:") {
        cmdF5()
    } else if lowerToken.hasPrefix("wait:") {
        let msText = String(lowerToken.dropFirst("wait:".count))
        guard let ms = UInt32(msText) else {
            fputs("invalid wait token: \(token)\n", stderr)
            exit(2)
        }
        usleep(ms * 1000)
    } else if lowerToken.hasPrefix("click:") {
        let coords = lowerToken.dropFirst("click:".count).split(separator: ",")
        guard coords.count == 2, let x = Int(coords[0]), let y = Int(coords[1]), x >= 0, x < 320, y >= 0, y < 200 else {
            fputs("invalid click token: \(token)\n", stderr)
            exit(2)
        }
        clickOriginalFrame(x: x, y: y)
    } else if let key = keycodes[lowerToken] {
        tap(key)
    } else {
        fputs("unknown route token: \(token)\n", stderr)
        exit(2)
    }
}
SWIFT
    echo "[pass-70] wrote ${CONF}"
    echo "[pass-70] wrote ${KEY_HELPER}"
}

normalize_existing() {
    local image_tool
    image_tool="$(need_image_tool)"
    mkdir -p "${CROP_DIR}"
    rm -f "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png "${RAW_MANIFEST}" "${CROP_MANIFEST}" "${SHOT_LABEL_MANIFEST}"

    python3 - "${OUT_DIR}" "${RAW_MANIFEST}" <<'PY'
from __future__ import annotations
from pathlib import Path
from datetime import datetime, timezone
import hashlib, struct, sys
out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
paths = sorted(out.glob("image*.png"))
if not paths:
    raise SystemExit(f"ERROR: no DOSBox raw screenshots found under {out}/image*.png")
if len(paths) != 6:
    raise SystemExit(f"ERROR: expected exactly 6 DOSBox raw screenshots under {out}/image*.png, found {len(paths)}")
with manifest.open("w") as f:
    f.write("index\tpath\tmtime_epoch_ns\tmtime_iso\tsha256\tsize_bytes\twidth\theight\n")
    for i, path in enumerate(paths):
        data = path.read_bytes()
        if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
            raise SystemExit(f"ERROR: not a PNG with IHDR: {path}")
        w, h = struct.unpack(">II", data[16:24])
        st = path.stat()
        iso = datetime.fromtimestamp(st.st_mtime_ns / 1_000_000_000, timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")
        f.write(f"{i:02d}\t{path}\t{st.st_mtime_ns}\t{iso}\t{hashlib.sha256(data).hexdigest()}\t{st.st_size}\t{w}\t{h}\n")
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
        if [[ ${#route_shot_labels[@]} -ne 6 ]]; then
            echo "ERROR: expected exactly 6 route shot labels, found ${#route_shot_labels[@]}" >&2
            exit 8
        fi
    fi
    printf 'index\tfilename\troute_label\troute_token\n' > "${SHOT_LABEL_MANIFEST}"
    local i=0 src label route_label route_token ppm png
    while IFS= read -r src; do
        if [[ $i -ge ${#labels[@]} ]]; then
            break
        fi
        label="${labels[$i]}"
        route_label="${route_shot_labels[$i]:-}"
        if [[ -n "$route_label" ]]; then
            route_token="shot:${route_label}"
        else
            route_token="shot"
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

    python3 - "${CROP_DIR}" "${CROP_MANIFEST}" <<'PY'
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
paths = sorted(crop_dir.glob("*.ppm"))
if len(paths) != 6:
    raise SystemExit(f"ERROR: expected exactly 6 normalized viewport PPM crops, found {len(paths)} in {crop_dir}")
with manifest.open("w") as f:
    f.write("kind\tfilename\twidth\theight\tbytes\tsha256\n")
    for path in paths:
        data = path.read_bytes()
        width, height = ppm_dims(data, path)
        if (width, height) != (224, 136):
            raise SystemExit(f"ERROR: wrong crop geometry for {path}: {width}x{height}")
        f.write(f"original_viewport_224x136\t{path.name}\t{width}\t{height}\t{len(data)}\t{hashlib.sha256(data).hexdigest()}\n")
PY
    ls -lh "${RAW_MANIFEST}" "${CROP_MANIFEST}" "${SHOT_LABEL_MANIFEST}" "${CROP_DIR}"/* | tee "${SIZE_LOG}"
    echo "[pass-70] normalized original viewport crops: ${CROP_MANIFEST}"
}

case "$mode" in
    print-pass94-diagnostic)
        print_pass94_diagnostic
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
        fi
        echo "[pass-70] normalize command after raw screenshots exist: scripts/dosbox_dm1_original_viewport_reference_capture.sh --normalize-only"
        exit 0
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
        if ! command -v swift >/dev/null 2>&1; then
            echo "ERROR: swift is required for targeted macOS CGEvent input" >&2
            exit 6
        fi
        if [[ ! -x "$DOSBOX" ]]; then
            echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
            exit 7
        fi
        write_helpers
        rm -f "${LOG}" "${PID_FILE}" "${KEY_LOG}" "${RAW_MANIFEST}" "${CROP_MANIFEST}" "${SIZE_LOG}"
        rm -f "${OUT_DIR}"/image*.png "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png
        "$DOSBOX" -conf "$CONF" >"$LOG" 2>&1 &
        pid=$!
        echo "$pid" > "$PID_FILE"
        cleanup() {
            osascript -e 'tell application "DOSBox Staging" to quit' >/dev/null 2>&1 || true
            kill "$pid" >/dev/null 2>&1 || true
        }
        trap cleanup EXIT
        sleep "$(python3 - <<PY
print(${WAIT_BEFORE_INPUT_MS}/1000)
PY
)"
        swift "$KEY_HELPER" "$pid" "$ROUTE_EVENTS" "$SKIP_STARTUP_SELECTOR" >"$KEY_LOG" 2>&1
        python3 - "$OUT_DIR" "$NEW_FILE_TIMEOUT_MS" <<'PY'
from pathlib import Path
import sys, time
out = Path(sys.argv[1])
timeout = int(sys.argv[2]) / 1000.0
start = time.monotonic()
while time.monotonic() - start < timeout:
    if len(list(out.glob("image*.png"))) >= 6:
        break
    time.sleep(0.025)
PY
        normalize_existing
        ;;
esac
