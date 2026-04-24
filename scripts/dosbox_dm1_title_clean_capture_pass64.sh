#!/usr/bin/env bash
# Pass 64 — clean, targeted DOSBox Staging TITLE capture for DM1 PC 3.4.
#
# This wrapper builds on the pass-63 selector path, but avoids foreground/overlay
# fragility by:
#   * posting the selector key events directly to the DOSBox process; and
#   * capturing the DOSBox window by CGWindowID instead of whole-screen capture.
# Evidence remains local under verification-screens/ and is not committed unless
# explicitly curated.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_STAGE="${REPO}/verification-screens/dm1-dosbox-capture/DungeonMasterPC34"
OUT_DIR="${REPO}/verification-screens/pass64-clean-title-capture"
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
WAIT_AFTER_INPUT_MS="${WAIT_AFTER_INPUT_MS:-7000}"
WAIT_BETWEEN_CAPTURES_MS="${WAIT_BETWEEN_CAPTURES_MS:-1200}"
CONF="${OUT_DIR}/dosbox-title-clean.conf"
LOG="${OUT_DIR}/dosbox-title-clean.log"
PID_FILE="${OUT_DIR}/dosbox.pid"
KEY_HELPER="${OUT_DIR}/post_selector_keys.swift"
WINDOW_HELPER="${OUT_DIR}/find_dosbox_window.swift"
KEY_LOG="${OUT_DIR}/post-selector-keys.log"
WINDOW_LOG="${OUT_DIR}/dosbox-window.txt"
SCREEN_0="${OUT_DIR}/title-clean-window-00.png"
SCREEN_1="${OUT_DIR}/title-clean-window-01.png"
SIZE_LOG="${OUT_DIR}/artifact-sizes.txt"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_title_clean_capture_pass64.sh [--prepare|--dry-run|--run]

Modes:
  --prepare   write the DOSBox config and Swift helpers only (default)
  --dry-run   write helpers and print the targeted selector sequence, no launch
  --run       launch DOSBox, target selector input to its PID, and capture the DOSBox window

Environment overrides:
  DOSBOX=/path/to/dosbox-staging
  WAIT_BEFORE_INPUT_MS=3000
  WAIT_AFTER_INPUT_MS=7000
  WAIT_BETWEEN_CAPTURES_MS=1200

Selector sequence used by --run:
  1 <Return>  # VGA Graphics
  1 <Return>  # No Sound
  1 <Return>  # Mouse

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
    echo "ERROR: swift is required for targeted macOS CGEvent/CGWindow helpers" >&2
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

if CommandLine.arguments.count != 2 {
    fputs("usage: post_selector_keys.swift PID\n", stderr)
    exit(2)
}

guard let pid = pid_t(CommandLine.arguments[1]) else {
    fputs("invalid pid\n", stderr)
    exit(2)
}

let source = CGEventSource(stateID: .hidSystemState)
func tap(_ key: CGKeyCode, _ delay: useconds_t = 120_000) {
    CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: true)?.postToPid(pid)
    usleep(20_000)
    CGEvent(keyboardEventSource: source, virtualKey: key, keyDown: false)?.postToPid(pid)
    usleep(delay)
}

// US ANSI virtual keycodes: 18 = '1', 36 = Return.
for _ in 0..<3 {
    tap(18)
    tap(36)
}
SWIFT

cat > "${WINDOW_HELPER}" <<'SWIFT'
import Foundation
import CoreGraphics

let opts = CGWindowListOption(arrayLiteral: .optionOnScreenOnly, .excludeDesktopElements)
guard let windows = CGWindowListCopyWindowInfo(opts, kCGNullWindowID) as? [[String: Any]] else {
    exit(1)
}

for window in windows {
    let owner = window[kCGWindowOwnerName as String] as? String ?? ""
    let layer = window[kCGWindowLayer as String] as? Int ?? -1
    if owner == "DOSBox Staging", layer == 0 {
        let id = window[kCGWindowNumber as String] as? Int ?? 0
        let name = window[kCGWindowName as String] as? String ?? ""
        let bounds = window[kCGWindowBounds as String] ?? ""
        print("id=\(id)")
        print("owner=\(owner)")
        print("name=\(name)")
        print("bounds=\(bounds)")
        exit(0)
    }
}

exit(1)
SWIFT

echo "[pass-64] wrote ${CONF}"
echo "[pass-64] wrote ${KEY_HELPER}"
echo "[pass-64] wrote ${WINDOW_HELPER}"

case "$mode" in
    prepare)
        exit 0
        ;;
    dry-run)
        echo "[pass-64] targeted PID sequence: keycode 18 ('1'), keycode 36 (Return), repeated 3 times"
        exit 0
        ;;
    run)
        ;;
esac

if [[ ! -x "$DOSBOX" ]]; then
    echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
    exit 5
fi
if ! command -v screencapture >/dev/null 2>&1; then
    echo "ERROR: screencapture not found" >&2
    exit 6
fi

rm -f "$LOG" "$PID_FILE" "$KEY_LOG" "$WINDOW_LOG" "$SCREEN_0" "$SCREEN_1" "$SIZE_LOG"
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

swift "$KEY_HELPER" "$pid" >"$KEY_LOG" 2>&1
sleep "$(python3 - <<PY
print(${WAIT_AFTER_INPUT_MS}/1000)
PY
)"

swift "$WINDOW_HELPER" >"$WINDOW_LOG"
wid="$(awk -F= '/^id=/{print $2; exit}' "$WINDOW_LOG")"
if [[ -z "$wid" ]]; then
    echo "ERROR: no DOSBox Staging window id found" >&2
    exit 7
fi

screencapture -x -l "$wid" "$SCREEN_0"
sleep "$(python3 - <<PY
print(${WAIT_BETWEEN_CAPTURES_MS}/1000)
PY
)"
screencapture -x -l "$wid" "$SCREEN_1"

ls -lh "$SCREEN_0" "$SCREEN_1" "$LOG" "$KEY_LOG" "$WINDOW_LOG" | tee "$SIZE_LOG"
