#!/usr/bin/env bash
set -euo pipefail

mednafen_bin=${MEDNAFEN_BIN:-}
cue=${THERON_US_CUE:-}
system_card=${THERON_SYSTEM_CARD:-}
trace=${THERON_LIVE_TRACE_OUTPUT:-}
seconds=${THERON_CAPTURE_SECONDS:-45}
capture_sdl_video_driver=${THERON_CAPTURE_SDL_VIDEODRIVER:-dummy}
configured_home=${THERON_MEDNAFEN_HOME:-}
host_key=${THERON_CAPTURE_HOST_KEY:-}
# The authentic System Card 3.0 title needs to become interactive before RUN.
# A real macOS capture shows that 8 seconds reaches the title, while 3 seconds
# merely sends the key during BIOS initialization.
host_key_delay=${THERON_CAPTURE_HOST_KEY_DELAY:-8}
host_key_hold=${THERON_CAPTURE_HOST_KEY_HOLD:-1}
host_key_repeats=${THERON_CAPTURE_HOST_KEY_REPEATS:-3}

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
if [[ -n "$host_key" ]]; then
    if [[ "$host_key" != return && "$host_key" != i && "$host_key" != select ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY currently supports only return, i, or select' >&2
        exit 1
    fi
    if [[ "$capture_sdl_video_driver" == dummy ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY requires a non-dummy SDL video driver' >&2
        exit 1
    fi
    if [[ "$(uname -s)" != Darwin ]] || ! command -v osascript >/dev/null 2>&1; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY=return requires macOS osascript accessibility input' >&2
        exit 1
    fi
    if [[ ! "$host_key_delay" =~ ^[0-9]+$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_DELAY must be a non-negative integer' >&2
        exit 1
    fi
    if [[ ! "$host_key_hold" =~ ^[1-9][0-9]*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_HOLD must be a positive integer' >&2
        exit 1
    fi
    if [[ ! "$host_key_repeats" =~ ^[1-9][0-9]*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_REPEATS must be a positive integer' >&2
        exit 1
    fi
fi

if command -v gtimeout >/dev/null 2>&1; then
    timeout_command=(gtimeout "$seconds")
elif command -v timeout >/dev/null 2>&1; then
    timeout_command=(timeout "$seconds")
else
    printf '%s\n' 'FAIL: timeout or gtimeout is required for bounded live capture' >&2
    exit 1
fi

trace_count() {
    local pattern=$1
    local file=$2
    local count

    count=$(grep -Ec "$pattern" "$file" 2>/dev/null || true)
    printf '%s' "${count:-0}"
}

trace_input_order_receipt() {
    local file=$1

    awk '
        /^pce_input_(read|write) / {
            input_total++
            if (first_host_line == 0) input_before_first_host++
            else input_after_first_host++
        }
        /^host_key_event / && first_host_line == 0 {
            first_host_line = NR
            input_before_first_host = input_total
        }
        END {
            if (first_host_line == 0) {
                printf "first_host_key_input_trace_line=0\n"
                printf "pce_input_transactions_before_first_host=%d\n", input_total
                printf "pce_input_transactions_after_first_host=0\n"
                printf "host_input_order=no_host_key_observed\n"
            } else {
                printf "first_host_key_input_trace_line=%d\n", first_host_line
                printf "pce_input_transactions_before_first_host=%d\n", input_before_first_host
                printf "pce_input_transactions_after_first_host=%d\n", input_after_first_host
                if (input_after_first_host > 0)
                    printf "host_input_order=followed_by_pce_input_poll\n"
                else
                    printf "host_input_order=after_last_observed_pce_input_poll\n"
            }
        }
    ' "$file"
}

trace_dir=$(dirname -- "$trace")
memory_trace="${trace}.memory"
cd_trace="${trace}.cd"
input_trace="${trace}.input"
transition_receipt="${trace}.transition"
stdout_file="$trace_dir/$(basename -- "$trace").stdout"
stderr_file="$trace_dir/$(basename -- "$trace").stderr"

mkdir -p "$trace_dir"
rm -f "$trace" "$memory_trace" "$cd_trace" "$input_trace" "$transition_receipt"
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

launch=(
    "${timeout_command[@]}" env
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
    -pce.cdbios "$system_card"
    "$cue"
)
set +e
"${launch[@]}" >"$stdout_file" 2>"$stderr_file" &
mednafen_pid=$!
if [[ -n "$host_key" ]]; then
    sleep "$host_key_delay"
    if ! osascript <<APPLESCRIPT
tell application "System Events"
    set targetProcess to first application process whose name is "mednafen"
    set frontmost of targetProcess to true
    delay 0.2
    if "$host_key" is "return" then
        key down 36
        delay $host_key_hold
        key up 36
    else if "$host_key" is "select" then
        key down 48
        delay $host_key_hold
        key up 48
    else
        repeat $host_key_repeats times
            key code 85
            delay 0.2
        end repeat
    end if
end tell
APPLESCRIPT
    then
        kill "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
        printf '%s\n' 'FAIL: macOS could not focus Mednafen and send Return; grant accessibility permission to the invoking terminal' >&2
        exit 1
    fi
fi
wait "$mednafen_pid"
status=$?
set -e

if [[ ! -s "$trace" ]] || ! grep -Fqx 'source=mednafen-pce-instrumented' "$trace"; then
    printf '%s\n' 'FAIL: Mednafen did not produce a provenance-marked live trace' >&2
    exit 1
fi
transition_input_count=$(trace_count '^pce_input_(read|write) ' "$input_trace")
transition_host_key_count=$(trace_count '^host_key_event ' "$input_trace")
transition_irq_count=$(trace_count '^pce_cd_irq cpu_pc=' "$cd_trace")
transition_non_system_card_count=$(trace_count '^pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3} ' "$cd_trace")
transition_sector_count=$(trace_count '^cd_interface_raw_sector_read ' "$cd_trace")
{
    printf '%s\n' 'source=authentic-mednafen-transition-receipt'
    printf 'input_transactions=%s\n' "$transition_input_count"
    printf 'host_key_events=%s\n' "$transition_host_key_count"
    printf 'cd_irq_callbacks=%s\n' "$transition_irq_count"
    printf 'non_system_card_pcecd_reads=%s\n' "$transition_non_system_card_count"
    printf 'raw_sector_spans=%s\n' "$transition_sector_count"
    trace_input_order_receipt "$input_trace"
    if [[ -n "$host_key" ]]; then
        printf 'requested_host_key=%s\n' "$host_key"
        printf 'requested_host_key_hold_seconds=%s\n' "$host_key_hold"
        printf 'requested_host_key_repeats=%s\n' "$host_key_repeats"
    fi
    if [[ "$transition_input_count" -gt 0 && "$transition_irq_count" -gt 0 &&
          "$transition_non_system_card_count" -gt 0 && "$transition_sector_count" -gt 0 ]]; then
        printf '%s\n' 'transition=observed'
    else
        printf '%s\n' 'transition=missing'
    fi
} >"$transition_receipt"
if [[ -n "$host_key" && "$transition_host_key_count" -eq 0 ]]; then
    printf 'BLOCKED: requested host key was not observed by Mednafen SDL dispatch (exit=%s)\n' "$status"
    exit 1
fi
if ! grep -Fq 'dynamic_cd_read_transaction ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_controller_state ' "$trace" ||
   ! grep -Fq 'dynamic_huc6260_palette_store ' "$trace"; then
    if [[ "$transition_sector_count" -gt 0 ]]; then
        printf 'BLOCKED: loader reached authentic raw sectors but dynamic CPU receipts are absent; host_keys=%s input=%s irq=%s non_system_card_pcecd=%s raw_sectors=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$transition_irq_count" "$transition_non_system_card_count" "$transition_sector_count" "$status"
        exit 1
    fi
    if grep -Fqx 'post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00' "$trace" &&
       grep -Eq '^c860_window_pc=c8c4 .*instruction=LDA \$222D  @ \$222D = \$00( |$)' "$trace" &&
       grep -Eq '^c860_window_pc=c8c7 .*instruction=CMP #\$08' "$trace" &&
       grep -Eq '^c860_window_pc=c8cb .*instruction=CMP #\$04' "$trace" &&
       grep -Eq '^c860_window_pc=c8cd .*instruction=BNE \$C897' "$trace"; then
        printf 'BLOCKED: System Card wait; host_keys=%s input=%s input_after_first_host=%s irq=%s non_system_card_pcecd=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$(awk -F= '/^pce_input_transactions_after_first_host=/{print $2}' "$transition_receipt")" "$transition_irq_count" "$transition_non_system_card_count" "$status"
        exit 1
    fi
    printf 'BLOCKED: dynamic receipts absent; host_keys=%s input=%s irq=%s non_system_card_pcecd=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$transition_irq_count" "$transition_non_system_card_count" "$status"
    exit 1
fi
if ! grep -Eq '^cd_interface_raw_sector_read lba=[0-9]+ bytes=2352 span_offset=0 span_bytes=32 span_fnv1a=[0-9a-f]{8}$' "$cd_trace"; then
    printf 'BLOCKED: dynamic CPU receipts lack a bounded authentic raw-sector span (exit=%s)\n' "$status"
    exit 1
fi
if ! awk '
    /^pce_input_(read|write) / { saw_input = 1 }
    /^pce_cd_irq cpu_pc=/ { saw_irq = 1 }
    /^pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3} / { saw_non_system_card = 1 }
    /^cd_interface_raw_sector_read / {
        reached_sector = 1
        passed = saw_input && saw_irq && saw_non_system_card
        exit
    }
    END { exit !(reached_sector && passed) }
' "$cd_trace"; then
    printf 'BLOCKED: raw sector span lacks prior input, CDIRQ, and non-System-Card PCECD caller receipts (exit=%s)\n' "$status"
    exit 1
fi

printf 'PASS: live trace contains combined CD/controller/HuC6260 and raw-sector span receipts (exit=%s)\n' "$status"
