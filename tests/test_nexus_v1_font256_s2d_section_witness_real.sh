#!/usr/bin/env bash
set -euo pipefail
root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
font="$root/FONT256.S2D"
expected_canonical="b820d606b4de4fbaa21d4e32f1df56b4cce6898939fb04f73cb6f55f4ebd13af"
expected_english="764a2d6ce11b463817f5c1f2dfefbf55ff9221a1362cb5e4366998100d8ff3bb"
if [[ ! -f "$font" ]]; then exit 77; fi
[[ "$(wc -c < "$font" | tr -d '[:space:]')" == "25012" ]]
actual="$(shasum -a 256 "$font" | awk '{print $1}')"
if [[ "$actual" != "$expected_canonical" && "$actual" != "$expected_english" ]]; then
    echo "FONT256.S2D section witness: SKIP (retail variant SHA-256 $actual is not an admitted retail corpus)"
    exit 77
fi
exec "$1" "$font"
