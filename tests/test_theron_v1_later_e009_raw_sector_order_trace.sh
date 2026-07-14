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
later_system_card_e009_destination_span caller_pc=ea00 return_pc=ea03 record=510 destination=3800 bytes=32 fnv1a=90abcdef
later_system_card_e009_return caller_pc=ea00 return_pc=ea03 record=510
later_system_card_e009_post_return_step caller_pc=ea00 return_pc=ea03 record=510 resume_pc=ea03 next_pc=ea04
EOF

"$script" "$work/valid.trace" us_bin >/dev/null

awk 'NR == 4 { saved = $0; next } NR == 5 { print; print saved; next } { print }' \
    "$work/valid.trace" >"$work/sector-after-return.trace"
if "$script" "$work/sector-after-return.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a raw-sector row after the e009 return\n' >&2
    exit 1
fi

awk 'NR == 5 { saved = $0; next } NR == 6 { print; print saved; next } { print }' \
    "$work/valid.trace" >"$work/destination-after-return.trace"
if "$script" "$work/destination-after-return.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a destination span after the e009 return\n' >&2
    exit 1
fi

grep -v '^later_system_card_e009_destination_span ' "$work/valid.trace" \
    >"$work/missing-destination.trace"
if "$script" "$work/missing-destination.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a missing e009 destination span\n' >&2
    exit 1
fi

grep -v '^later_system_card_e009_post_return_step ' "$work/valid.trace" \
    >"$work/missing-post-return-step.trace"
if "$script" "$work/missing-post-return-step.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a missing e009 post-return step\n' >&2
    exit 1
fi

awk 'NR == 6 { saved = $0; next } NR == 7 { print; print saved; next } { print }' \
    "$work/valid.trace" >"$work/post-return-before-return.trace"
if "$script" "$work/post-return-before-return.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a post-return step before the e009 return\n' >&2
    exit 1
fi

sed 's/next_pc=ea04/next_pc=10000/' "$work/valid.trace" \
    >"$work/invalid-post-return-pc.trace"
if "$script" "$work/invalid-post-return-pc.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted an out-of-range post-return PC\n' >&2
    exit 1
fi

sed 's/resume_pc=ea03/resume_pc=ea04/' "$work/valid.trace" \
    >"$work/mismatched-post-return-target.trace"
if "$script" "$work/mismatched-post-return-target.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a post-return edge from another return target\n' >&2
    exit 1
fi

sed 's/post_return_step caller_pc=ea00/post_return_step caller_pc=ea01/' \
    "$work/valid.trace" >"$work/mismatched-post-return.trace"
if "$script" "$work/mismatched-post-return.trace" us_bin >/dev/null 2>&1; then
    printf 'FAIL: verifier accepted a post-return step from another e009 call\n' >&2
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
