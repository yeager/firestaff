#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
root=${FIRESTAFF_NEXUS_CAPTURE_ROOT:-$HOME/.firestaff/external/nexus-capture}
track=${FIRESTAFF_NEXUS_TRACK1:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan) (Track 1).bin}
for capture in "$root"/*; do
    [[ -d "$capture" ]] || continue
    if python3 "$repo_root/scripts/verify_nexus_title_mapd_cram_same_session.py" \
            "$capture" "$track" >/dev/null 2>&1; then
        python3 "$repo_root/scripts/verify_nexus_title_mapd_cram_same_session.py" \
            "$capture" "$track"
        echo "Nexus title MAPD palette same-session CRAM boundary: PASS"
        exit 0
    fi
done
echo "SKIP: no hash-bound Nexus MAPD palette RAM-to-CRAM capture"; exit 77
