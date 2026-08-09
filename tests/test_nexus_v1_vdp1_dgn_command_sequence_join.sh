#!/usr/bin/env bash
set -euo pipefail

root=$(cd "$(dirname "$0")/.." && pwd)
capture=${FIRESTAFF_NEXUS_DGN_SEQUENCE_CAPTURE:-}
data_dir=${FIRESTAFF_NEXUS_DGN_SEQUENCE_DATA_DIR:-}
frames=${FIRESTAFF_NEXUS_DGN_SEQUENCE_CAPTURE_FRAMES:-900}
frame=${FIRESTAFF_NEXUS_DGN_SEQUENCE_FRAME:-899}

if [[ -z "$capture" || -z "$data_dir" || ! -f "$capture" || ! -d "$data_dir" ]]; then
  echo "nexus VDP1 DGN command-sequence join: SKIP (set capture and data dir)"
  exit 77
fi

output=$(PYTHONPATH="$root/scripts${PYTHONPATH:+:$PYTHONPATH}" \
  python3 "$root/scripts/analyze_nexus_vdp1_dgn_command_sequence_join.py" \
  "$capture" --data-dir "$data_dir" --frame "$frame" \
  --capture-frames "$frames")

grep -Fx "frame=$frame chain_records=220" <<<"$output" >/dev/null
grep -Fx "textured_draws=209 source_matches=204 palette_matches=204 face_owner_matches=175" <<<"$output" >/dev/null
grep -Fx "sequence_dgn_material_join=unbound" <<<"$output" >/dev/null
grep -Fx "semantic_admission=blocked" <<<"$output" >/dev/null
echo "nexus VDP1 DGN command-sequence join: PASS (bounded evidence; production remains blocked)"
