#!/usr/bin/env bash
set -euo pipefail
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
for level in $(seq -w 0 15); do [[ -f "$root/LEV${level}.DGN" ]] || exit 77; done
FIRESTAFF_NEXUS_DATA_DIR="$root" exec "$1"
