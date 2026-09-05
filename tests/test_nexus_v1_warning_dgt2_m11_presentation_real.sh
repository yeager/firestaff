#!/usr/bin/env bash
set -euo pipefail
cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
if [[ -f "$cue" ]]; then exec "$1" "$cue::WARNING.BIN"; fi
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
asset="$root/WARNING.BIN"
expected="8783fa9defda0a358d0474da56480d476b5511c8ca6d3eb61fe097c5697d44ab"
if [[ ! -f "$asset" ]]; then exit 77; fi
[[ "$(wc -c < "$asset" | tr -d '[:space:]')" == "101256" ]]
[[ "$(shasum -a 256 "$asset" | awk '{print $1}')" == "$expected" ]]
exec "$1" "$asset"
