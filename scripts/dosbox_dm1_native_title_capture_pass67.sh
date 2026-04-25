#!/usr/bin/env bash
# Pass 67 — native DOSBox Staging raw screenshot sequence for original TITLE runtime.
#
# Uses DOSBox Staging's own Cmd+F5 screenshot action with
# default_image_capture_formats=raw, avoiding macOS host-window crop timing.  The
# resulting imageNNNN-raw.png files are raw emulator framebuffer captures; in VGA
# mode 13h they are 320x200 PNGs.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_STAGE="${REPO}/verification-screens/dm1-dosbox-capture/DungeonMasterPC34"
OUT_DIR="${REPO}/verification-screens/pass67-native-title-raw-sequence"
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
WAIT_AFTER_INPUT_MS="${WAIT_AFTER_INPUT_MS:-100}"
WAIT_BETWEEN_SHOTS_MS="${WAIT_BETWEEN_SHOTS_MS:-55}"
SHOT_COUNT="${SHOT_COUNT:-72}"
NEW_FILE_TIMEOUT_MS="${NEW_FILE_TIMEOUT_MS:-1200}"
CONF="${OUT_DIR}/dosbox-native-title.conf"
LOG="${OUT_DIR}/dosbox-native-title.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/native_title_keys.swift"
KEY_LOG="${OUT_DIR}/native-title-keys.log"
MANIFEST="${OUT_DIR}/manifest.tsv"
SIZE_LOG="${OUT_DIR}/artifact-sizes.txt"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_native_title_capture_pass67.sh [--prepare|--dry-run|--run]

Modes:
  --prepare   write the DOSBox config and Swift helper only (default)
  --dry-run   write helpers and print the native raw screenshot plan, no launch
  --run       launch DOSBox, enter selector choices, and trigger native raw screenshots

Environment overrides:
  DOSBOX=/path/to/dosbox-staging
  WAIT_BEFORE_INPUT_MS=3000
  WAIT_AFTER_INPUT_MS=100
  WAIT_BETWEEN_SHOTS_MS=55
  SHOT_COUNT=72
  NEW_FILE_TIMEOUT_MS=1200

Outputs:
  ${OUT_DIR}
EOF
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --dry-run) mode="dry-run"; shift ;;
        --run) mode="run"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ ! -f "${SRC_STAGE}/DM.EXE" ]]; then
    echo "ERROR: staged DM1 PC 3.4 tree missing: ${SRC_STAGE}" >&2
    echo "Run scripts/dosbox_dm1_capture.sh first." >&2
    exit 3
fi
if ! command -v swift >/dev/null 2>&1; then
    echo "ERROR: swift is required for targeted macOS CGEvent input" >&2
    exit 4
fi

mkdir -p "${OUT_DIR}"
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

if CommandLine.arguments.count != 5 {
    fputs("usage: native_title_keys.swift PID SHOT_COUNT WAIT_AFTER_INPUT_MS WAIT_BETWEEN_SHOTS_MS\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]),
      let shotCount = Int(CommandLine.arguments[2]),
      let waitAfterInputMs = UInt32(CommandLine.arguments[3]),
      let waitMs = UInt32(CommandLine.arguments[4]) else {
    fputs("invalid arguments\n", stderr)
    exit(2)
}

let source = CGEventSource(stateID: .hidSystemState)
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
    // macOS DOSBox Staging maps screenshot to Cmd+F5.  Posting a standalone
    // command-key down event matters; setting flags on F5 alone does not
    // reliably trigger the mapper action.
    post(55, true, flags: .maskCommand)   // Command
    usleep(20_000)
    post(96, true, flags: .maskCommand)   // F5
    usleep(20_000)
    post(96, false, flags: .maskCommand)
    usleep(20_000)
    post(55, false)
}

// Original PC 3.4 selector choices: graphics=1, sound=1, input=1.
for _ in 0..<3 {
    tap(18) // '1'
    tap(36) // Return
}

usleep(waitAfterInputMs * 1000)

for i in 0..<shotCount {
    cmdF5()
    if i + 1 < shotCount {
        usleep(waitMs * 1000)
    }
}
SWIFT

echo "[pass-67] wrote ${CONF}"
echo "[pass-67] wrote ${KEY_HELPER}"

case "$mode" in
    prepare)
        exit 0
        ;;
    dry-run)
        echo "[pass-67] selector sequence: keycode 18 ('1'), keycode 36 (Return), repeated 3 times"
        echo "[pass-67] native screenshot plan: wait ${WAIT_AFTER_INPUT_MS} ms after selector, then ${SHOT_COUNT} Cmd+F5 raw screenshots, ${WAIT_BETWEEN_SHOTS_MS} ms requested gap, capture_dir ${OUT_DIR}"
        exit 0
        ;;
    run)
        ;;
esac

if [[ ! -x "$DOSBOX" ]]; then
    echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
    exit 5
fi

rm -f "$LOG" "$PID_FILE" "$KEY_LOG" "$MANIFEST" "$SIZE_LOG"
rm -f "${OUT_DIR}"/image*.png
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

# The helper sends the selector and all screenshot hotkeys to DOSBox's PID.
# Then this wrapper waits until all generated files are visible and writes a
# reproducibility manifest with file mtimes and dimensions.
swift "$KEY_HELPER" "$pid" "$SHOT_COUNT" "$WAIT_AFTER_INPUT_MS" "$WAIT_BETWEEN_SHOTS_MS" >"$KEY_LOG" 2>&1

python3 - "$OUT_DIR" "$SHOT_COUNT" "$NEW_FILE_TIMEOUT_MS" <<'PY'
from __future__ import annotations
from pathlib import Path
import sys, time
out = Path(sys.argv[1])
want = int(sys.argv[2])
timeout = int(sys.argv[3]) / 1000.0
start = time.monotonic()
while time.monotonic() - start < timeout:
    if len(list(out.glob("image*.png"))) >= want:
        break
    time.sleep(0.025)
PY

printf 'index\tpath\tmtime_epoch_ns\tmtime_iso\tsha256\tsize_bytes\twidth\theight\n' > "$MANIFEST"
python3 - "$OUT_DIR" "$MANIFEST" <<'PY'
from __future__ import annotations
from pathlib import Path
from datetime import datetime, timezone
import hashlib, os, sys
from PIL import Image
out = Path(sys.argv[1])
manifest = Path(sys.argv[2])
paths = sorted(out.glob("image*.png"))
with manifest.open("a") as f:
    for i, path in enumerate(paths):
        st = path.stat()
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        with Image.open(path) as im:
            w, h = im.size
        iso = datetime.fromtimestamp(st.st_mtime_ns / 1_000_000_000, timezone.utc).isoformat(timespec="microseconds").replace("+00:00", "Z")
        f.write(f"{i:02d}\t{path}\t{st.st_mtime_ns}\t{iso}\t{digest}\t{st.st_size}\t{w}\t{h}\n")
PY

ls -lh "$MANIFEST" "$LOG" "$KEY_LOG" "${OUT_DIR}"/image*.png | tee "$SIZE_LOG"
