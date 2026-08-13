#!/usr/bin/env bash
set -euo pipefail

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mednafen_bin=${MEDNAFEN_BIN:-}
cue=${THERON_US_CUE:-${THERON_CUE:-}}
system_card=${THERON_SYSTEM_CARD:-}
trace=${THERON_LIVE_TRACE_OUTPUT:-}
seconds=${THERON_CAPTURE_SECONDS:-45}
capture_startup_grace=${THERON_CAPTURE_STARTUP_GRACE:-30}
capture_shutdown_signal=${THERON_CAPTURE_SHUTDOWN_SIGNAL:-INT}
# Keep the historical silent capture default, but allow a real CDDA-backed
# run when investigating BIOS/CD initialization.  This flag changes only the
# emulator's audio output path; it never promotes media bytes to game state.
capture_sound=${THERON_CAPTURE_SOUND:-0}
# Mednafen exposes two HuC6280 CD cores in some builds. Keep the
# source-compatible `pce` default, and allow `pce_fast` only when Mednafen's
# own module list advertises it. This only changes the emulator core; it never
# changes admitted media bytes.
capture_mednafen_module=${THERON_CAPTURE_MEDNAFEN_MODULE:-pce}
capture_arcadecard_setting=pce.arcadecard
capture_cdbios_setting=pce.cdbios
if [[ "$capture_mednafen_module" == pce_fast ]]; then
    capture_arcadecard_setting=pce_fast.arcadecard
    capture_cdbios_setting=pce_fast.cdbios
fi
# An empty value is intentional on macOS: it lets native SDL2 select Cocoa.
# Use `-` rather than `:-` so the caller can distinguish that from the
# headless dummy default.
capture_sdl_video_driver=${THERON_CAPTURE_SDL_VIDEODRIVER-dummy}
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
capture_input_grab_delay=${THERON_CAPTURE_INPUT_GRAB_DELAY:-2}
replay_input_script=${THERON_CAPTURE_REPLAY_INPUT_SCRIPT:-}
autoload_state=${THERON_CAPTURE_AUTOLOAD_STATE:-}
autoload_movie=${THERON_CAPTURE_AUTOLOAD_MOVIE:-}
rng_consumer_sample_limit=${THERON_CAPTURE_RNG_CONSUMER_SAMPLE_LIMIT:-512}
rng_consumer_window_limit=${THERON_CAPTURE_RNG_CONSUMER_WINDOW_LIMIT:-32}
main_ram_consumer_sample_limit=${THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT:-65536}
input_route=${THERON_CAPTURE_INPUT_ROUTE:-pid}
host_focus_x=${THERON_CAPTURE_FOCUS_X:-960}
host_focus_y=${THERON_CAPTURE_FOCUS_Y:-540}
host_key_code=
host_input_requested=0
if [[ -n "$host_key" || -n "$host_key_sequence" ]]; then
    host_input_requested=1
fi
quartz_keypair_script="$script_dir/send_theron_macos_quartz_keypair.swift"
quartz_grab_script="$script_dir/send_theron_macos_quartz_chord.swift"

if [[ -z "$mednafen_bin" || -z "$cue" || -z "$system_card" || -z "$trace" ]]; then
    printf '%s\n' 'SKIP: MEDNAFEN_BIN, THERON_US_CUE/THERON_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required'
    exit 0
fi
if [[ ! -x "$mednafen_bin" || ! -f "$cue" || ! -f "$system_card" ]]; then
    printf '%s\n' 'FAIL: Mednafen, US CUE, or System Card path is unavailable' >&2
    exit 1
fi
if [[ ! "$rng_consumer_sample_limit" =~ ^[0-9]+$ ]] ||
   (( rng_consumer_sample_limit < 512 || rng_consumer_sample_limit > 65536 )); then
    printf '%s\n' 'FAIL: THERON_CAPTURE_RNG_CONSUMER_SAMPLE_LIMIT must be an integer from 512 through 65536' >&2
    exit 1
fi
if [[ ! "$rng_consumer_window_limit" =~ ^[0-9]+$ ]] ||
   (( rng_consumer_window_limit < 1 || rng_consumer_window_limit > 1024 )); then
    printf '%s\n' 'FAIL: THERON_CAPTURE_RNG_CONSUMER_WINDOW_LIMIT must be an integer from 1 through 1024' >&2
    exit 1
fi
if [[ ! "$main_ram_consumer_sample_limit" =~ ^[0-9]+$ ]] ||
   (( main_ram_consumer_sample_limit < 4096 || main_ram_consumer_sample_limit > 1048576 )); then
    printf '%s\n' 'FAIL: THERON_CAPTURE_MAIN_RAM_CONSUMER_SAMPLE_LIMIT must be an integer from 4096 through 1048576' >&2
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

capture_profile_scancode() {
    local key=$1
    awk -v key="$key" '
        $1 == key && $2 == "keyboard" && $3 == "0x0" { print $4; exit }
    ' "$configured_home/mednafen.cfg"
}

capture_host_code_for_mapping() {
    local key=$1
    local scancode=$2
    case "$key:$scancode" in
        # The checked-in external capture home uses keypad 3/2 and WASD.
        i:91) printf '%s' 85 ;;
        ii:90) printf '%s' 84 ;;
        up:26) printf '%s' 13 ;;
        down:22) printf '%s' 1 ;;
        left:4) printf '%s' 0 ;;
        right:7) printf '%s' 2 ;;
        # The normal macOS home may use number-row 3/2 and arrow keys.
        # Mednafen stores SDL scancodes: 3 is 32 and 2 is 31.
        i:32) printf '%s' 20 ;;
        ii:31) printf '%s' 19 ;;
        up:82) printf '%s' 126 ;;
        down:81) printf '%s' 125 ;;
        left:80) printf '%s' 123 ;;
        right:79) printf '%s' 124 ;;
        # Keep physical-I/II profiles usable when supplied by an older
        # Mednafen template; the mapping is still authenticated by cfg.
        i:12) printf '%s' 34 ;;
        # Stable letter bindings avoid keyboard-layout-dependent punctuation
        # on macOS. SDL_SCANCODE_Z/X are 29/27; Quartz virtual keycodes are 6/7.
        i:29) printf '%s' 6 ;;
        ii:27) printf '%s' 7 ;;
        # SDL_SCANCODE_COMMA/SDL_SCANCODE_PERIOD are common on compact
        # Mac keyboards when Button I/II have been assigned to `,`/`.`.
        # Quartz uses virtual keycodes 43/47 for those two keys.  Accept
        # either punctuation key for either PCE button; the Mednafen cfg is
        # the authority for which one is Button I versus Button II.
        i:54) printf '%s' 43 ;;
        i:55) printf '%s' 47 ;;
        ii:54) printf '%s' 43 ;;
        ii:55) printf '%s' 47 ;;
        *) return 1 ;;
    esac
}

