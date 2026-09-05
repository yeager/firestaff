#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
while IFS= read -r raw; do
    manifest="${raw%/*}/manifest.txt"
    [[ -s "$manifest" ]] || continue
    if receipt=$(python3 "$repo_root/scripts/analyze_nexus_saturn_hardware_capture.py" "$raw" "$manifest"); then
        grep -Fq '"schema": "FIRESTAFF_NEXUS_AUTHENTIC_HARDWARE_RECEIPT_V1"' <<<"$receipt"
        grep -Fq '"asset_semantics": "unassigned"' <<<"$receipt"
        echo "$receipt"; echo "Nexus authenticated VDP1/VDP2 hardware capture: PASS"; exit 0
    fi
done < <(python3 "$repo_root/scripts/list_authenticated_nexus_saturn_captures.py" --cue "$cue")
echo "SKIP: no authenticated capture contains complete VDP1/VDP2 hardware domains"; exit 77
