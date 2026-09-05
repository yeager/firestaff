#!/usr/bin/env bash
# An operator-produced raw Saturn capture. This proves hardware state only;
# no source asset is joined or promoted to a drawing route.
set -euo pipefail
if [ "$#" -ne 1 ]; then exit 2; fi
capture="${FIRESTAFF_NEXUS_NBG1_BITMAP_CAPTURE:-}"
if [ -z "$capture" ]; then
    repo_root=$(cd "$(dirname "$0")/.." && pwd)
    cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
    while IFS= read -r candidate; do
        if FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$candidate" \
           FIRESTAFF_NEXUS_REQUIRE_NBG1_BITMAP=1 "$1" >/dev/null 2>&1; then
            capture="$candidate"
            break
        fi
    done < <(python3 "$repo_root/scripts/list_authenticated_nexus_saturn_captures.py" --cue "$cue")
fi
if [ -z "$capture" ] || [ ! -f "$capture" ]; then
    echo "SKIP: NBG1 bitmap capture not present: $capture"
    exit 77
fi
FIRESTAFF_NEXUS_RUNTIME_CAPTURE="$capture" \
FIRESTAFF_NEXUS_REQUIRE_NBG1_BITMAP=1 \
exec "$1"
