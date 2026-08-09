#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
capture=${FIRESTAFF_NEXUS_RUNTIME_CAPTURE:-}
frames=${FIRESTAFF_NEXUS_RUNTIME_CAPTURE_FRAMES:-300}

if [[ -z "$capture" || ! -f "$capture" ]]; then
  echo "nexus VDP1 command-sequence capture: SKIP (set FIRESTAFF_NEXUS_RUNTIME_CAPTURE)"
  exit 77
fi

output=$(PYTHONPATH="$root/scripts${PYTHONPATH:+:$PYTHONPATH}" \
  python3 "$root/scripts/analyze_nexus_vdp1_command_sequence.py" \
  "$capture" --capture-frames "$frames" --summary --require-complete)
grep -F "covered_frames=$frames" <<<"$output" >/dev/null
grep -Fx "semantic_admission=blocked" <<<"$output" >/dev/null
echo "nexus VDP1 command-sequence capture: PASS"
