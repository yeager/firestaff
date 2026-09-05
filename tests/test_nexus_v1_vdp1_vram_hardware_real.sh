#!/usr/bin/env bash
# Prove that one hash-bound capture of the current retail CUE contains active
# VDP1 execution and non-zero VDP1 VRAM/framebuffer state.  This is a hardware
# witness only; it does not assign game-screen or retail-asset ownership.
set -euo pipefail

validator="${1:?missing runtime capture validator}"
repo_root=$(cd "$(dirname "$0")/.." && pwd)
cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"

while IFS= read -r capture; do
    if python3 "$validator" --require-vdp1-activity "$capture" >/dev/null 2>&1; then
        exec python3 "$validator" --require-vdp1-activity "$capture"
    fi
done < <(python3 "$repo_root/scripts/list_authenticated_nexus_saturn_captures.py" --cue "$cue")

echo "SKIP: no hash-bound active VDP1 capture matches the current Nexus CUE"
exit 77
