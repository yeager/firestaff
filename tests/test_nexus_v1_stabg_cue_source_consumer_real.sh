#!/usr/bin/env bash
# Requires the user-owned Japanese retail CUE/BIN set. No extracted files,
# capture exports or synthetic STMP fixture is accepted by this regression.
set -euo pipefail
if [ "$#" -ne 1 ]; then exit 2; fi
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
cue="$root/Dungeon Master Nexus (Japan).cue"
if [ ! -f "$cue" ]; then
    echo "SKIP: authentic Nexus Japanese CUE not present: $cue"
    exit 77
fi
exec "$1" "$cue"
