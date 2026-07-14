#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/verify_theron_later_e009_raw_sector_order_trace.sh
work=$(mktemp -d "$repo/.theron-order-trace.XXXXXX")
trap 'rm -rf "$work"' EXIT

cat >"$work/valid.trace" <<'EOF'
source=mednafen-pce-instrumented-coalesced
dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 record_cl=e0 record_dl=04 record_ch=00 variant=us_bin record=4e0
later_system_card_e009_dispatch caller_pc=ea00 return_pc=ea03 sector_count=1 record_cl=10 record_dl=5 record_ch=0 record=510
cd_interface_raw_sector_read lba=1296 bytes=2352 sector_fnv1a=1234abcd span_offset=0 span_bytes=32 span_fnv1a=5678ef90
later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510
EOF

"$script" "$work/valid.trace" us_bin >/dev/null

awk 'NR == 4 { saved = $0; next } NR == 5 { print; print saved; next } { print }' \
    "$work/valid.trace" >"$work/sector-after-return.trace"
if "$script" "$work/sector-after-return.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a raw-sector row after the e009 return\n' >&2
    exit 1
fi

sed 's/source=mednafen-pce-instrumented-coalesced/source=mednafen-pce-instrumented/' \
    "$work/valid.trace" >"$work/unmarked.trace"
if "$script" "$work/unmarked.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a non-coalesced provenance marker\n' >&2
    exit 1
fi

sed 's/sector_fnv1a=1234abcd/sector_fnv1a=1234abcg/' \
    "$work/valid.trace" >"$work/malformed-sector.trace"
if "$script" "$work/malformed-sector.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a malformed raw-sector fingerprint\n' >&2
    exit 1
fi

printf 'PASS: later e009 raw-sector ordering verifier is fail-closed\n'
