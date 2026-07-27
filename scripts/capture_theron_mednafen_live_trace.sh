#!/usr/bin/env bash
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mednafen_bin=${MEDNAFEN_BIN:-}
cue=${THERON_US_CUE:-}
system_card=${THERON_SYSTEM_CARD:-}
trace=${THERON_LIVE_TRACE_OUTPUT:-}
seconds=${THERON_CAPTURE_SECONDS:-45}
capture_startup_grace=${THERON_CAPTURE_STARTUP_GRACE:-30}
capture_sdl_video_driver=${THERON_CAPTURE_SDL_VIDEODRIVER:-dummy}
configured_home=${THERON_MEDNAFEN_HOME:-}
host_key=${THERON_CAPTURE_HOST_KEY:-}
# The authentic System Card 3.0 title needs to become interactive before RUN.
# A real macOS capture shows that 8 seconds reaches the title, while 3 seconds
# merely sends the key during BIOS initialization.
host_key_delay=${THERON_CAPTURE_HOST_KEY_DELAY:-8}
host_key_hold=${THERON_CAPTURE_HOST_KEY_HOLD:-1}
host_key_repeats=${THERON_CAPTURE_HOST_KEY_REPEATS:-3}
host_key_delays=${THERON_CAPTURE_HOST_KEY_DELAYS:-}
host_key_sequence=${THERON_CAPTURE_HOST_KEY_SEQUENCE:-}
input_route=${THERON_CAPTURE_INPUT_ROUTE:-pid}
host_focus_x=${THERON_CAPTURE_FOCUS_X:-960}
host_focus_y=${THERON_CAPTURE_FOCUS_Y:-540}
host_key_code=
host_input_requested=0
if [[ -n "$host_key" || -n "$host_key_sequence" ]]; then
    host_input_requested=1
fi
quartz_keypair_script="$script_dir/send_theron_macos_quartz_keypair.swift"

if [[ -z "$mednafen_bin" || -z "$cue" || -z "$system_card" || -z "$trace" ]]; then
    printf '%s\n' 'SKIP: MEDNAFEN_BIN, THERON_US_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required'
    exit 0
fi
if [[ ! -x "$mednafen_bin" || ! -f "$cue" || ! -f "$system_card" ]]; then
    printf '%s\n' 'FAIL: Mednafen, US CUE, or System Card path is unavailable' >&2
    exit 1
fi

md5_file() {
    if command -v md5 >/dev/null 2>&1; then
        md5 -q "$1"
    elif command -v md5sum >/dev/null 2>&1; then
        md5sum "$1" | awk '{print $1}'
    else
        return 1
    fi
}

