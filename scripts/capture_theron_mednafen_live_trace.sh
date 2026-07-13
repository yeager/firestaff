#!/usr/bin/env bash
set -euo pipefail

mednafen_bin=${MEDNAFEN_BIN:-}
cue=${THERON_US_CUE:-}
system_card=${THERON_SYSTEM_CARD:-}
trace=${THERON_LIVE_TRACE_OUTPUT:-}
seconds=${THERON_CAPTURE_SECONDS:-45}
capture_sdl_video_driver=${THERON_CAPTURE_SDL_VIDEODRIVER:-dummy}
configured_home=${THERON_MEDNAFEN_HOME:-}

if [[ -z "$mednafen_bin" || -z "$cue" || -z "$system_card" || -z "$trace" ]]; then
    printf '%s\n' 'SKIP: MEDNAFEN_BIN, THERON_US_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required'
    exit 0
fi
if [[ ! -x "$mednafen_bin" || ! -f "$cue" || ! -f "$system_card" ]]; then
    printf '%s\n' 'FAIL: Mednafen, US CUE, or System Card path is unavailable' >&2
    exit 1
fi
if [[ ! "$seconds" =~ ^[1-9][0-9]*$ ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_SECONDS must be a positive integer' >&2
    exit 1
fi
if [[ -n "$configured_home" && ! -d "$configured_home" ]]; then
    printf '%s\n' 'FAIL: THERON_MEDNAFEN_HOME must name an existing Mednafen configuration directory' >&2
    exit 1
fi

if command -v gtimeout >/dev/null 2>&1; then
    timeout_command=(gtimeout "$seconds")
elif command -v timeout >/dev/null 2>&1; then
    timeout_command=(timeout "$seconds")
else
    printf '%s\n' 'FAIL: timeout or gtimeout is required for bounded live capture' >&2
    exit 1
fi

trace_dir=$(dirname -- "$trace")
memory_trace="${trace}.memory"
cd_trace="${trace}.cd"
input_trace="${trace}.input"
stdout_file="$trace_dir/$(basename -- "$trace").stdout"
stderr_file="$trace_dir/$(basename -- "$trace").stderr"

mkdir -p "$trace_dir"
rm -f "$trace" "$memory_trace" "$cd_trace" "$input_trace"
if [[ -n "$configured_home" ]]; then
    home_dir=$configured_home
    cleanup_home=0
else
    home_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-mednafen.XXXXXX")
    cleanup_home=1
fi
if [[ "$cleanup_home" == 1 ]]; then
    trap 'rm -rf "$home_dir"' EXIT
fi

set +e
"${timeout_command[@]}" env \
    MEDNAFEN_HOME="$home_dir" \
    FIRESTAFF_THERON_IRQ2_TRACE="$trace" \
    FIRESTAFF_THERON_IRQ2_MEMORY_TRACE="$memory_trace" \
    FIRESTAFF_THERON_IRQ2_CD_TRACE="$cd_trace" \
    FIRESTAFF_THERON_IRQ2_INPUT_TRACE="$input_trace" \
    SDL_VIDEODRIVER="$capture_sdl_video_driver" \
    SDL_AUDIODRIVER=dummy \
    "$mednafen_bin" \
    -sound 0 \
    -video.driver softfb \
    -pce.arcadecard 0 \
    -pce.cdbios "$system_card" \
    "$cue" >"$stdout_file" 2>"$stderr_file"
status=$?
set -e

if [[ ! -s "$trace" ]] || ! grep -Fqx 'source=mednafen-pce-instrumented' "$trace"; then
    printf '%s\n' 'FAIL: Mednafen did not produce a provenance-marked live trace' >&2
    exit 1
fi
if ! grep -Fq 'dynamic_cd_read_transaction ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_controller_state ' "$trace" ||
   ! grep -Fq 'dynamic_huc6260_palette_store ' "$trace"; then
    if grep -Fqx 'post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00' "$trace" &&
       grep -Eq '^c860_window_pc=c8c4 .*instruction=LDA \$222D  @ \$222D = \$00( |$)' "$trace" &&
       grep -Eq '^c860_window_pc=c8c7 .*instruction=CMP #\$08' "$trace" &&
       grep -Eq '^c860_window_pc=c8cb .*instruction=CMP #\$04' "$trace" &&
       grep -Eq '^c860_window_pc=c8cd .*instruction=BNE \$C897' "$trace"; then
        printf 'BLOCKED: authentic System Card controller wait observed before a Track 02 transaction (exit=%s)\n' "$status"
        exit 1
    fi
    printf 'BLOCKED: live trace ended without combined CD/controller/HuC6260 store receipts (exit=%s)\n' "$status"
    exit 1
fi

printf 'PASS: live trace contains combined CD/controller/HuC6260 store receipts (exit=%s)\n' "$status"
