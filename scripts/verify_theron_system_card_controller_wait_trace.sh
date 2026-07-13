#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 1 || ! -s "$1" ]]; then
    printf 'usage: %s MEDNAFEN_TRACE\n' "$0" >&2
    exit 2
fi

trace=$1
if ! grep -Fqx 'source=mednafen-pce-instrumented' "$trace" ||
   ! grep -Fqx 'post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00' "$trace" ||
   ! grep -Eq '^c860_window_pc=c8c4 .*instruction=LDA \$222D  @ \$222D = \$00( |$)' "$trace"; then
    printf 'FAIL: trace does not prove the bounded System Card controller wait\n' >&2
    exit 1
fi
if grep -Fq 'dynamic_cd_read_transaction ' "$trace" ||
   grep -Fq 'dynamic_cd_read_controller_state ' "$trace" ||
   grep -Fq 'dynamic_huc6260_palette_store ' "$trace"; then
    printf 'FAIL: a dynamic Track 02 receipt must not be classified as a pre-read wait\n' >&2
    exit 1
fi

printf 'PASS: authentic Mednafen trace proves the pre-Track02 System Card controller wait\n'