require_capture_profile_mappings() {
    if [[ ! -f "$configured_home/mednafen.cfg" ]]; then
        printf '%s\n' 'FAIL: THERON_MEDNAFEN_HOME has no Mednafen configuration file' >&2
        exit 1
    fi

    # Mednafen's PCE defaults and the user's explicit macOS profile are both
    # valid. Resolve the host key from the actual SDL scancode instead of
    # silently sending a non-PCE key when the profile uses number-row or
    # keypad Button I/II bindings.
    local run_scancode select_scancode
    run_scancode=$(capture_profile_scancode pce.input.port1.gamepad.run)
    select_scancode=$(capture_profile_scancode pce.input.port1.gamepad.select)
    if [[ "$run_scancode" != 40 || "$select_scancode" != 43 ]]; then
        printf 'FAIL: THERON_MEDNAFEN_HOME must retain RUN=40 and SELECT=43 (got RUN=%s SELECT=%s)\n' \
            "${run_scancode:-missing}" "${select_scancode:-missing}" >&2
        exit 1
    fi

    local key label scancode host_code
    for key in i ii up down left right; do
        case "$key" in
            i) label=I ;;
            ii) label=II ;;
            up) label=UP ;;
            down) label=DOWN ;;
            left) label=LEFT ;;
            right) label=RIGHT ;;
        esac
        scancode=$(capture_profile_scancode "pce.input.port1.gamepad.$key")
        host_code=$(capture_host_code_for_mapping "$key" "$scancode" || true)
        if [[ -z "$host_code" ]]; then
            printf 'FAIL: THERON_MEDNAFEN_HOME does not retain a supported %s mapping (SDL scancode %s)\n' \
                "$label" "${scancode:-missing}" >&2
            exit 1
        fi
        printf -v "capture_${key}_host_code" '%s' "$host_code"
    done
}

