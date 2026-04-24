#!/usr/bin/env bash
# Pass 63 — DOSBox Staging selector-input automation for DM1 PC 3.4 TITLE capture.
#
# This wrapper uses the already-installed macOS `cliclick` utility to type the
# original DM PC 3.4 setup selections into DOSBox Staging.  It is intentionally
# small and reproducible: the DOSBox config stays text-only, input is host-key
# injection, and captured screenshots are kept under verification-screens/ and
# should not be committed unless explicitly curated.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_STAGE="${REPO}/verification-screens/dm1-dosbox-capture/DungeonMasterPC34"
OUT_DIR="${REPO}/verification-screens/pass63-dosbox-title-automation"
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"
CLICLICK="${CLICLICK:-cliclick}"
SCREENSHOT="${SCREENSHOT:-screencapture}"
WAIT_BEFORE_INPUT_MS="${WAIT_BEFORE_INPUT_MS:-3000}"
WAIT_AFTER_INPUT_MS="${WAIT_AFTER_INPUT_MS:-7000}"
CONF="${OUT_DIR}/dosbox-title-input.conf"
LOG="${OUT_DIR}/dosbox-title-input.log"
KEY_LOG="${OUT_DIR}/cliclick-title-input.log"
SCREEN_OUT="${OUT_DIR}/title-after-selector.png"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_title_input_pass63.sh [--prepare|--dry-run|--run]

Modes:
  --prepare   write the DOSBox config only (default)
  --dry-run   write config and print the exact cliclick sequence, no DOSBox launch
  --run       launch DOSBox, type the setup selector sequence, and save a host screenshot

Environment overrides:
  DOSBOX=/path/to/dosbox-staging
  CLICLICK=/path/to/cliclick
  WAIT_BEFORE_INPUT_MS=3000
  WAIT_AFTER_INPUT_MS=7000

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
cpu_cycles=fixed 3000

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

echo "[pass-63] wrote ${CONF}"

sequence=(t:1 kp:return t:1 kp:return t:1 kp:return)

case "$mode" in
    prepare)
        exit 0
        ;;
    dry-run)
        printf '[pass-63] cliclick sequence:'
        printf ' %q' "${sequence[@]}"
        printf '\n'
        exit 0
        ;;
    run)
        ;;
esac

if [[ ! -x "$DOSBOX" ]]; then
    echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
    exit 4
fi
if ! command -v "$CLICLICK" >/dev/null 2>&1; then
    echo "ERROR: cliclick not found; install or set CLICLICK=/path/to/cliclick" >&2
    exit 5
fi
if ! command -v "$SCREENSHOT" >/dev/null 2>&1; then
    echo "ERROR: screencapture not found; set SCREENSHOT=/path/to/screencapture" >&2
    exit 6
fi

rm -f "$LOG" "$KEY_LOG" "$SCREEN_OUT"
"$DOSBOX" -conf "$CONF" >"$LOG" 2>&1 &
pid=$!
echo "$pid" > "${OUT_DIR}/dosbox.pid"

cleanup() {
    osascript -e 'tell application "DOSBox Staging" to quit' >/dev/null 2>&1 || true
    kill "$pid" >/dev/null 2>&1 || true
}
trap cleanup EXIT

sleep "$(python3 - <<PY
print(${WAIT_BEFORE_INPUT_MS}/1000)
PY
)"
open -a 'DOSBox Staging' >/dev/null 2>&1 || true
"$CLICLICK" -m verbose -w 120 "${sequence[@]}" >"$KEY_LOG" 2>&1
sleep "$(python3 - <<PY
print(${WAIT_AFTER_INPUT_MS}/1000)
PY
)"
"$SCREENSHOT" -x "$SCREEN_OUT"
ls -lh "$SCREEN_OUT"
