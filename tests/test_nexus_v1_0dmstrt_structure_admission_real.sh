#!/usr/bin/env bash
# 0DMSTRT.BIN structure real-data wrapper. Skips with code 77 unless
# the pinned 0DMSTRT.BIN matches the expected byte size and SHA-256.
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <test_nexus_v1_0dmstrt_structure_admission binary>" >&2
    exit 2
fi

root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
asset="$root/0DMSTRT.BIN"
expected_sha256="8a026f155af27cfd43a33b29f7da5b75ee7b09b2c4f016fc3be1ebb4787d20b6"

if [ ! -f "$asset" ]; then
    cue="${FIRESTAFF_NEXUS_CUE:-$root/Dungeon Master Nexus (Japan).cue}"
    if [ -f "$cue" ]; then
        # The binary uses Firestaff's CUE/ISO reader and keeps the member in
        # process memory.  No game-data extraction occurs in this real gate.
        exec "$1" --cue "$cue"
    fi
    echo "SKIP: neither $asset nor $cue is present"
    exit 77
fi

size="$(wc -c < "$asset" | tr -d ' ')"
if [ "$size" != "39516" ]; then
    echo "SKIP: $asset size $size != 39516"
    exit 77
fi

if command -v shasum >/dev/null 2>&1; then
    actual="$(shasum -a 256 "$asset" | awk '{print $1}')"
elif command -v sha256sum >/dev/null 2>&1; then
    actual="$(sha256sum "$asset" | awk '{print $1}')"
else
    echo "SKIP: no sha256 tool available"
    exit 77
fi

if [ "$actual" != "$expected_sha256" ]; then
    echo "SKIP: $asset sha256 mismatch"
    exit 77
fi

exec "$1" "$asset"
