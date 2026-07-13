#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/verify_theron_system_card_controller_wait_trace.sh
trace=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-controller-wait.XXXXXX")
trap 'rm -f "$trace"' EXIT

cat >"$trace" <<'EOF'
source=mednafen-pce-instrumented
post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00
c860_window_pc=c8c4 physical_pc=0000c8c4 instruction=LDA $222D  @ $222D = $00
c860_window_pc=c8c7 physical_pc=0000c8c7 instruction=CMP #$08
c860_window_pc=c8cb physical_pc=0000c8cb instruction=CMP #$04
c860_window_pc=c8cd physical_pc=0000c8cd instruction=BNE $C897
EOF
"$script" "$trace"

printf '%s\n' 'dynamic_cd_read_transaction pc=4090' >>"$trace"
if "$script" "$trace"; then
    printf 'FAIL: dynamic Track 02 receipt was accepted as a controller wait\n' >&2
    exit 1
fi

printf 'PASS: controller-wait verifier remains fail-closed\n'
