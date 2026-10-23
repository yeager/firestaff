#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_firestaff_debug_perf_metrics.sh <firestaff>}
scratch=$(mktemp -d)
trap 'rm -rf "$scratch"' EXIT
mkdir -p "$scratch/data" "$scratch/config"

output=$(FIRESTAFF_CONFIG_PATH="$scratch/config/config.toml" \
    XDG_CONFIG_HOME="$scratch/config" \
    APPDATA="$scratch/config" \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy \
    "$app" --debug --duration 8000 --data-dir "$scratch/data" 2>&1) || {
    printf '%s\n' "$output" >&2
    exit 1
}

if ! grep -Eq 'firestaff: perf game=menu active=0 .*process-cpu-of-one-core-percent=([0-9]+\.[0-9]+|[0-9]+)' <<<"$output" ||
   ! grep -Eq 'process-rss-bytes=[1-9][0-9]*' <<<"$output" ||
   ! grep -Eq 'process-virtual-bytes=[1-9][0-9]*' <<<"$output" ||
   ! grep -Fq 'target-loop-fps=' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: --debug did not emit CPU, memory, and loop telemetry without game media' >&2
    exit 1
fi

printf '%s\n' 'PASS: --debug emits CPU, RSS, virtual-memory, and loop telemetry without game media'
