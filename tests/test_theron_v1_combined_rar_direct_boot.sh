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

# Finding an extractor executable is not enough: this authentic archive uses
# a RAR compression method that some 7-Zip builds can list but cannot decode.
# Mirror Firestaff's extractor priority and probe one real CUE member before
# treating an unreadable host archive as a product regression.
rar_tool=
for candidate in unrar 7zz 7z bsdtar; do
    if command -v "$candidate" >/dev/null 2>&1; then
        rar_tool=$candidate
        break
    fi
done
if [[ -z "$rar_tool" ]]; then
    printf 'test_theron_v1_combined_rar_direct_boot: SKIP (no RAR extractor installed)\n'
    exit 77
fi
case "$rar_tool" in
    unrar)
        if ! unrar p -inul "$rar" TQUS.cue >/dev/null 2>&1; then
            printf 'test_theron_v1_combined_rar_direct_boot: SKIP (installed unrar cannot decode authentic CUE member)\n'
            exit 77
        fi
        ;;
    bsdtar)
        if ! bsdtar -xOf "$rar" TQUS.cue >/dev/null 2>&1; then
            printf 'test_theron_v1_combined_rar_direct_boot: SKIP (installed bsdtar cannot decode authentic CUE member)\n'
            exit 77
        fi
        ;;
    *)
        if ! "$rar_tool" x -so -- "$rar" TQUS.cue >/dev/null 2>&1; then
            printf 'test_theron_v1_combined_rar_direct_boot: SKIP (installed %s cannot decode authentic CUE member)\n' "$rar_tool"
            exit 77
        fi
        ;;
esac

for region in us jp; do
    if [[ "$region" == us ]]; then
        expected_md5=ceb02343868f80cec899e9b239aff2da
        expected_track02="$rar::@concat(TQUS19.iso,TQUS02End.iso)"
    else
        expected_md5=62a39bbf43415c9739c41c2481080a49
        expected_track02="$rar::@concat(TQJP19.iso,TQJP02End.iso)"
    fi
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
        "$firestaff" --game theron --platform pce --theron-native "$region" \
        --enable-external-archive-tools \
        --data-dir "$rar" \
        --boot-probe --boot-probe-frames 32 \
        --script 'enter,wait4,enter,wait4,enter,wait4' --duration 0 2>&1)
    printf '%s\n' "$output"
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output"
    grep -Fq "assetMd5=$expected_md5" <<<"$output"
    grep -Fq "TQR] Verified Track 02 accepted: $expected_track02" <<<"$output"
    grep -Fq 'theronTrack01CddaReady=1' <<<"$output"
done
printf 'test_theron_v1_combined_rar_direct_boot: PASS (original media, no extraction)\n'
