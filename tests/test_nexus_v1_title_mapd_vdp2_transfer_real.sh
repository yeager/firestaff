#!/usr/bin/env bash
set -euo pipefail
repo_root=$(cd "$(dirname "$0")/.." && pwd)
root=${FIRESTAFF_NEXUS_CAPTURE_ROOT:-$HOME/.firestaff/external/nexus-capture}
track=${FIRESTAFF_NEXUS_TRACK1:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan) (Track 1).bin}
for capture in "$root"/*; do
    [[ -d "$capture" ]] || continue
    if python3 "$repo_root/scripts/verify_nexus_title_mapd_vdp2_transfer.py" \
            "$capture" "$track" >/dev/null 2>&1; then
        python3 "$repo_root/scripts/verify_nexus_title_mapd_vdp2_transfer.py" \
            "$capture" "$track"
        echo "Nexus title MAPD VDP2 transfer: PASS"
        exit 0
    fi
done
echo "SKIP: no hash-bound Nexus MAPD-to-VDP2 transfer capture"; exit 77
