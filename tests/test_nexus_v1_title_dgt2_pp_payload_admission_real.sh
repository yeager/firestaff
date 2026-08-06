#!/usr/bin/env bash
# TITLE.BIN DGT2 payload real-data wrapper. Skips with code 77 unless
# the pinned TITLE.BIN matches the expected byte size and SHA-256.
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: $0 <test_nexus_v1_title_dgt2_pp_payload_admission binary>" >&2
    exit 2
fi

root="${FIRESTAFF_NEXUS_DATA_DIR:-$HOME/.firestaff/data/nexus}"
asset="$root/TITLE.BIN"
expected_sha256_canonical="51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
expected_sha256_english="a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"

if [ ! -f "$asset" ]; then
    echo "SKIP: $asset not present"
    exit 77
fi

size="$(wc -c < "$asset" | tr -d ' ')"
if [ "$size" != "112216" ]; then
    echo "SKIP: $asset size $size != 112216"
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

if [ "$actual" != "$expected_sha256_canonical" ] &&
   [ "$actual" != "$expected_sha256_english" ]; then
    echo "SKIP: $asset sha256 mismatch"
    exit 77
fi

FIRESTAFF_NEXUS_TITLE_BIN_SHA256="$actual" exec "$1" "$asset"
