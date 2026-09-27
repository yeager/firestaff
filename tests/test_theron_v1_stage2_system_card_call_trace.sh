#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
verify=$repo/scripts/verify_theron_stage2_system_card_call_trace.sh
scratch_root=${FIRESTAFF_TEST_SCRATCH_ROOT:-"$repo/build/test-scratch"}
mkdir -p "$scratch_root"
trace=$(mktemp "$scratch_root/firestaff-theron-stage2-call.XXXXXX")
trap 'rm -f "$trace"' EXIT

cat >"$trace" <<'EOF'
stage2_system_card_call pc=40cd return_pc=40d0 target=e009 a=01 x=03 y=03 p=00 mpr0=ff table=00e30302 fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff
stage2_system_card_return pc=40d0 call_pc=40cd a=00 x=01 y=ff p=03 mpr0=ff fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff
stage2_system_card_call pc=40a4 return_pc=40a7 target=e00f a=01 x=03 y=ff p=01 mpr0=ff table=00e70311 fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff
stage2_system_card_return pc=40a7 call_pc=40a4 a=78 x=00 y=03 p=00 mpr0=ff fc=00 physical_fc=00 fd=00 fe=ff f8=00 fa=ff fb=ff ff=ff
EOF

"$verify" "$trace"
sed -i.bak 's/y=03 p=00 mpr0=ff table=00e30302/y=99 p=00 mpr0=ff table=00e30302/' "$trace"
rm -f "$trace.bak"
"$verify" "$trace"
sed -i.bak 's/y=99 p=00 mpr0=ff table=00e30302/y=98 p=00 mpr0=ff table=00e30302/' "$trace"
rm -f "$trace.bak"
if "$verify" "$trace"; then
    printf 'FAIL: an unobserved Y register value was accepted\n' >&2
    exit 1
fi
sed -i.bak 's/y=98 p=00 mpr0=ff table=00e30302/y=99 p=00 mpr0=ff table=00e30302/' "$trace"
rm -f "$trace.bak"
authentic_trace=${THERON_STAGE2_SYSTEM_CARD_TRACE:-}
if [[ -n "$authentic_trace" ]]; then
    if [[ ! -f "$authentic_trace" ]]; then
        printf 'FAIL: configured authentic Theron trace is unavailable: %s\n' \
            "$authentic_trace" >&2
        exit 1
    fi
else
    authentic_trace="${HOME:-}/.firestaff/firestaff-probe-screenshots/theron-live-20260727-172740.trace"
fi
if [[ -f "$authentic_trace" ]]; then
    "$verify" "$authentic_trace"
else
    printf '%s\n' 'SKIP: no authentic Theron System Card trace is available'
fi
sed -i.bak 's/table=00e70311/table=00e70310/' "$trace"
rm -f "$trace.bak"
if "$verify" "$trace"; then
    printf 'FAIL: altered stage-two table was accepted\n' >&2
    exit 1
fi

printf 'PASS: stage-two System Card trace verifier is fail-closed\n'
