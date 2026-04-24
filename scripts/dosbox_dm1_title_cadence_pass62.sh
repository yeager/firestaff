#!/usr/bin/env bash
# Pass 62 — targeted DOSBox TITLE cadence capture harness for DM1 PC 3.4.
#
# This prepares small, reproducible DOSBox Staging configs for the original
# runtime TITLE handoff investigation.  It deliberately does not claim timing
# parity; the configs are evidence-gathering helpers only.

set -euo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_STAGE="${REPO}/verification-screens/dm1-dosbox-capture/DungeonMasterPC34"
OUT_DIR="${REPO}/verification-screens/pass62-dosbox-title-cadence"
DIRECT_STAGE="${OUT_DIR}/stage-vga-direct"
DOSBOX="${DOSBOX:-/Applications/DOSBox Staging.app/Contents/MacOS/dosbox}"

usage() {
    cat <<EOF
Usage: scripts/dosbox_dm1_title_cadence_pass62.sh [--prepare] [--run-pause|--run-vga-arg|--run-redirect-long]

Prepares targeted pass-62 DOSBox configs under:
  ${OUT_DIR}

Modes:
  --prepare          write configs only (default)
  --run-pause        launch DM VGA after an interactive DOS pause
  --run-vga-arg      launch DM VGA directly; confirms selector still blocks
  --run-redirect-long launch DM VGA with a long redirected input file; confirms redirection is not a clean handoff path

Optional:
  DOSBOX=/path/to/dosbox-staging overrides the DOSBox binary.
EOF
}

mode="prepare"
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prepare) mode="prepare"; shift ;;
        --run-pause) mode="run-pause"; shift ;;
        --run-vga-arg) mode="run-vga-arg"; shift ;;
        --run-redirect-long) mode="run-redirect-long"; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "unknown arg: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if [[ ! -f "${SRC_STAGE}/DM.EXE" ]]; then
    echo "ERROR: staged DM1 PC 3.4 tree missing: ${SRC_STAGE}" >&2
    echo "Run scripts/dosbox_dm1_capture.sh first." >&2
    exit 3
fi

mkdir -p "${OUT_DIR}/dosbox-hotkey-capture"
mkdir -p "${DIRECT_STAGE}"
rsync -a "${SRC_STAGE}/" "${DIRECT_STAGE}/"
cp "${DIRECT_STAGE}/VGA" "${DIRECT_STAGE}/VGA.EXE"
python3 - <<PY
from pathlib import Path
stage = Path(r"${DIRECT_STAGE}")
(stage / "KEYS.TXT").write_bytes((b"1\r\n" + b"\r\n" * 20000 + b"1\r\n" * 20000))
PY

write_common_conf() {
    local conf="$1"
    local mount_dir="$2"
    local autoexec_body="$3"
    cat > "$conf" <<EOF
[sdl]
fullscreen=false
output=surface
autolock=false

[dosbox]
machine=vgaonly
captures=${OUT_DIR}/dosbox-hotkey-capture
memsize=4

[cpu]
core=normal
cputype=386
cycles=fixed 3000

[render]
aspect=false
scaler=none

[mixer]
nosound=true

[speaker]
pcspeaker=false
tandy=off
disney=false

[autoexec]
mount c "${mount_dir}"
c:
${autoexec_body}
EOF
}

write_common_conf "${OUT_DIR}/dosbox-title-cadence.conf" "${SRC_STAGE}" $'echo Pass 62 TITLE cadence capture: press a key to launch DM.EXE.\npause\nDM VGA'
write_common_conf "${OUT_DIR}/dosbox-title-vga-arg.conf" "${SRC_STAGE}" "DM VGA"
write_common_conf "${OUT_DIR}/dosbox-title-redirect-long.conf" "${DIRECT_STAGE}" "DM VGA < KEYS.TXT"
write_common_conf "${OUT_DIR}/dosbox-run-vgaexe.conf" "${DIRECT_STAGE}" "VGA.EXE"

echo "[pass-62] wrote configs under ${OUT_DIR}"

run_conf() {
    local conf="$1"
    if [[ ! -x "$DOSBOX" ]]; then
        echo "ERROR: DOSBox binary not executable: $DOSBOX" >&2
        exit 4
    fi
    exec "$DOSBOX" -conf "$conf"
}

case "$mode" in
    prepare) ;;
    run-pause) run_conf "${OUT_DIR}/dosbox-title-cadence.conf" ;;
    run-vga-arg) run_conf "${OUT_DIR}/dosbox-title-vga-arg.conf" ;;
    run-redirect-long) run_conf "${OUT_DIR}/dosbox-title-redirect-long.conf" ;;
esac
