#!/usr/bin/env bash
set -euo pipefail
cue="${FIRESTAFF_NEXUS_CUE:-$HOME/.firestaff/data/nexus/Dungeon Master Nexus (Japan).cue}"
if [[ -f "$cue" ]]; then exec "$1" "$cue::WARNING.BIN"; fi
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
if [[ ! -f "$cue" ]]; then
    for candidate in "$root"/Dungeon\ Master\ Nexus\ *.cue; do
        if [[ -f "$candidate" ]]; then cue="$candidate"; break; fi
    done
fi
if [[ -f "$cue" ]]; then exec "$1" "$cue::WARNING.BIN"; fi
asset="$root/WARNING.BIN"
expected="8783fa9defda0a358d0474da56480d476b5511c8ca6d3eb61fe097c5697d44ab"
if [[ ! -f "$asset" ]]; then exit 77; fi
[[ "$(wc -c < "$asset" | tr -d '[:space:]')" == "101256" ]]
if command -v sha256sum >/dev/null 2>&1; then
    actual="$(sha256sum "$asset" | awk '{print $1}')"
else
    actual="$(shasum -a 256 "$asset" | awk '{print $1}')"
fi
[[ "$actual" == "$expected" ]]
exec "$1" "$asset"