require_capture_profile_mapping() {
    local key=$1
    local expected_scancode=$2
    local label=$3
    local actual_scancode

    actual_scancode=$(awk -v key="$key" '
        $1 == key && $2 == "keyboard" && $3 == "0x0" { print $4; exit }
    ' "$configured_home/mednafen.cfg")
    if [[ "$actual_scancode" != "$expected_scancode" ]]; then
        printf 'FAIL: THERON_MEDNAFEN_HOME does not retain the required %s mapping (expected SDL scancode %s, got %s)\n' \
            "$label" "$expected_scancode" "${actual_scancode:-missing}" >&2
        exit 1
    fi
}

require_capture_profile_mappings() {
    if [[ ! -f "$configured_home/mednafen.cfg" ]]; then
        printf '%s\n' 'FAIL: THERON_MEDNAFEN_HOME has no Mednafen configuration file' >&2
        exit 1
    fi

    # The Quartz codes below are only valid for this explicit physical-key
    # profile. Refuse profile drift instead of silently sending a non-PCE key.
    require_capture_profile_mapping pce.input.port1.gamepad.run 40 RUN
    require_capture_profile_mapping pce.input.port1.gamepad.select 43 SELECT
    require_capture_profile_mapping pce.input.port1.gamepad.i 12 I
    require_capture_profile_mapping pce.input.port1.gamepad.ii 90 II
    require_capture_profile_mapping pce.input.port1.gamepad.up 26 UP
    require_capture_profile_mapping pce.input.port1.gamepad.down 22 DOWN
    require_capture_profile_mapping pce.input.port1.gamepad.left 4 LEFT
    require_capture_profile_mapping pce.input.port1.gamepad.right 7 RIGHT
}

track02_member=$(awk '
    /^FILE "/ {
        line = $0
        sub(/^FILE "/, "", line)
        sub(/" BINARY[[:space:]]*$/, "", line)
        file = line
        next
    }
    /^[[:space:]]*TRACK[[:space:]]+02[[:space:]]+MODE1\/2352[[:space:]]*$/ {
        print file
        exit
    }
' "$cue")
if [[ -z "$track02_member" || "$track02_member" == */* || "$track02_member" == *\\* ]]; then
    printf '%s\n' 'FAIL: CUE has no safe TRACK 02 MODE1/2352 member' >&2
    exit 1
fi
track02_path="$(dirname -- "$cue")/$track02_member"
if [[ ! -f "$track02_path" ]]; then
    printf '%s\n' 'FAIL: CUE TRACK 02 payload is unavailable' >&2
    exit 1
fi
system_card_md5=$(md5_file "$system_card") || {
    printf '%s\n' 'FAIL: md5 or md5sum is required for authentic media capture' >&2
    exit 1
}
track02_md5=$(md5_file "$track02_path") || {
    printf '%s\n' 'FAIL: could not hash CUE TRACK 02 payload' >&2
    exit 1
}
mednafen_binary_md5=$(md5_file "$mednafen_bin") || {
    printf '%s\n' 'FAIL: could not hash the instrumented Mednafen binary' >&2
    exit 1
}
if [[ "$system_card_md5" != ff1a674273fe3540ccef576376407d1d ]]; then
    printf '%s\n' 'FAIL: System Card 3.0 MD5 mismatch' >&2
    exit 1
fi
case "$track02_md5" in
    b7afb338ad31be1025b53f9aff12d73a|f23601102138f87c33025877767ebf76) ;;
    *)
        printf '%s\n' 'FAIL: CUE TRACK 02 is not an authenticated Theron JP/US raw BIN' >&2
        exit 1
        ;;
esac
if [[ ! "$seconds" =~ ^[1-9][0-9]*$ ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_SECONDS must be a positive integer' >&2
    exit 1
fi
if [[ ! "$capture_startup_grace" =~ ^[1-9][0-9]*$ ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_STARTUP_GRACE must be a positive integer' >&2
    exit 1
fi
if [[ -n "$configured_home" && ! -d "$configured_home" ]]; then
    printf '%s\n' 'FAIL: THERON_MEDNAFEN_HOME must name an existing Mednafen configuration directory' >&2
    exit 1
fi
if [[ "$host_input_requested" == 1 ]]; then
    if [[ -z "$configured_home" ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY requires THERON_MEDNAFEN_HOME with an explicit PCE input mapping' >&2
        exit 1
    fi
    require_capture_profile_mappings
    if [[ -n "$host_key_sequence" && ( -n "$host_key" || -n "$host_key_delays" ) ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_SEQUENCE cannot be combined with HOST_KEY or HOST_KEY_DELAYS' >&2
        exit 1
    fi
    if [[ -n "$host_key_sequence" && ! "$host_key_sequence" =~ ^(return|ii|i|select|up|down|left|right)@[0-9]+(,(return|ii|i|select|up|down|left|right)@[0-9]+)*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_SEQUENCE must be comma-separated PCE key@seconds entries' >&2
        exit 1
    fi
    if [[ -z "$host_key_sequence" && "$host_key" != return && "$host_key" != i && "$host_key" != ii && "$host_key" != select && "$host_key" != up && "$host_key" != down && "$host_key" != left && "$host_key" != right ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY must name a supported PCE key' >&2
        exit 1
    fi
    if [[ "$input_route" != pid && "$input_route" != global_hid ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_INPUT_ROUTE must be pid or global_hid' >&2
        exit 1
    fi
    if [[ -z "$host_key_sequence" ]]; then
        case "$host_key" in
            return) host_key_code=36 ;;
            select) host_key_code=48 ;;
            # The configured PCE I button is SDL scancode 12. The macOS
            # physical I key is kVK_ANSI_I=34 and emits that SDL scancode.
            # Quartz Q (12) becomes SDL 20; keypad 3 becomes SDL 91.
            i) host_key_code=34 ;;
            ii) host_key_code=84 ;;
            up) host_key_code=13 ;;
            down) host_key_code=1 ;;
            left) host_key_code=0 ;;
            right) host_key_code=2 ;;
        esac
    fi
    if [[ "$capture_sdl_video_driver" == dummy ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY requires a non-dummy SDL video driver' >&2
        exit 1
    fi
    if [[ "$(uname -s)" != Darwin ]] || ! command -v osascript >/dev/null 2>&1; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY=return requires macOS osascript accessibility input' >&2
        exit 1
    fi
    if [[ ! -f "$quartz_keypair_script" ]] || ! command -v swift >/dev/null 2>&1; then
        printf '%s\n' 'FAIL: host input requires Swift and the checked-in Quartz keypair helper' >&2
        exit 1
    fi
    if [[ ! "$host_key_delay" =~ ^[0-9]+$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_DELAY must be a non-negative integer' >&2
        exit 1
    fi
    if [[ -n "$host_key_delays" && ! "$host_key_delays" =~ ^[0-9]+(,[0-9]+)*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_DELAYS must be comma-separated non-negative seconds' >&2
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
    if [[ "$input_route" == global_hid ]]; then
        if [[ ! "$host_focus_x" =~ ^[1-9][0-9]*$ ||
              ! "$host_focus_y" =~ ^[1-9][0-9]*$ ]] ||
           ! command -v cliclick >/dev/null 2>&1; then
            printf '%s\n' 'FAIL: host input requires cliclick and positive THERON_CAPTURE_FOCUS_X/Y coordinates' >&2
            exit 1
        fi
    fi
fi

capture_timeout_seconds=$seconds
if [[ "$host_input_requested" == 1 ]]; then
    capture_timeout_seconds=$((seconds + capture_startup_grace))
fi
if command -v gtimeout >/dev/null 2>&1; then
    timeout_command=(gtimeout "$capture_timeout_seconds")
elif command -v timeout >/dev/null 2>&1; then
    timeout_command=(timeout "$capture_timeout_seconds")
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

trace_files_are_line_delimited() {
    local trace_file
    local existing_trace_files=()

    # A literal backslash-n comes from a misescaped instrumented C string.
    # It merges independent observations and must never be admitted as evidence.
    for trace_file in "$@"; do
        [[ -e "$trace_file" ]] && existing_trace_files+=("$trace_file")
    done
    if (( ${#existing_trace_files[@]} == 0 )); then
        return 0
    fi
    LC_ALL=C perl -ne 'exit 1 if index($_, chr(92) . chr(92) . "n") >= 0' "${existing_trace_files[@]}"
}

trace_event_types() {
    local file=$1

    awk '
        /^host_sdl_event type=/ {
            split($0, fields, "=")
            if (!(fields[2] in seen)) {
                seen[fields[2]] = 1
                values[++count] = fields[2]
            }
        }
        END {
            if (count == 0) {
                print "none"
                exit
            }
            for (i = 1; i <= count; ++i) {
                if (i > 1)
                    printf ","
                printf "%s", values[i]
            }
            print ""
        }
    ' "$file"
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

wait_for_host_key_events() {
    local file=$1
    local expected_count=$2
    local attempts=$3
    local attempt
    local observed_count

    [[ -n "$attempts" ]] || attempts=40
    for ((attempt = 0; attempt < attempts; ++attempt)); do
        observed_count=$(trace_count '^host_key_event ' "$file")
        if (( observed_count >= expected_count )); then
            return 0
        fi
        sleep 0.25
    done
    return 1
}

wait_for_trace_producer() {
    local file=$1
    local attempts=$2
    local attempt

    for ((attempt = 0; attempt < attempts; ++attempt)); do
        if [[ -s "$file" ]] && grep -Fqx 'source=mednafen-pce-instrumented' "$file"; then
            return 0
        fi
        sleep 0.25
    done
    return 1
}

require_instrumented_mednafen_binary() {
    local binary=$1
    local marker

    # The stock Mednafen binary accepts the environment variables below but
    # silently ignores them. Refuse it before starting a timed original-media
    # run: an empty trace is not capture evidence. The runtime needs both the
    # general CPU/CD producer and the main-RAM control-flow producer.
    for marker in FIRESTAFF_THERON_IRQ2_TRACE FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE; do
        if ! grep -aFq "$marker" "$binary" 2>/dev/null; then
            printf '%s\n' 'FAIL: MEDNAFEN_BIN lacks the required Firestaff Theron instrumentation; build it with scripts/build_mednafen_theron_irq2_trace.sh' >&2
            return 1
        fi
    done
    return 0
}

resolve_mednafen_ui_pid() {
    local parent_pid=$1
    local child_pid
    local child_command
    local nested_pid

    for child_pid in $(pgrep -P "$parent_pid" 2>/dev/null || true); do
        child_command=$(ps -p "$child_pid" -o command= 2>/dev/null || true)
        if [[ "$child_command" == *"$mednafen_bin"* ]]; then
            printf '%s\n' "$child_pid"
            return 0
        fi
        nested_pid=$(resolve_mednafen_ui_pid "$child_pid" || true)
        if [[ "$nested_pid" =~ ^[1-9][0-9]*$ ]]; then
            printf '%s\n' "$nested_pid"
            return 0
        fi
    done
    return 1
}

resolve_mednafen_ui_pid_with_retry() {
    local parent_pid=$1
    local attempts=${2:-40}
    local attempt
    local resolved_pid

    for ((attempt = 0; attempt < attempts; ++attempt)); do
        resolved_pid=$(resolve_mednafen_ui_pid "$parent_pid" || true)
        if [[ "$resolved_pid" =~ ^[1-9][0-9]*$ ]]; then
            printf '%s\n' "$resolved_pid"
            return 0
        fi
        sleep 0.25
    done
    return 1
}

activate_mednafen_ui_pid_with_retry() {
    local target_pid=$1
    local attempts=${2:-20}
    local attempt

    for ((attempt = 0; attempt < attempts; ++attempt)); do
        if osascript <<APPLESCRIPT
tell application "System Events"
    set targetProcess to first application process whose unix id is $target_pid
    set frontmost of targetProcess to true
end tell
APPLESCRIPT
        then
            return 0
        fi
        sleep 0.25
    done
    return 1
}

trace_dir=$(dirname -- "$trace")
memory_trace="${trace}.memory"
cd_trace="${trace}.cd"
input_trace="${trace}.input"
main_ram_loader_trace="${trace}.main-ram-loader"
transition_receipt="${trace}.transition"
stage2_system_card_receipt="${trace}.stage2-system-card"
stdout_file="$trace_dir/$(basename -- "$trace").stdout"
stderr_file="$trace_dir/$(basename -- "$trace").stderr"

require_instrumented_mednafen_binary "$mednafen_bin" || exit 1

mkdir -p "$trace_dir"
rm -f "$trace" "$memory_trace" "$cd_trace" "$input_trace" "$main_ram_loader_trace" "$transition_receipt" "$stage2_system_card_receipt"
home_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-mednafen.XXXXXX")
cleanup_home=1
if [[ -n "$configured_home" ]]; then
    # The configured home is an input-map template, never the live capture
    # home. A private copy prevents an interrupted or concurrent capture from
    # retaining Mednafen's base-directory lock over the user's configuration.
    if ! cp -R "$configured_home/." "$home_dir"; then
        rm -rf "$home_dir"
        printf '%s\n' 'FAIL: could not prepare an isolated Mednafen capture home' >&2
        exit 1
    fi
fi
cleanup_capture() {
    local exit_status=$?

    trap - EXIT INT TERM
    if [[ "${mednafen_pid:-}" =~ ^[1-9][0-9]*$ ]] &&
       kill -0 "$mednafen_pid" 2>/dev/null; then
        # gtimeout owns a separate process group; terminate it as a group so
        # an interrupted capture cannot leave Mednafen holding its home lock.
        kill -TERM -- "-$mednafen_pid" 2>/dev/null ||
            kill -TERM "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
    fi
    if [[ "$cleanup_home" == 1 ]]; then
        rm -rf "$home_dir"
    fi
    return "$exit_status"
}
trap cleanup_capture EXIT INT TERM

launch=(
    "${timeout_command[@]}" env
    MEDNAFEN_HOME="$home_dir" \
    FIRESTAFF_THERON_IRQ2_TRACE="$trace" \
    FIRESTAFF_THERON_IRQ2_MEMORY_TRACE="$memory_trace" \
    FIRESTAFF_THERON_IRQ2_CD_TRACE="$cd_trace" \
    FIRESTAFF_THERON_IRQ2_INPUT_TRACE="$input_trace" \
    FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE="$main_ram_loader_trace" \
    SDL_VIDEODRIVER="$capture_sdl_video_driver" \
    SDL_AUDIODRIVER=dummy \
    "$mednafen_bin" \
    -sound 0 \
    -video.driver softfb \
    -pce.input.multitap 0 \
    -pce.input.port1 gamepad \
    -pce.arcadecard 0 \
    -pce.cdbios "$system_card"
    "$cue"
)
set +e
"${launch[@]}" >"$stdout_file" 2>"$stderr_file" &
mednafen_pid=$!
mednafen_ui_pid=0
if [[ "$host_input_requested" == 1 ]]; then
    host_key_previous_delay=0
    host_key_sequence_codes=()
    host_key_sequence_delays=()
    if [[ -n "$host_key_sequence" ]]; then
        IFS=',' read -r -a host_key_sequence_entries <<<"$host_key_sequence"
        host_key_repeats=${#host_key_sequence_entries[@]}
        for host_key_sequence_entry in "${host_key_sequence_entries[@]}"; do
            host_key_sequence_label=${host_key_sequence_entry%@*}
            host_key_sequence_delays+=("${host_key_sequence_entry#*@}")
            case "$host_key_sequence_label" in
                return) host_key_sequence_codes+=(36) ;;
                select) host_key_sequence_codes+=(48) ;;
                i) host_key_sequence_codes+=(34) ;;
                ii) host_key_sequence_codes+=(84) ;;
                up) host_key_sequence_codes+=(13) ;;
                down) host_key_sequence_codes+=(1) ;;
                left) host_key_sequence_codes+=(0) ;;
                right) host_key_sequence_codes+=(2) ;;
            esac
        done
    elif [[ -n "$host_key_delays" ]]; then
        IFS=',' read -r -a host_key_delay_entries <<<"$host_key_delays"
        host_key_repeats=${#host_key_delay_entries[@]}
    else
        host_key_delay_entries=("$host_key_delay")
    fi
    # Resolve only descendants of this capture's timeout/env launcher. A
    # global pgrep can select a stale Mednafen process from another capture.
    # The timeout/env process needs a short, bounded window to spawn Mednafen.
    mednafen_ui_pid=$(resolve_mednafen_ui_pid_with_retry "$mednafen_pid" || true)
    if [[ ! "$mednafen_ui_pid" =~ ^[1-9][0-9]*$ ]]; then
        kill "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
        printf '%s\n' 'FAIL: could not resolve the launched Mednafen UI process' >&2
        exit 1
    fi
    if [[ "$input_route" == global_hid ]]; then
        if ! cliclick "c:${host_focus_x},${host_focus_y}" ||
           ! activate_mednafen_ui_pid_with_retry "$mednafen_ui_pid"; then
            kill "$mednafen_pid" 2>/dev/null || true
            wait "$mednafen_pid" 2>/dev/null || true
            printf '%s\n' 'FAIL: macOS could not focus Mednafen for global HID input; grant accessibility permission to the invoking terminal' >&2
            exit 1
        fi
        # Cocoa activation is asynchronous; wait until the activated process
        # owns the foreground before using the global HID route.
        sleep 1
    fi
    if ! wait_for_trace_producer "$trace" "$((capture_startup_grace * 4))"; then
        kill "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
        printf '%s\n' 'FAIL: Mednafen did not produce an instrumented trace before host-input scheduling' >&2
        exit 1
    fi
    host_key_schedule_seconds=$SECONDS
    # Send real Quartz key-down/up pairs after PID-bound focus.
    # The old AppleScript tap never consumed THERON_CAPTURE_HOST_KEY_HOLD, so
    # its receipt overstated what reached SDL.  These explicit pairs make the
    # requested duration and repeat count observable at the host boundary;
    # only Mednafen's own input trace can establish emulated delivery.
    for ((host_key_attempt = 1; host_key_attempt <= host_key_repeats; ++host_key_attempt)); do
        if [[ -n "$host_key_sequence" ]]; then
            host_key_current_delay=${host_key_sequence_delays[$((host_key_attempt - 1))]}
            host_key_current_code=${host_key_sequence_codes[$((host_key_attempt - 1))]}
            if (( host_key_attempt > 1 && host_key_current_delay < host_key_previous_delay )); then
                kill "$mednafen_pid" 2>/dev/null || true
                wait "$mednafen_pid" 2>/dev/null || true
                printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_SEQUENCE times must be ordered' >&2
                exit 1
            fi
        elif [[ -n "$host_key_delays" ]]; then
            host_key_current_delay=${host_key_delay_entries[$((host_key_attempt - 1))]}
            host_key_current_code=$host_key_code
            if (( host_key_attempt > 1 && host_key_current_delay < host_key_previous_delay )); then
                kill "$mednafen_pid" 2>/dev/null || true
                wait "$mednafen_pid" 2>/dev/null || true
                printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_DELAYS must be ordered' >&2
                exit 1
            fi
        else
            host_key_current_delay=${host_key_delay_entries[0]}
            host_key_current_code=$host_key_code
        fi
        host_key_elapsed_seconds=$((SECONDS - host_key_schedule_seconds))
        if (( host_key_current_delay > host_key_elapsed_seconds )); then
            sleep "$((host_key_current_delay - host_key_elapsed_seconds))"
        fi
        host_key_previous_delay=$host_key_current_delay
        quartz_arguments=("$host_key_current_code" "$host_key_hold" "$mednafen_ui_pid")
        if [[ "$input_route" == global_hid ]]; then
            quartz_arguments+=(--global-hid)
        fi
        quartz_receipt=$(swift "$quartz_keypair_script" "${quartz_arguments[@]}") || {
            kill "$mednafen_pid" 2>/dev/null || true
            wait "$mednafen_pid" 2>/dev/null || true
            printf '%s\n' 'FAIL: macOS could not deliver the requested Quartz key pair' >&2
            exit 1
        }
        expected_quartz_route=quartz_keypair=posted_to_pid
        expected_quartz_activation=quartz_activation=not_required
        if [[ "$input_route" == global_hid ]]; then
            expected_quartz_route=quartz_keypair=posted_to_global_hid
            expected_quartz_activation=quartz_activation=accepted
        fi
        if [[ "$quartz_receipt" != *$'quartz_event_access=granted'* ||
              "$quartz_receipt" != *"$expected_quartz_route"* ||
              "$quartz_receipt" != *"quartz_target_pid=$mednafen_ui_pid"* ||
              "$quartz_receipt" != *"$expected_quartz_activation"* ]]; then
            kill "$mednafen_pid" 2>/dev/null || true
            wait "$mednafen_pid" 2>/dev/null || true
            printf '%s\n' 'FAIL: Quartz helper did not attest requested key delivery' >&2
            exit 1
        fi
        if [[ "$input_route" == global_hid &&
              "$quartz_receipt" != *"quartz_frontmost_pid=$mednafen_ui_pid"* ]]; then
            kill "$mednafen_pid" 2>/dev/null || true
            wait "$mednafen_pid" 2>/dev/null || true
            printf '%s\n' 'FAIL: Quartz global HID delivery requires Mednafen to own the foreground' >&2
            exit 1
        fi
        # The first two Run presses prove PID-bound delivery before the route
        # enters original loading screens. Later screens can intentionally
        # defer SDL dispatch; their actual count remains in the final receipt.
        if (( host_key_attempt <= 2 )) &&
           ! wait_for_host_key_events "$input_trace" "$((host_key_attempt * 2 - 1))" 40; then
            kill "$mednafen_pid" 2>/dev/null || true
            wait "$mednafen_pid" 2>/dev/null || true
            printf 'FAIL: Mednafen did not observe preflight key-down attempt %s after Quartz delivery\n' "$host_key_attempt" >&2
            exit 1
        fi
        sleep 0.2
    done
fi
wait "$mednafen_pid"
status=$?
set -e

if [[ ! -s "$trace" ]] || ! grep -Fqx 'source=mednafen-pce-instrumented' "$trace"; then
    printf '%s\n' 'FAIL: Mednafen did not produce a provenance-marked live trace' >&2
    exit 1
fi
if ! trace_files_are_line_delimited "$trace" "$cd_trace" "$memory_trace" "$input_trace" "$main_ram_loader_trace"; then
    printf '%s\n' 'FAIL: Mednafen emitted a literal backslash-n in a trace record' >&2
    exit 1
fi
# The loader receipt is separate from the later dynamic game-data handoff.
# Absence is expected for captures that do not reach this exact stage.
if ! "$script_dir/verify_theron_stage2_system_card_call_trace.sh" "$trace" >"$stage2_system_card_receipt" 2>/dev/null; then
    rm -f "$stage2_system_card_receipt"
fi
transition_input_count=$(trace_count '^pce_input_(read|write) ' "$input_trace")
transition_host_key_count=$(trace_count '^host_key_event ' "$input_trace")
transition_host_sdl_event_count=$(trace_count '^host_sdl_event ' "$input_trace")
transition_host_window_event_count=$(trace_count '^host_window_event ' "$input_trace")
transition_host_focus_state_count=$(trace_count '^host_focus_state ' "$input_trace")
transition_host_sdl_event_types=$(trace_event_types "$input_trace")
transition_irq_count=$(trace_count '^pce_cd_irq cpu_pc=' "$cd_trace")
transition_non_system_card_count=$(trace_count '^pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3} ' "$cd_trace")
transition_sector_count=$(trace_count '^cd_interface_raw_sector_read ' "$cd_trace")
transition_scsi_read_command_count=$(trace_count '^scsi_read_command ' "$cd_trace")
transition_scsi_sector_binding_count=$(trace_count '^scsi_read_sector_binding ' "$cd_trace")
transition_data_destination_count=$(trace_count '^pce_cd_data_destination_candidate ' "$cd_trace")
transition_game_main_ram_e009_count=$(trace_count '^game_main_ram_e009_dispatch ' "$trace")
transition_main_ram_loader_tii_count=$(trace_count '^main_ram_loader_block_transfer .*operation=tii ' "$main_ram_loader_trace")
transition_continuation_tii_count=$(trace_count '^main_ram_loader_block_transfer .*operation=tii source=3c80 ' "$main_ram_loader_trace")
transition_main_ram_loader_rts_count=$(trace_count '^main_ram_loader_rts ' "$main_ram_loader_trace")
transition_main_ram_loader_post_rts_count=$(trace_count '^main_ram_loader_post_rts ' "$main_ram_loader_trace")
transition_main_ram_loader_call_entry_count=$(trace_count '^main_ram_loader_call_entry ' "$main_ram_loader_trace")
transition_main_ram_loader_entry_next_count=$(trace_count '^main_ram_loader_entry_next ' "$main_ram_loader_trace")
transition_main_ram_loader_entry_successor_next_count=$(trace_count '^main_ram_loader_entry_successor_next ' "$main_ram_loader_trace")
transition_main_ram_loader_bra_count=$(trace_count '^main_ram_loader_bra ' "$main_ram_loader_trace")
transition_main_ram_loader_bra_target_count=$(trace_count '^main_ram_loader_bra_target ' "$main_ram_loader_trace")
transition_main_ram_loader_bra_target_jsr_count=$(trace_count '^main_ram_loader_bra_target_jsr ' "$main_ram_loader_trace")
transition_main_ram_loader_e009_dispatch_count=$(trace_count '^main_ram_loader_e009_dispatch ' "$main_ram_loader_trace")
{
    printf '%s\n' 'source=authentic-mednafen-transition-receipt'
    printf 'mednafen_binary_md5=%s\n' "$mednafen_binary_md5"
    printf 'track02_md5=%s\n' "$track02_md5"
    printf 'system_card_md5=%s\n' "$system_card_md5"
    printf 'input_transactions=%s\n' "$transition_input_count"
    printf 'host_key_events=%s\n' "$transition_host_key_count"
    printf 'host_sdl_events=%s\n' "$transition_host_sdl_event_count"
    printf 'host_sdl_event_types=%s\n' "$transition_host_sdl_event_types"
    printf 'host_window_events=%s\n' "$transition_host_window_event_count"
    printf 'host_focus_state_events=%s\n' "$transition_host_focus_state_count"
    printf 'cd_irq_callbacks=%s\n' "$transition_irq_count"
    printf 'non_system_card_pcecd_reads=%s\n' "$transition_non_system_card_count"
    printf 'raw_sector_spans=%s\n' "$transition_sector_count"
    printf 'scsi_read_commands=%s\n' "$transition_scsi_read_command_count"
    printf 'scsi_read_sector_bindings=%s\n' "$transition_scsi_sector_binding_count"
    printf 'byte_exact_fifo_ram_destinations=%s\n' "$transition_data_destination_count"
    printf 'game_main_ram_e009_dispatches=%s\n' "$transition_game_main_ram_e009_count"
    printf 'main_ram_loader_tii_transfers=%s\n' "$transition_main_ram_loader_tii_count"
    printf 'continuation_tii_source_3c80=%s\n' "$transition_continuation_tii_count"
    printf 'main_ram_loader_rts=%s\n' "$transition_main_ram_loader_rts_count"
    printf 'main_ram_loader_post_rts=%s\n' "$transition_main_ram_loader_post_rts_count"
    printf 'main_ram_loader_call_entries=%s\n' "$transition_main_ram_loader_call_entry_count"
    printf 'main_ram_loader_entry_next=%s\n' "$transition_main_ram_loader_entry_next_count"
    printf 'main_ram_loader_entry_successor_next=%s\n' "$transition_main_ram_loader_entry_successor_next_count"
    printf 'main_ram_loader_bra=%s\n' "$transition_main_ram_loader_bra_count"
    printf 'main_ram_loader_bra_targets=%s\n' "$transition_main_ram_loader_bra_target_count"
    printf 'main_ram_loader_bra_target_jsrs=%s\n' "$transition_main_ram_loader_bra_target_jsr_count"
    printf 'main_ram_loader_e009_dispatches=%s\n' "$transition_main_ram_loader_e009_dispatch_count"
    trace_input_order_receipt "$input_trace"
    if [[ "$host_input_requested" == 1 ]]; then
        if [[ -n "$host_key_sequence" ]]; then
            printf 'requested_host_key_sequence=%s\n' "$host_key_sequence"
        else
            printf 'requested_host_key=%s\n' "$host_key"
        fi
        printf 'host_input_target_pid=%s\n' "$mednafen_ui_pid"
        printf 'host_input_focus=screen_click:%s,%s\n' "$host_focus_x" "$host_focus_y"
        printf 'host_input_delivery=quartz_%s_key_down_up\n' "$input_route"
        printf '%s\n' 'host_input_schedule_origin=trace_ready'
        printf 'host_input_startup_grace_seconds=%s\n' "$capture_startup_grace"
        if [[ -n "$host_key_sequence" ]]; then
            printf 'host_input_delivery_key_code=sequence\n'
        else
            printf 'host_input_delivery_key_code=%s\n' "$host_key_code"
        fi
        printf 'host_input_delivery_attempts=%s\n' "$host_key_repeats"
        printf 'requested_host_key_hold_seconds=%s\n' "$host_key_hold"
        printf 'requested_host_key_repeats=%s\n' "$host_key_repeats"
        if [[ -n "$host_key_delays" ]]; then
            printf 'requested_host_key_delays=%s\n' "$host_key_delays"
        fi
    fi
    if [[ "$transition_input_count" -gt 0 && "$transition_irq_count" -gt 0 &&
          "$transition_non_system_card_count" -gt 0 && "$transition_sector_count" -gt 0 ]]; then
        printf '%s\n' 'transition=observed'
    else
        printf '%s\n' 'transition=missing'
    fi
} >"$transition_receipt"
if [[ "$host_input_requested" == 1 && "$transition_host_key_count" -eq 0 ]]; then
    printf 'BLOCKED: requested host key was not observed by Mednafen SDL dispatch; sdl_events=%s window_events=%s focus_events=%s (exit=%s)\n' "$transition_host_sdl_event_count" "$transition_host_window_event_count" "$transition_host_focus_state_count" "$status"
    exit 1
fi
if ! grep -Fq 'dynamic_cd_read_transaction ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_controller_state ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_destination_span pc=4093 destination=3800 bytes=32 fnv1a=' "$trace" ||
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
if ! grep -Eq '^cd_interface_raw_sector_read lba=[0-9]+ bytes=2352 sector_fnv1a=[0-9a-f]{8} span_offset=0 span_bytes=32 span_fnv1a=[0-9a-f]{8}$' "$cd_trace"; then
    printf 'BLOCKED: dynamic CPU receipts lack a complete authentic raw-sector receipt (exit=%s)\n' "$status"
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
