#!/usr/bin/env bash
set -euo pipefail

firestaff=${1:?usage: test_theron_v1_combined_rar_direct_boot.sh FIRESTAFF}
rar=${FIRESTAFF_THERON_RAR:-}
if [[ -z "$rar" ]]; then
    rar="$HOME/.firestaff/data/theron/Theron's Quest for PC-Engine (US and Japanese versions).rar"
fi
if [[ ! -f "$rar" ]]; then
    printf 'test_theron_v1_combined_rar_direct_boot: SKIP (authentic RAR not found)\n'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$firestaff" --game theron --platform pce \
    --enable-external-archive-tools \
    --data-dir "$rar" \
    --boot-probe --boot-probe-frames 32 \
    --script 'enter,wait4,enter,wait4,enter,wait4' --duration 0 2>&1)
printf '%s\n' "$output"
grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output"
grep -Fq 'assetMd5=ceb02343868f80cec899e9b239aff2da' <<<"$output"
grep -Fq "TQR] Verified Track 02 accepted: $rar::@concat(TQUS19.iso,TQUS02End.iso)" <<<"$output"
printf 'test_theron_v1_combined_rar_direct_boot: PASS (original media, no extraction)\n'
