#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    printf 'usage: %s <firestaff-binary> <authentic-archive> <edition-label>\n' "$0" >&2
    exit 2
fi

app=$1
archive=$2
edition=$3
if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf 'SKIP: authentic Atari ST %s media or Firestaff binary is unavailable\n' "$edition"
    exit 77
fi

scratch=${FIRESTAFF_TEST_SCRATCH:-"$PWD/.codex-scratch"}
mkdir -p "$scratch"
probe_root=$(mktemp -d "$scratch/dm1-atari-first-hall.XXXXXX")
trap 'rm -rf "$probe_root"' EXIT
mkdir -p "$probe_root/home"

# ReDMCSB STARTUP1.C:162-174 runs F0441, retries F0435 and calls F0462. A
# fresh game places the party via MOVESENS.C F0267 from the off-square PARTY
# sentinel before presenting the first Hall frame.
HOME="$probe_root/home" FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON="$probe_root/runtime.json" \
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" \
    --script 'enter,enter,enter,wait30,enter,wait60,enter' \
    --duration 15000 >/dev/null 2>&1

python3 - "$probe_root/runtime.json" "$edition" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as probe_file:
    probe = json.load(probe_file)
startup = probe["startup"]
party = probe["party"]
if (probe["launchedEver"] != 1 or probe["active"] != 1 or
        probe["sourceId"] != "dm1" or startup["receiptReady"] != 1 or
        startup["phase"] != "dm1-runtime" or startup["levelLoaded"] != 1 or
        startup["startupActive"] != 0 or
        startup["dm1StartupHandoffExecuted"] != 1 or
        startup["dm1StartupHoCFirstFrameReady"] != 1 or
        (party["mapIndex"], party["mapX"], party["mapY"],
         party["direction"], party["championCount"]) != (0, 1, 3, 2, 0)):
    raise SystemExit(f"FAIL: authentic Atari ST {sys.argv[2]} missed its fresh-game party handoff: {probe}")
print(f"PASS: authentic DM1 Atari ST {sys.argv[2]} first Hall frame confirms F0267 party placement")
PY