track02_mode=$(awk '
    /^[[:space:]]*TRACK[[:space:]]+02[[:space:]]+MODE1\/(2352|2048)[[:space:]]*$/ {
        if ($0 ~ /MODE1\/2352/) print "MODE1/2352"; else print "MODE1/2048"
        exit
    }
' "$cue")
if [[ "$track02_mode" != 'MODE1/2352' && "$track02_mode" != 'MODE1/2048' ]]; then
    printf '%s\n' 'FAIL: CUE has no safe TRACK 02 MODE1/2352 or MODE1/2048 member' >&2
    exit 1
fi
track02_member=$(awk '
    /^FILE "/ {
        line = $0
        sub(/^FILE "/, "", line)
        sub(/" BINARY[[:space:]]*$/, "", line)
        file = line
        next
    }
    /^[[:space:]]*FILE[[:space:]]+[^\"]+[[:space:]]+BINARY[[:space:]]*$/ {
        line = $0
        sub(/^[[:space:]]*FILE[[:space:]]+/, "", line)
        sub(/[[:space:]]+BINARY[[:space:]]*$/, "", line)
        file = line
        next
    }
    /^[[:space:]]*TRACK[[:space:]]+02[[:space:]]+MODE1\/(2352|2048)[[:space:]]*$/ {
        print file
        exit
    }
' "$cue")
if [[ -z "$track02_member" || "$track02_member" == */* || "$track02_member" == *\\* ]]; then
    printf '%s\n' 'FAIL: CUE has no safe TRACK 02 data member' >&2
    exit 1
fi
track02_path="$(dirname -- "$cue")/$track02_member"
capture_cue="$cue"
capture_cue_needs_split_iso=0
capture_split_iso_cache="${HOME:-}/.firestaff/cache/theron/TQUS02-ceb02343868f80cec899e9b239aff2da.iso"
capture_jp_iso_sibling="$(dirname -- "$cue")/TQJP02End.iso"
if [[ ! -f "$track02_path" && "$track02_mode" == 'MODE1/2048' &&
      ( "$track02_member" == 'TQUS02.iso' ||
        "$track02_member" == 'TQUS02End.iso' ) &&
      -f "$capture_split_iso_cache" ]]; then
    split_iso_md5=$(md5_file "$capture_split_iso_cache") || split_iso_md5=
    if [[ "$split_iso_md5" == ceb02343868f80cec899e9b239aff2da ]]; then
        # The supplied retail CUE names TQUS02.iso, while Decode.bat's real
        # split distribution stores that Track 02 payload as TQUS19.iso
        # followed by TQUS02End.iso. The production intake assembles and hashes this exact ISO.
        # Give Mednafen a private normalized CUE so
        # the capture consumes the same authenticated bytes rather than
        # silently falling back to the incomplete/missing member.
        capture_cue_needs_split_iso=1
        track02_path="$capture_split_iso_cache"
    fi
fi
if [[ ! -f "$track02_path" && "$track02_mode" == 'MODE1/2048' &&
      "$track02_member" == 'TQJP02.iso' &&
      -f "$capture_jp_iso_sibling" ]]; then
    split_iso_md5=$(md5_file "$capture_jp_iso_sibling") || split_iso_md5=
    if [[ "$split_iso_md5" == 397039af02d50d15c70b74088eb8a1cb ]]; then
        # The Japanese retail CUE names TQJP02.iso, while the supplied
        # archive retains the complete authenticated payload as TQJP02End.iso.
        # Keep the same private CUE normalization used by the US split route.
        capture_cue_needs_split_iso=1
        capture_split_iso_cache="$capture_jp_iso_sibling"
        track02_path="$capture_jp_iso_sibling"
    fi
fi
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
if [[ "$track02_mode" == 'MODE1/2352' ]]; then
    case "$track02_md5" in
        b7afb338ad31be1025b53f9aff12d73a|f23601102138f87c33025877767ebf76) ;;
        *)
            printf '%s\n' 'FAIL: CUE TRACK 02 is not an authenticated Theron JP/US raw BIN' >&2
            exit 1
            ;;
    esac
else
    # The retail CUE sheets also carry a MODE1/2048 data track. These are the
    # complete canonical ISO identities admitted by the runtime intake; they
    # are not interchangeable with the raw BIN identities above.
    case "$track02_md5" in
        397039af02d50d15c70b74088eb8a1cb|ceb02343868f80cec899e9b239aff2da) ;;
        *)
            printf '%s\n' 'FAIL: CUE TRACK 02 is not an authenticated Theron JP/US ISO' >&2
            exit 1
            ;;
    esac
fi
if [[ ! "$seconds" =~ ^[1-9][0-9]*$ ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_SECONDS must be a positive integer' >&2
    exit 1
fi
if [[ ! "$capture_startup_grace" =~ ^[1-9][0-9]*$ ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_STARTUP_GRACE must be a positive integer' >&2
    exit 1
fi
if [[ "$capture_sound" != 0 && "$capture_sound" != 1 ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_SOUND must be 0 or 1' >&2
    exit 1
fi
if [[ "$capture_mednafen_module" != pce && "$capture_mednafen_module" != pce_fast ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_MEDNAFEN_MODULE must be pce or pce_fast' >&2
    exit 1
fi
require_mednafen_module() {
    local requested=$1
    local help_output

    # Fail before media setup when the selected binary does not actually
    # contain the requested core. Some macOS builds expose only `pce`; passing
    # `pce_fast` to those builds otherwise leaves an expensive, empty capture
    # with no trustworthy runtime receipt.
    help_output=$("$mednafen_bin" -help 2>&1 || true)
    if printf '%s\n' "$help_output" |
       awk -v requested="$requested" '
           /^ Emulation modules:/ {
               for (i = 3; i <= NF; ++i)
                   if ($i == requested) found = 1
           }
           END { exit(found ? 0 : 1) }
       '; then
        return 0
    fi

    # Some instrumented 1.32.1 macOS builds omit the module-list block from
    # -help even though the compiled PCE core is present and selectable. Keep
    # the source-compatible `pce` fallback, but never infer `pce_fast` from a
    # string in the binary: its command-line settings differ and a string
    # match can accept a build that rejects -force_module pce_fast.
    case "$requested" in
        pce)
            if grep -aFq 'PC Engine (CD)/TurboGrafx 16 (CD)/SuperGrafx' "$mednafen_bin" &&
               grep -aFq '.pce' "$mednafen_bin"; then
                return 0
            fi
            ;;
        pce_fast)
            # No help-less fallback is safe for pce_fast.
            ;;
    esac
    printf 'FAIL: MEDNAFEN_BIN does not expose the requested emulation module: %s\n' \
        "$requested" >&2
    exit 1
}
require_mednafen_module "$capture_mednafen_module"
if [[ "$capture_shutdown_signal" != INT && "$capture_shutdown_signal" != TERM ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_SHUTDOWN_SIGNAL must be INT or TERM' >&2
    exit 1
fi
if [[ -n "$configured_home" && ! -d "$configured_home" ]]; then
    printf '%s\n' 'FAIL: THERON_MEDNAFEN_HOME must name an existing Mednafen configuration directory' >&2
    exit 1
fi
if [[ -n "$autoload_state" && ! -f "$autoload_state" ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_AUTOLOAD_STATE must name an existing Mednafen state file' >&2
    exit 1
fi
if [[ -n "$autoload_state" ]]; then
    autoload_state_magic=$(dd if="$autoload_state" bs=1 count=4 2>/dev/null || true)
    if [[ "$autoload_state_magic" == 'HUBM' ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_AUTOLOAD_STATE points to HUBM SRAM, not a Mednafen savestate' >&2
        exit 1
    fi
fi
autoload_state_md5=none
if [[ -n "$autoload_state" ]]; then
    autoload_state_md5=$(md5_file "$autoload_state") || {
        printf '%s\n' 'FAIL: could not hash the authentic Mednafen autoload state' >&2
        exit 1
    }
fi
if [[ -n "$autoload_movie" && ! -f "$autoload_movie" ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_AUTOLOAD_MOVIE must name an existing Mednafen movie file' >&2
    exit 1
fi
if [[ -n "$autoload_state" && -n "$autoload_movie" ]]; then
    printf '%s\n' 'FAIL: THERON_CAPTURE_AUTOLOAD_STATE and THERON_CAPTURE_AUTOLOAD_MOVIE cannot be combined' >&2
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
    if [[ -n "$host_key_sequence" && ! "$host_key_sequence" =~ ^(run|return|ii|i|select|up|down|left|right)@[0-9]+(,(run|return|ii|i|select|up|down|left|right)@[0-9]+)*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY_SEQUENCE must be comma-separated PCE key@seconds entries' >&2
        exit 1
    fi
    if [[ -z "$host_key_sequence" && "$host_key" != run && "$host_key" != return && "$host_key" != i && "$host_key" != ii && "$host_key" != select && "$host_key" != up && "$host_key" != down && "$host_key" != left && "$host_key" != right ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_HOST_KEY must name a supported PCE key' >&2
        exit 1
    fi
    if [[ "$input_route" != pid && "$input_route" != global_hid ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_INPUT_ROUTE must be pid or global_hid' >&2
        exit 1
    fi
    if [[ -z "$host_key_sequence" ]]; then
        case "$host_key" in
            run|return) host_key_code=36 ;;
            select) host_key_code=48 ;;
            i) host_key_code=$capture_i_host_code ;;
            ii) host_key_code=$capture_ii_host_code ;;
            up) host_key_code=$capture_up_host_code ;;
            down) host_key_code=$capture_down_host_code ;;
            left) host_key_code=$capture_left_host_code ;;
            right) host_key_code=$capture_right_host_code ;;
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
    if [[ ! -f "$quartz_keypair_script" || ! -f "$quartz_grab_script" ]] ||
       ! command -v swift >/dev/null 2>&1; then
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
    if [[ ! "$capture_input_grab_delay" =~ ^[1-9][0-9]*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_INPUT_GRAB_DELAY must be a positive integer' >&2
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
if [[ -n "$replay_input_script" ]]; then
    if [[ "$host_input_requested" == 1 ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_REPLAY_INPUT_SCRIPT cannot be combined with host-key input' >&2
        exit 1
    fi
    if [[ ! "$replay_input_script" =~ ^(run|ii|i|select|up|down|left|right)@[1-9][0-9]*(:[1-9][0-9]*)?(,(run|ii|i|select|up|down|left|right)@[1-9][0-9]*(:[1-9][0-9]*)?)*$ ]]; then
        printf '%s\n' 'FAIL: THERON_CAPTURE_REPLAY_INPUT_SCRIPT must be comma-separated PCE key@frame or key@frame:hold entries' >&2
        exit 1
    fi
fi

capture_timeout_seconds=$seconds
if [[ "$host_input_requested" == 1 ]]; then
    capture_timeout_seconds=$((seconds + capture_startup_grace))
fi
# Mednafen acknowledges SIGINT as an in-emulator event and may keep its main
# loop alive. Keep the requested soft shutdown signal for trace flushing, but
# bound that grace period so a failed capture cannot retain the isolated home
# lock or leave an orphaned emulator process behind.
capture_force_kill_seconds=5
if command -v gtimeout >/dev/null 2>&1; then
    timeout_command=(gtimeout -k "$capture_force_kill_seconds" -s "$capture_shutdown_signal" "$capture_timeout_seconds")
elif command -v timeout >/dev/null 2>&1; then
    timeout_command=(timeout -k "$capture_force_kill_seconds" -s "$capture_shutdown_signal" "$capture_timeout_seconds")
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

require_snapshot_size() {
    local file=$1
    local expected=$2
    local label=$3
    local actual

    if [[ ! -f "$file" ]]; then
        printf 'FAIL: Mednafen did not emit the %s snapshot\n' "$label" >&2
        return 1
    fi
    actual=$(wc -c <"$file" | tr -d '[:space:]')
    if [[ "$actual" != "$expected" ]]; then
        printf 'FAIL: %s snapshot has %s bytes; expected %s\n' "$label" "$actual" "$expected" >&2
        return 1
    fi
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

scripted_input_expected_mask() {
    case "$1" in
        i) printf '%04x' 1 ;;
        ii) printf '%04x' 2 ;;
        select) printf '%04x' 4 ;;
        run) printf '%04x' 8 ;;
        up) printf '%04x' 16 ;;
        right) printf '%04x' 32 ;;
        down) printf '%04x' 64 ;;
        left) printf '%04x' 128 ;;
        *) return 1 ;;
    esac
}

verify_scripted_input_masks() {
    local entry expected_key expected_mask actual_key actual_mask
    local event_index=0
    local expected_entries=()

    [[ -n "$replay_input_script" ]] || return 0
    IFS=',' read -r -a expected_entries <<< "$replay_input_script"
    while IFS= read -r event_line; do
        [[ -n "$event_line" ]] || continue
        if (( event_index >= ${#expected_entries[@]} )); then
            printf '%s\n' 'BLOCKED: Mednafen emitted more scripted PCE input events than requested' >&2
            return 1
        fi
        entry=${expected_entries[$event_index]}
        expected_key=${entry%@*}
        expected_mask=$(scripted_input_expected_mask "$expected_key") || {
            printf 'BLOCKED: unsupported scripted PCE input key in capture plan: %s\n' "$expected_key" >&2
            return 1
        }
        actual_key=$(printf '%s\n' "$event_line" |
            sed -n 's/.* key=\([^ ]*\) mask=\([^ ]*\) hold=.*/\1/p')
        actual_mask=$(printf '%s\n' "$event_line" |
            sed -n 's/.* key=\([^ ]*\) mask=\([^ ]*\) hold=.*/\2/p')
        if [[ "$actual_key" != "$expected_key" ||
              "$actual_mask" != "$expected_mask" ]]; then
            printf 'BLOCKED: scripted PCE input mask mismatch for %s: got key=%s mask=%s expected key=%s mask=%s\n' \
                "$expected_key" "$actual_key" "$actual_mask" "$expected_key" "$expected_mask" >&2
            return 1
        fi
        event_index=$((event_index + 1))
    done < <(awk '/^scripted_pce_input_event / { print }' "$input_trace")

    if (( event_index != ${#expected_entries[@]} )); then
        printf 'BLOCKED: Mednafen emitted %s scripted PCE input events; expected %s\n' \
            "$event_index" "${#expected_entries[@]}" >&2
        return 1
    fi
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
    local marker=${3:-source=mednafen-pce-instrumented}
    local attempt

    for ((attempt = 0; attempt < attempts; ++attempt)); do
        if [[ -s "$file" ]] && grep -Fqx "$marker" "$file"; then
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
    # general CPU/CD producer, main-RAM control-flow producer, and the
    # game-owned main-RAM consumer-read producer.
    for marker in FIRESTAFF_THERON_IRQ2_TRACE FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE FIRESTAFF_THERON_RNG_CONSUMER_TRACE FIRESTAFF_THERON_VDC_IO_TRACE; do
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
main_ram_consumer_trace="${trace}.main-ram-consumer"
main_ram_target_trace="${trace}.main-ram-target"
spawn_consumer_trace="${trace}.spawn-consumer"
spawn_register_trace="${trace}.spawn-registers"
rng_consumer_trace="${trace}.rng-consumer"
rng_code_trace="${trace}.rng-code"
vram_snapshot="${trace}.vram"
vce_snapshot="${trace}.vce"
vdc_io_trace="${trace}.vdc-io"
transition_receipt="${trace}.transition"
stage2_system_card_receipt="${trace}.stage2-system-card"
stdout_file="$trace_dir/$(basename -- "$trace").stdout"
stderr_file="$trace_dir/$(basename -- "$trace").stderr"

require_instrumented_mednafen_binary "$mednafen_bin" || exit 1
if [[ -n "$replay_input_script" ]] &&
   ! grep -aFq 'FIRESTAFF_THERON_REPLAY_INPUT_SCRIPT' "$mednafen_bin" 2>/dev/null; then
    printf '%s\n' 'FAIL: MEDNAFEN_BIN lacks the required Firestaff Theron scripted-PCE-input producer' >&2
    exit 1
fi

mkdir -p "$trace_dir"
rm -f "$trace" "$memory_trace" "$cd_trace" "$input_trace" "$main_ram_loader_trace" "$main_ram_consumer_trace" "$main_ram_target_trace" "$spawn_consumer_trace" "$spawn_register_trace" "$rng_consumer_trace" "$rng_code_trace" "$vram_snapshot" "$vce_snapshot" "$vdc_io_trace" "$transition_receipt" "$stage2_system_card_receipt"
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
link_capture_cue_members() {
    local source_cue=$1
    local destination_dir=$2
    local source_dir
    local member source_path destination_path destination_parent

    source_dir=$(CDPATH= cd -- "$(dirname -- "$source_cue")" && pwd)
    while IFS= read -r member; do
        [[ -n "$member" ]] || continue
        # Track 02 is replaced below by the hash-verified normalized ISO.
        [[ "$member" == "$track02_member" ]] && continue
        # An absolute FILE already resolves independently of the private CUE
        # directory. Relative members must be made visible there.
        [[ "$member" == /* ]] && continue
        case "$member" in
            ../*|*/../*|*/..)
                printf 'FAIL: CUE member escapes its source directory: %s\n' \
                    "$member" >&2
                return 1
                ;;
        esac
        source_path="$source_dir/$member"
        if [[ ! -f "$source_path" ]]; then
            printf 'FAIL: CUE member is missing from source directory: %s\n' \
                "$source_path" >&2
            return 1
        fi
        destination_path="$destination_dir/$member"
        destination_parent=$(dirname -- "$destination_path")
        mkdir -p "$destination_parent"
        if [[ -e "$destination_path" || -L "$destination_path" ]]; then
            if [[ ! -L "$destination_path" ||
                  "$(readlink "$destination_path")" != "$source_path" ]]; then
                printf 'FAIL: capture-home member collides with another file: %s\n' \
                    "$destination_path" >&2
                return 1
            fi
        else
            ln -s "$source_path" "$destination_path"
        fi
    done < <(awk '
        /^[[:space:]]*FILE[[:space:]]+"/ {
            line = $0
            sub(/^[[:space:]]*FILE[[:space:]]+"/, "", line)
            sub(/"[[:space:]]+(WAVE|BINARY)[[:space:]]*$/, "", line)
            print line
            next
        }
        /^[[:space:]]*FILE[[:space:]]+[^"[:space:]]+[[:space:]]+(WAVE|BINARY)[[:space:]]*$/ {
            line = $0
            sub(/^[[:space:]]*FILE[[:space:]]+/, "", line)
            sub(/[[:space:]]+(WAVE|BINARY)[[:space:]]*$/, "", line)
            print line
        }
    ' "$source_cue")
}
if [[ "$capture_cue_needs_split_iso" == 1 ]]; then
    capture_cue="$home_dir/theron-capture.cue"
    # Accept both the quoted and unquoted FILE spelling used by the supplied
    # retail CUE sheets. Only the authenticated Track 02 member is replaced;
    # audio and Track 19 references retain the user's original layout.
    sed \
        -e "s#^FILE \\\"${track02_member}\\\" BINARY[[:space:]]*\$#FILE \\\"$capture_split_iso_cache\\\" BINARY#" \
        -e "s#^FILE ${track02_member} BINARY[[:space:]]*\$#FILE \\\"$capture_split_iso_cache\\\" BINARY#" \
        "$cue" >"$capture_cue"
    link_capture_cue_members "$cue" "$home_dir" || exit 1
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

# RMDUI defaults to the first disc, but make the capture contract explicit:
# the authenticated Track 02 medium must be inserted before BIOS execution.
launch=(
    "${timeout_command[@]}" env
    MEDNAFEN_HOME="$home_dir" \
    FIRESTAFF_THERON_IRQ2_TRACE="$trace" \
    FIRESTAFF_THERON_IRQ2_MEMORY_TRACE="$memory_trace" \
    FIRESTAFF_THERON_IRQ2_CD_TRACE="$cd_trace" \
    FIRESTAFF_THERON_IRQ2_INPUT_TRACE="$input_trace" \
    FIRESTAFF_THERON_REPLAY_INPUT_SCRIPT="$replay_input_script" \
    FIRESTAFF_THERON_AUTOLOAD_STATE="$autoload_state" \
    FIRESTAFF_THERON_AUTOLOAD_MOVIE="$autoload_movie" \
    FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE="$main_ram_loader_trace" \
    FIRESTAFF_THERON_MAIN_RAM_CONSUMER_TRACE="$main_ram_consumer_trace" \
    FIRESTAFF_THERON_MAIN_RAM_CONSUMER_SAMPLE_LIMIT="$main_ram_consumer_sample_limit" \
    FIRESTAFF_THERON_MAIN_RAM_TARGET_TRACE="$main_ram_target_trace" \
    FIRESTAFF_THERON_SPAWN_CONSUMER_TRACE="$spawn_consumer_trace" \
    FIRESTAFF_THERON_SPAWN_REGISTER_TRACE="$spawn_register_trace" \
    FIRESTAFF_THERON_SPAWN_REGISTER_SAMPLE_LIMIT="${THERON_CAPTURE_SPAWN_REGISTER_SAMPLE_LIMIT:-65536}" \
    FIRESTAFF_THERON_RNG_CONSUMER_TRACE="$rng_consumer_trace" \
    FIRESTAFF_THERON_RNG_CONSUMER_SAMPLE_LIMIT="$rng_consumer_sample_limit" \
    FIRESTAFF_THERON_RNG_CONSUMER_WINDOW_LIMIT="$rng_consumer_window_limit" \
    FIRESTAFF_THERON_RNG_CODE_TRACE="$rng_code_trace" \
    FIRESTAFF_THERON_VRAM_SNAPSHOT="$vram_snapshot" \
    FIRESTAFF_THERON_VCE_SNAPSHOT="$vce_snapshot" \
    FIRESTAFF_THERON_VDC_IO_TRACE="$vdc_io_trace" \
    SDL_VIDEODRIVER="$capture_sdl_video_driver" \
    SDL_AUDIODRIVER=dummy \
    "$mednafen_bin" \
    -force_module "$capture_mednafen_module" \
    -which_medium 0 \
    -sound "$capture_sound" \
    -video.driver softfb \
    -pce.input.multitap 0 \
    -pce.input.port1 gamepad \
    -"$capture_arcadecard_setting" 0 \
    -"$capture_cdbios_setting" "$system_card"
    "$capture_cue"
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
                run|return) host_key_sequence_codes+=(36) ;;
                select) host_key_sequence_codes+=(48) ;;
                i) host_key_sequence_codes+=($capture_i_host_code) ;;
                ii) host_key_sequence_codes+=($capture_ii_host_code) ;;
                up) host_key_sequence_codes+=($capture_up_host_code) ;;
                down) host_key_sequence_codes+=($capture_down_host_code) ;;
                left) host_key_sequence_codes+=($capture_left_host_code) ;;
                right) host_key_sequence_codes+=($capture_right_host_code) ;;
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
        # owns the foreground before using the global HID route.  The delay is
        # configurable because macOS can restore another SDL window while the
        # System Card is still opening the CD title.
        sleep "$capture_input_grab_delay"
    fi
    # The CPU trace is stdio-buffered until shutdown. The input producer is
    # flushed per capture event and is the safe readiness boundary for
    # scheduling real Quartz input while the process is still running.
    if ! wait_for_trace_producer "$input_trace" \
        "$((capture_startup_grace * 4))" \
        'source=mednafen-pce-instrumented-input'; then
        kill "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
        printf '%s\n' 'FAIL: Mednafen did not produce an instrumented trace before host-input scheduling' >&2
        exit 1
    fi
    # Mednafen does not deliver emulated keyboard state until input grab is
    # active. The checked-in macOS profile uses Ctrl+Shift+G because the
    # default Menu-key shortcut is unavailable on most Mac keyboards.
    grab_quartz_arguments=("$mednafen_ui_pid")
    if [[ "$input_route" == global_hid ]]; then
        grab_quartz_arguments+=(--global-hid)
    fi
    expected_grab_route=quartz_chord=posted_to_pid
    if [[ "$input_route" == global_hid ]]; then
        expected_grab_route=quartz_chord=posted_to_global_hid
    fi
    grab_trace_ready=0
    grab_receipt_valid=0
    # A foreground SDL window can be replaced by another app between the
    # activation receipt and the first chord. Retry only this reversible input
    # handshake, and require both Quartz's receipt and Mednafen's own
    # input_grab_state marker before sending any gameplay keys.
    for ((grab_attempt = 1; grab_attempt <= 4; ++grab_attempt)); do
        grab_receipt=$(swift "$quartz_grab_script" "${grab_quartz_arguments[@]}" 2>/dev/null || true)
        if [[ "$grab_receipt" == *$'quartz_event_access=granted'* &&
              "$grab_receipt" == *"$expected_grab_route"* &&
              "$grab_receipt" == *"quartz_target_pid=$mednafen_ui_pid"* &&
              "$grab_receipt" == *'quartz_chord_keys=ctrl+shift+g'* ]]; then
            grab_receipt_valid=1
            for ((grab_wait = 0; grab_wait < 40; ++grab_wait)); do
                if grep -Fq 'input_grab_state enabled=1' "$input_trace"; then
                    grab_trace_ready=1
                    break
                fi
                sleep 0.25
            done
            [[ "$grab_trace_ready" == 1 ]] && break
        fi
        sleep 1
    done
    if [[ "$grab_receipt_valid" != 1 || "$grab_trace_ready" != 1 ]]; then
        kill "$mednafen_pid" 2>/dev/null || true
        wait "$mednafen_pid" 2>/dev/null || true
        printf 'FAIL: Mednafen did not attest InputGrab=1 after Quartz chord retries (receipt=%s trace=%s)\n' \
            "$grab_receipt_valid" "$grab_trace_ready" >&2
        exit 1
    fi
    printf 'host_input_grab_chord route=%s target_pid=%s keys=ctrl+shift+g attempts=%s\n' \
        "${input_route}" "$mednafen_ui_pid" "$grab_attempt" >>"$input_trace"
    sleep 0.2
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
if ! trace_files_are_line_delimited "$trace" "$cd_trace" "$memory_trace" "$input_trace" "$main_ram_loader_trace" "$main_ram_consumer_trace" "$main_ram_target_trace" "$spawn_consumer_trace" "$spawn_register_trace" "$rng_consumer_trace" "$rng_code_trace"; then
    printf '%s\n' 'FAIL: Mednafen emitted a literal backslash-n in a trace record' >&2
    exit 1
fi
require_snapshot_size "$vram_snapshot" 65536 'VDC VRAM' || exit 1
require_snapshot_size "$vce_snapshot" 1024 'VCE palette RAM' || exit 1
if [[ ! -s "$vdc_io_trace" ]] ||
   ! grep -Fqx 'FIRESTAFF_THERON_VDC_IO_TRACE_V1' "$vdc_io_trace"; then
    printf '%s\n' 'FAIL: Mednafen did not produce a provenance-marked VDC I/O trace' >&2
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
transition_input_grab_chord_count=$(trace_count '^host_input_grab_chord ' "$input_trace")
transition_host_sdl_event_types=$(trace_event_types "$input_trace")
transition_irq_count=$(trace_count '^pce_cd_irq cpu_pc=' "$cd_trace")
# The low-PC register-read counter is retained as a diagnostic, but it is not
# a valid ownership test: the retail CD path may reach the FIFO through a
# banked HuC6280 routine outside that narrow address expression.  The
# authenticated ownership boundary is instead a byte-exact CD/FIFO -> RAM
# origin receipt.  See docs/source-lock/theron-disassembly/theron-runtime-spawn-capture.md
# and docs/source-lock/theron-fifo-origin-capture-2026-08-06.md.  This admits
# transport only; the RNG/AI/T700/T900 consumers remain fail-closed elsewhere.
transition_non_system_card_count=$(trace_count '^pce_cd_register_read cpu_pc=[0-9a-b][0-9a-f]{3} ' "$cd_trace")
transition_sector_count=$(trace_count '^cd_interface_raw_sector_read ' "$cd_trace")
transition_scsi_read_command_count=$(trace_count '^scsi_read_command ' "$cd_trace")
transition_scsi_sector_binding_count=$(trace_count '^scsi_read_sector_binding ' "$cd_trace")
transition_data_destination_count=$(trace_count '^pce_cd_data_destination_candidate ' "$cd_trace")
transition_adpcm_fifo_read_count=$(trace_count '^pce_cd_fifo_read transport=adpcm ' "$cd_trace")
transition_adpcm_ram_write_count=$(trace_count '^pce_cd_adpcm_ram_write ' "$cd_trace")
transition_adpcm_ram_read_prepare_count=$(trace_count '^pce_cd_adpcm_ram_read_prepare ' "$cd_trace")
transition_adpcm_cpu_read_count=$(trace_count '^pce_cd_adpcm_cpu_read ' "$cd_trace")
transition_origin_ram_receipt_count=$(trace_count '^pce_cd_origin_ram_receipt ' "$cd_trace")
transition_authenticated_cd_ram_count=$(trace_count '^pce_cd_(origin_ram_receipt|fifo_origin_ram_receipt|origin_main_ram_receipt|fifo_origin_main_ram_receipt) ' "$cd_trace")
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
transition_main_ram_e009_enter_count=$(trace_count '^main_ram_e009_enter ' "$cd_trace")
transition_main_ram_e009_data_read_count=$(trace_count '^main_ram_e009_data_read ' "$cd_trace")
transition_main_ram_e009_return_count=$(trace_count '^main_ram_e009_return ' "$cd_trace")
transition_main_ram_e009_register_write_count=$(trace_count '^main_ram_e009_register_write ' "$cd_trace")
transition_main_ram_consumer_read_count=$(trace_count '^main_ram_consumer_read ' "$main_ram_consumer_trace")
transition_main_ram_target_read_count=$(trace_count '^main_ram_target_read ' "$main_ram_target_trace")
transition_main_ram_target_write_count=$(trace_count '^main_ram_target_write ' "$main_ram_target_trace")
transition_spawn_consumer_read_count=$(trace_count '^spawn_consumer_read ' "$spawn_consumer_trace")
transition_spawn_register_sample_count=$(trace_count '^spawn_consumer_registers ' "$spawn_register_trace")
# THQUEST.ASM LB0E5 is a regular-spawn entry only when A is one of the four
# source-defined categories. The same logical address occurs in other bank
# overlays; retain their count as diagnostic address hits but never publish
# them as spawn samples.
transition_spawn_entry_b0e5_address_count=$(trace_count '^spawn_consumer_registers .* pc=b0e5 ' "$spawn_register_trace")
transition_spawn_entry_b0e5_count=$(perl -ne 'if (/^spawn_consumer_registers .* pc=b0e5 .* a=([0-9a-fA-F]+)/ && hex($1) <= 3) { $count++ } END { print $count || 0 }' "$spawn_register_trace" 2>/dev/null || printf '0')
transition_spawn_preconsumer_4644_count=$(trace_count '^spawn_consumer_registers .*preconsumer_4644=1' "$spawn_register_trace")
transition_spawn_helper_4667_count=$(trace_count '^spawn_consumer_registers .*helper_4667=1' "$spawn_register_trace")
transition_spawn_helper_4667_special_count=$(perl -ne 'if (/^spawn_consumer_registers .*helper_4667=1/ && / b3=([0-9a-fA-F]+)/ && ((hex($1) & 7) == 4)) { $count++ } END { print $count || 0 }' "$spawn_register_trace" 2>/dev/null || printf '0')
transition_rng_consumer_sample_count=$(trace_count '^rng_consumer_window ' "$rng_consumer_trace")
transition_rng_code_window_count=$(trace_count '^rng_code_window ' "$rng_code_trace")
transition_scripted_input_count=$(trace_count '^scripted_pce_input_event ' "$input_trace")
{
    printf '%s\n' 'source=authentic-mednafen-transition-receipt'
    printf 'mednafen_module=%s\n' "$capture_mednafen_module"
    printf 'mednafen_binary_md5=%s\n' "$mednafen_binary_md5"
    printf 'track02_mode=%s\n' "$track02_mode"
    printf 'track02_md5=%s\n' "$track02_md5"
    printf 'system_card_md5=%s\n' "$system_card_md5"
    printf 'autoload_state_md5=%s\n' "$autoload_state_md5"
    printf 'input_transactions=%s\n' "$transition_input_count"
    printf 'host_key_events=%s\n' "$transition_host_key_count"
    printf 'host_sdl_events=%s\n' "$transition_host_sdl_event_count"
    printf 'host_sdl_event_types=%s\n' "$transition_host_sdl_event_types"
    printf 'host_window_events=%s\n' "$transition_host_window_event_count"
    printf 'host_focus_state_events=%s\n' "$transition_host_focus_state_count"
    printf 'input_grab_chord_events=%s\n' "$transition_input_grab_chord_count"
    printf 'cd_irq_callbacks=%s\n' "$transition_irq_count"
    printf 'non_system_card_pcecd_reads=%s\n' "$transition_non_system_card_count"
    printf 'raw_sector_spans=%s\n' "$transition_sector_count"
    printf 'scsi_read_commands=%s\n' "$transition_scsi_read_command_count"
    printf 'scsi_read_sector_bindings=%s\n' "$transition_scsi_sector_binding_count"
    printf 'byte_exact_fifo_ram_destinations=%s\n' "$transition_data_destination_count"
    printf 'adpcm_fifo_reads=%s\n' "$transition_adpcm_fifo_read_count"
    printf 'adpcm_ram_writes=%s\n' "$transition_adpcm_ram_write_count"
    printf 'adpcm_ram_read_prepares=%s\n' "$transition_adpcm_ram_read_prepare_count"
    printf 'adpcm_cpu_reads=%s\n' "$transition_adpcm_cpu_read_count"
    printf 'byte_exact_origin_ram_receipts=%s\n' "$transition_origin_ram_receipt_count"
    printf 'authenticated_cd_ram_receipts=%s\n' "$transition_authenticated_cd_ram_count"
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
    printf 'main_ram_e009_enters=%s\n' "$transition_main_ram_e009_enter_count"
    printf 'main_ram_e009_data_reads=%s\n' "$transition_main_ram_e009_data_read_count"
    printf 'main_ram_e009_returns=%s\n' "$transition_main_ram_e009_return_count"
    printf 'main_ram_e009_register_writes=%s\n' "$transition_main_ram_e009_register_write_count"
    printf 'main_ram_consumer_reads=%s\n' "$transition_main_ram_consumer_read_count"
    printf 'main_ram_target_reads=%s\n' "$transition_main_ram_target_read_count"
    printf 'main_ram_target_writes=%s\n' "$transition_main_ram_target_write_count"
    printf 'spawn_consumer_reads=%s\n' "$transition_spawn_consumer_read_count"
    printf 'spawn_register_samples=%s\n' "$transition_spawn_register_sample_count"
    printf 'spawn_preconsumer_4644_samples=%s\n' "$transition_spawn_preconsumer_4644_count"
    printf 'spawn_helper_4667_samples=%s\n' "$transition_spawn_helper_4667_count"
    printf 'spawn_helper_4667_special_branch_samples=%s\n' "$transition_spawn_helper_4667_special_count"
    printf 'spawn_entry_b0e5_address_hits=%s\n' "$transition_spawn_entry_b0e5_address_count"
    printf 'spawn_entry_b0e5_samples=%s\n' "$transition_spawn_entry_b0e5_count"
    printf 'rng_consumer_samples=%s\n' "$transition_rng_consumer_sample_count"
    printf 'rng_consumer_sample_limit=%s\n' "$rng_consumer_sample_limit"
    printf 'rng_consumer_window_limit=%s\n' "$rng_consumer_window_limit"
    printf 'rng_code_windows=%s\n' "$transition_rng_code_window_count"
    printf 'scripted_pce_input_events=%s\n' "$transition_scripted_input_count"
    printf 'vdc_vram_snapshot_bytes=65536\n'
    printf 'vce_palette_snapshot_bytes=1024\n'
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
    if [[ -n "$replay_input_script" ]]; then
        printf 'input_delivery=scripted_pce_replay\n'
        printf 'scripted_pce_input_plan=%s\n' "$replay_input_script"
    fi
    if [[ "$transition_input_count" -gt 0 && "$transition_irq_count" -gt 0 &&
          "$transition_authenticated_cd_ram_count" -gt 0 && "$transition_sector_count" -gt 0 &&
          "$transition_scsi_read_command_count" -gt 0 && "$transition_scsi_sector_binding_count" -gt 0 ]]; then
        printf '%s\n' 'transition=observed'
    else
        printf '%s\n' 'transition=missing'
    fi
} >"$transition_receipt"
if ! verify_scripted_input_masks; then
    exit 1
fi
if [[ "$host_input_requested" == 1 && "$transition_host_key_count" -eq 0 ]]; then
    printf 'BLOCKED: requested host key was not observed by Mednafen SDL dispatch; sdl_events=%s window_events=%s focus_events=%s (exit=%s)\n' "$transition_host_sdl_event_count" "$transition_host_window_event_count" "$transition_host_focus_state_count" "$status"
    exit 1
fi
if ! grep -Fq 'dynamic_cd_read_transaction ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_controller_state ' "$trace" ||
   ! grep -Fq 'dynamic_cd_read_destination_span pc=4093 destination=3800 bytes=32 fnv1a=' "$trace" ||
   ! grep -Fq 'dynamic_huc6260_palette_store ' "$trace"; then
    # Some authentic runs expose the retail CD/FIFO transfer as an origin
    # receipt without the optional high-level dynamic-CD markers.  Do not
    # reject that lower-level, source-bound transport evidence merely because
    # the marker-producing probe was not active.
    if [[ "$transition_sector_count" -gt 0 &&
          "$transition_scsi_read_command_count" -gt 0 &&
          "$transition_scsi_sector_binding_count" -gt 0 &&
          "$transition_authenticated_cd_ram_count" -gt 0 &&
          "$transition_input_count" -gt 0 && "$transition_irq_count" -gt 0 ]]; then
        printf 'INFO: high-level dynamic markers absent; accepting authenticated CD->RAM origin receipts as transport evidence (main_ram_e009_dispatches=%s main_ram_consumer_reads=%s)\n' "$transition_game_main_ram_e009_count" "$transition_main_ram_consumer_read_count" >&2
    else
        if [[ "$transition_sector_count" -gt 0 ]]; then
            printf 'BLOCKED: loader reached authentic raw sectors but no authenticated CD->RAM origin receipt was observed; host_keys=%s input=%s irq=%s authenticated_cd_ram=%s raw_sectors=%s main_ram_e009_dispatches=%s main_ram_e009_enters=%s main_ram_e009_data_reads=%s main_ram_e009_register_writes=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$transition_irq_count" "$transition_authenticated_cd_ram_count" "$transition_sector_count" "$transition_main_ram_loader_e009_dispatch_count" "$transition_main_ram_e009_enter_count" "$transition_main_ram_e009_data_read_count" "$transition_main_ram_e009_register_write_count" "$status"
            exit 1
        fi
        if grep -Fqx 'post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00' "$trace" &&
           grep -Eq '^c860_window_pc=c8c4 .*instruction=LDA \$222D  @ \$222D = \$00( |$)' "$trace" &&
           grep -Eq '^c860_window_pc=c8c7 .*instruction=CMP #\$08' "$trace" &&
           grep -Eq '^c860_window_pc=c8cb .*instruction=CMP #\$04' "$trace" &&
           grep -Eq '^c860_window_pc=c8cd .*instruction=BNE \$C897' "$trace"; then
            printf 'BLOCKED: System Card wait; host_keys=%s input=%s input_after_first_host=%s irq=%s authenticated_cd_ram=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$(awk -F= '/^pce_input_transactions_after_first_host=/{print $2}' "$transition_receipt")" "$transition_irq_count" "$transition_authenticated_cd_ram_count" "$status"
            exit 1
        fi
        printf 'BLOCKED: dynamic receipts absent; host_keys=%s input=%s irq=%s authenticated_cd_ram=%s (exit=%s)\n' "$transition_host_key_count" "$transition_input_count" "$transition_irq_count" "$transition_authenticated_cd_ram_count" "$status"
        exit 1
    fi
fi
if ! grep -Eq '^cd_interface_raw_sector_read lba=[0-9]+ bytes=2352 sector_fnv1a=[0-9a-f]{8} span_offset=0 span_bytes=32 span_fnv1a=[0-9a-f]{8}$' "$cd_trace"; then
    printf 'BLOCKED: dynamic CPU receipts lack a complete authentic raw-sector receipt (exit=%s)\n' "$status"
    exit 1
fi
if [[ "$transition_input_count" -eq 0 || "$transition_irq_count" -eq 0 ||
      "$transition_authenticated_cd_ram_count" -eq 0 ]]; then
    printf 'BLOCKED: raw sector span lacks input, CDIRQ, and authenticated CD->RAM origin receipts (exit=%s)\n' "$status"
    exit 1
fi
if ! awk '
    /^cd_interface_raw_sector_read / { reached_sector = 1 }
    /^pce_cd_(origin_ram_receipt|fifo_origin_ram_receipt|origin_main_ram_receipt|fifo_origin_main_ram_receipt) / {
        if (reached_sector) saw_origin = 1
    }
    END { exit !(reached_sector && saw_origin) }
' "$cd_trace"; then
    printf 'BLOCKED: raw sector span lacks a following authenticated CD->RAM origin receipt (exit=%s)\n' "$status"
    exit 1
fi

printf 'PASS: live trace contains combined CD/controller/HuC6260 and raw-sector span receipts (exit=%s)\n' "$status"
