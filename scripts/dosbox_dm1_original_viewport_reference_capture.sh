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
CONF="${OUT_DIR}/dosbox-original-viewports.conf"
LOG="${OUT_DIR}/dosbox-original-viewports.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/original_viewport_route_keys.swift"
KEY_LOG="${OUT_DIR}/original-viewpoint-route-keys.log"
RAW_MANIFEST="${OUT_DIR}/raw_manifest.tsv"
CROP_MANIFEST="${OUT_DIR}/original_viewport_224x136_manifest.tsv"
CROP_DIR="${OUT_DIR}/viewport_224x136"
SIZE_LOG="${OUT_DIR}/artifact-sizes.txt"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_original_viewport_reference_capture.sh [--prepare|--dry-run|--run|--normalize-only]

Modes:
  --prepare         write DOSBox config and Swift key helper only (default)
  --dry-run         show blockers/plan, no launch
  --run             launch DOSBox, post an explicit route, capture raw frames, normalize crops
  --normalize-only  crop/hash existing image*.png raw screenshots in OUT_DIR

Required for --run:
  DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot right wait:300 shot up wait:300 shot ...'

Supported route tokens:
  shot, wait:<ms>, enter, esc, space, up, down, left, right, one, two, three,
  four, five, six, a-z, 0-9

Outputs:
  raw screenshots: ${OUT_DIR}/image*.png (DOSBox Staging raw 320x200 if capture succeeds)
  crops:           ${CROP_DIR}/*.ppm and *.png

Optional environment:
  DM1_ORIGINAL_STAGE_DIR=/path/to/DM1-PC34-tree-with-DM.EXE
                    override the default staged tree path
  manifest:        ${CROP_MANIFEST}

Honesty note:
  The route string must be validated against the original runtime state that
  corresponds to Firestaff's run_capture_screenshots.sh sequence.  This script
  will not invent that route.
EOF
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --run) mode="run"; shift ;;
        --normalize-only) mode="normalize-only"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

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
allowed |= set("abcdefghijklmnopqrstuvwxyz") | set("0123456789")
shots = 0
for token in route:
    low = token.lower()
    if low in {"shot", "capture", "screenshot"}:
        shots += 1
        continue
    if low.startswith("wait:"):
        if not re.fullmatch(r"wait:[0-9]+", low):
            raise SystemExit(f"ERROR: invalid wait token: {token}")
        continue
    if low not in allowed:
        raise SystemExit(f"ERROR: unknown route token: {token}")
if shots != 6:
    raise SystemExit(f"ERROR: DM1_ORIGINAL_ROUTE_EVENTS must contain exactly 6 shot tokens, found {shots}")
print(f"[pass-70] route shape OK: {len(route)} tokens, {shots} shots")
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
DM VGA
EOF

    cat > "${KEY_HELPER}" <<'SWIFT'
import Foundation
import CoreGraphics

if CommandLine.arguments.count != 3 {
    fputs("usage: original_viewport_route_keys.swift PID ROUTE_EVENTS\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}
let route = CommandLine.arguments[2].split(separator: " ").map(String.init)
let source = CGEventSource(stateID: .hidSystemState)

let keycodes: [String: CGKeyCode] = [
    "a": 0, "s": 1, "d": 2, "f": 3, "h": 4, "g": 5, "z": 6, "x": 7, "c": 8, "v": 9,
    "b": 11, "q": 12, "w": 13, "e": 14, "r": 15, "y": 16, "t": 17,
    "one": 18, "1": 18, "two": 19, "2": 19, "three": 20, "3": 20, "four": 21, "4": 21,
    "six": 22, "6": 22, "five": 23, "5": 23, "zero": 29, "0": 29,
    "o": 31, "u": 32, "i": 34, "p": 35, "l": 37, "j": 38, "k": 40,
    "n": 45, "m": 46,
    "enter": 36, "return": 36, "space": 49, "esc": 53, "escape": 53,
    "left": 123, "right": 124, "down": 125, "up": 126
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

// Original PC 3.4 startup selector: graphics=1, sound=1, input=1.
for _ in 0..<3 {
    tap(18) // '1'
    tap(36) // Return
}

for token in route {
    if token == "shot" || token == "capture" || token == "screenshot" {
        cmdF5()
    } else if token.hasPrefix("wait:") {
        let msText = String(token.dropFirst("wait:".count))
        guard let ms = UInt32(msText) else {
            fputs("invalid wait token: \(token)\n", stderr)
            exit(2)
        }
        usleep(ms * 1000)
    } else if let key = keycodes[token.lowercased()] {
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
    rm -f "${CROP_DIR}"/*.ppm "${CROP_DIR}"/*.png "${RAW_MANIFEST}" "${CROP_MANIFEST}"

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
    local i=0 src label ppm png
    while IFS= read -r src; do
        if [[ $i -ge ${#labels[@]} ]]; then
            break
        fi
        label="${labels[$i]}"
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
    ls -lh "${RAW_MANIFEST}" "${CROP_MANIFEST}" "${CROP_DIR}"/* | tee "${SIZE_LOG}"
    echo "[pass-70] normalized original viewport crops: ${CROP_MANIFEST}"
}

case "$mode" in
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
            echo "Example shape only (not validated): DM1_ORIGINAL_ROUTE_EVENTS='wait:7000 enter wait:1500 shot right wait:300 shot up wait:300 shot wait:300 shot wait:300 shot wait:300 shot'" >&2
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
        swift "$KEY_HELPER" "$pid" "$ROUTE_EVENTS" >"$KEY_LOG" 2>&1
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
