#!/usr/bin/env bash
set -euo pipefail
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
font="$root/FONT256.S2D"
expected="b820d606b4de4fbaa21d4e32f1df56b4cce6898939fb04f73cb6f55f4ebd13af"
if [[ ! -f "$font" ]]; then exit 77; fi
[[ "$(wc -c < "$font" | tr -d '[:space:]')" == "25012" ]]
[[ "$(shasum -a 256 "$font" | awk '{print $1}')" == "$expected" ]]
exec "$1" "$font"
