#!/usr/bin/env bash
# Capture original DM1/CSB/DM2 FM Towns frames using Tsugaru's own framebuffer
# command, never a desktop/compositor screenshot.
#
# Development evidence only.  Firestaff does not invoke Tsugaru, require a
# BIOS, or extract any game archive at runtime.  The caller supplies original
# media; staged payloads and resulting copyrighted captures remain untracked
# beneath .codex-scratch.

set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:---prepare}"
game="${FMTOWNS_GAME:-dm1}"
archive="${FMTOWNS_ARCHIVE:-}"
rom_dir="${FMTOWNS_ROM_DIR:-}"
tsugaru="${FMTOWNS_TSUGARU:-Tsugaru_CUI}"
out="${FMTOWNS_CAPTURE_OUT:-$repo/.codex-scratch/fmtowns-original-startup-capture}"
stage="${FMTOWNS_STAGE_DIR:-$repo/.codex-scratch/fmtowns-original-media-stage}"
timeline="${FMTOWNS_CAPTURE_TIMELINE:-}"
input_timeline="${FMTOWNS_INPUT_TIMELINE:-}"
pointer_clicks="${FMTOWNS_POINTER_CLICKS:-}"
towns_type="${FMTOWNS_TYPE:-MX}"
boot_key="${FMTOWNS_BOOT_KEY:-}"
high_fidelity="${FMTOWNS_HIGH_FIDELITY:-0}"
nowait_boot="${FMTOWNS_NOWAIT_BOOT:-1}"
diagnostics="${FMTOWNS_DIAGNOSTICS:-0}"
diff_mouse="${FMTOWNS_DIFF_MOUSE:-0}"
no_wait="${FMTOWNS_NOWAIT:-0}"
frequency_mhz="${FMTOWNS_FREQ_MHZ:-0}"
xvfb_display_num="${FMTOWNS_XVFB_DISPLAY:-170}"

usage() {
    cat <<'EOF'
Usage: capture_fmtowns_original_startup.sh [--prepare|--run]

Required for --run:
  FMTOWNS_GAME=dm1|csb|dm2
  FMTOWNS_ARCHIVE=/path/to/original-fm-towns.zip
  FMTOWNS_ROM_DIR=/path/to/extracted-fm-towns-rom-directory
  FMTOWNS_CAPTURE_TIMELINE='host-seconds:label [host-seconds:label ...]'

Optional:
  FMTOWNS_TSUGARU=/path/to/Tsugaru_CUI    (default: Tsugaru_CUI on PATH)
  FMTOWNS_CAPTURE_OUT=/safe/output/path   (default: repository .codex-scratch)
  FMTOWNS_STAGE_DIR=/safe/staging/path    (default: repository .codex-scratch)
  FMTOWNS_TYPE=MX                          (FM Towns machine type)
  FMTOWNS_BOOT_KEY=CD                      (optional boot key; default is the ROM's normal boot path)
  FMTOWNS_HIGH_FIDELITY=1|0                (default: 0; opt in only after VM boot validation)
  FMTOWNS_NOWAIT_BOOT=1|0                  (default: 1; bypass host-time memory-test delay)
  FMTOWNS_NOWAIT=1|0                       (default: 0; diagnostic unthrottled VM run, recorded in receipt)
  FMTOWNS_FREQ_MHZ=0|1..200                (default: 0; diagnostic emulated CPU frequency, recorded in receipt)
  FMTOWNS_DIAGNOSTICS=1|0                  (default: 0; log emulated CRTC/CD state at each frame)
  FMTOWNS_DIFF_MOUSE=1|0                   (default: 0; enable Tsugaru's differential
                                             mouse integration for original desktop routes)
  FMTOWNS_INPUT_TIMELINE='host-seconds:enter|e|up|down|left|right [...]'
                                             (default: empty; opt-in original-route input;
                                             injected as X11 key events into Tsugaru)
  FMTOWNS_POINTER_CLICKS='host-seconds:x,y[@milliseconds] [...]'
                                             (default: empty; original 640x480
                                             framebuffer coordinates, private X input)
  FMTOWNS_XVFB_DISPLAY=170                 (default: 170; private Xvfb display for Tsugaru CUI)

The ZIP is staged only for this development-time emulator session because
Tsugaru requires a seekable CUE plus BIN or IMG track image.  Timeline values are
host-wall-clock delays after `RUN`, not asserted guest-time positions: a busy
or throttled emulator may reach a different guest frame at the same value.
The archive is never modified, the
stage must live beneath .codex-scratch, and every result receives hashes for
the archive, selected CUE/track image and ROM files.  Images are produced by Tsugaru's
`SS` command from its emulated framebuffer.  Host desktop captures are not
accepted.  A successful capture is original-emulator evidence only; it is not
a Firestaff pixel-parity claim.
EOF
}

case "$mode" in
    --prepare) usage; exit 0 ;;
    --run) ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac

if [[ "$game" != "dm1" && "$game" != "csb" && "$game" != "dm2" ]]; then
    echo "ERROR: FMTOWNS_GAME must be dm1, csb, or dm2" >&2
    exit 2
fi
if [[ -z "$archive" || ! -f "$archive" || "${archive,,}" != *.zip ]]; then
    echo "ERROR: FMTOWNS_ARCHIVE must name the original FM Towns ZIP" >&2
    exit 3
fi
if [[ -z "$rom_dir" || ! -d "$rom_dir" ]]; then
    echo "ERROR: FMTOWNS_ROM_DIR must name an extracted, caller-supplied ROM directory" >&2
    exit 3
fi
if [[ -z "$timeline" || ! "$timeline" =~ ^[0-9]+:[A-Za-z0-9_-]+(\ [0-9]+:[A-Za-z0-9_-]+)*$ ]]; then
    echo "ERROR: FMTOWNS_CAPTURE_TIMELINE must use seconds:label entries separated by spaces" >&2
    exit 3
fi
if [[ -n "$input_timeline" && ! "$input_timeline" =~ ^[0-9]+:(enter|e|up|down|left|right)(\ [0-9]+:(enter|e|up|down|left|right))*$ ]]; then
    echo "ERROR: FMTOWNS_INPUT_TIMELINE accepts enter, e, up, down, left, or right entries" >&2
    exit 3
fi
if [[ -n "$input_timeline" ]]; then
    last_input_entry="${input_timeline##* }"
    last_capture_entry="${timeline##* }"
    if (( ${last_input_entry%%:*} > ${last_capture_entry%%:*} )); then
        echo "ERROR: input timestamp occurs after the final capture timestamp" >&2
        exit 3
    fi
fi
if [[ -n "$pointer_clicks" && ! "$pointer_clicks" =~ ^[0-9]+:[0-9]+,[0-9]+(@[0-9]+)?(\ [0-9]+:[0-9]+,[0-9]+(@[0-9]+)?)*$ ]]; then
    echo "ERROR: FMTOWNS_POINTER_CLICKS must use seconds:x,y or seconds:x,y@milliseconds entries separated by spaces" >&2
    exit 3
fi
if [[ "$high_fidelity" != "0" && "$high_fidelity" != "1" ]]; then
    echo "ERROR: FMTOWNS_HIGH_FIDELITY must be 0 or 1" >&2
    exit 3
fi
if [[ -n "$boot_key" && ! "$boot_key" =~ ^(CD|F0|F1|F2|F3|H0|H1|H2|H3|H4|ICM|DEBUG|PAD_A|PAD_B|PAD_AB|FASTMODE|SLOWMODE)$ ]]; then
    echo "ERROR: FMTOWNS_BOOT_KEY is not a Tsugaru boot-key selector" >&2
    exit 3
fi
if [[ "$nowait_boot" != "0" && "$nowait_boot" != "1" ]]; then
    echo "ERROR: FMTOWNS_NOWAIT_BOOT must be 0 or 1" >&2
    exit 3
fi
if [[ "$diagnostics" != "0" && "$diagnostics" != "1" ]]; then
    echo "ERROR: FMTOWNS_DIAGNOSTICS must be 0 or 1" >&2
    exit 3
fi
if [[ "$diff_mouse" != "0" && "$diff_mouse" != "1" ]]; then
    echo "ERROR: FMTOWNS_DIFF_MOUSE must be 0 or 1" >&2
    exit 3
fi
if [[ "$no_wait" != "0" && "$no_wait" != "1" ]]; then
    echo "ERROR: FMTOWNS_NOWAIT must be 0 or 1" >&2
    exit 3
fi
if [[ ! "$frequency_mhz" =~ ^[0-9]+$ ]] || (( frequency_mhz > 200 )); then
    echo "ERROR: FMTOWNS_FREQ_MHZ must be 0 or an emulated frequency from 1 to 200 MHz" >&2
    exit 3
fi
if [[ ! "$xvfb_display_num" =~ ^[1-9][0-9]*$ ]]; then
    echo "ERROR: FMTOWNS_XVFB_DISPLAY must be a positive display number" >&2
    exit 3
fi
seven_zip="$(command -v 7zz 2>/dev/null || command -v 7z 2>/dev/null || true)"
for required in "$tsugaru" "$seven_zip" sha256sum python3 Xvfb; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "ERROR: required capture tool is unavailable: $required" >&2
        exit 4
    }
done
if [[ -n "$pointer_clicks" || -n "$input_timeline" ]] && ! command -v xdotool >/dev/null 2>&1; then
    echo "ERROR: xdotool is required when FMTOWNS_POINTER_CLICKS or FMTOWNS_INPUT_TIMELINE is set" >&2
    exit 4
fi
python3 -c 'from PIL import Image' >/dev/null 2>&1 || {
    echo "ERROR: Pillow is required to validate Tsugaru framebuffer PNGs" >&2
    exit 4
}

scratch_root="$repo/.codex-scratch"
mkdir -p "$scratch_root"
out="$(mkdir -p "$out" && cd "$out" && pwd)"
stage="$(mkdir -p "$stage" && cd "$stage" && pwd)"
case "$out" in "$scratch_root"/*) ;; *) echo "ERROR: capture output must remain beneath .codex-scratch" >&2; exit 3;; esac
case "$stage" in "$scratch_root"/*) ;; *) echo "ERROR: media stage must remain beneath .codex-scratch" >&2; exit 3;; esac
if find "$out" -mindepth 1 -maxdepth 1 -print -quit | grep -q .; then
    echo "ERROR: capture output must be empty; use a fresh directory" >&2
    exit 3
fi

# The exact CUE/track-image pair is selected from real media, not guessed from a
# filename.  Staging outside the source tree prevents game payloads entering
# a commit by accident.
if find "$stage" -mindepth 1 -maxdepth 1 -print -quit | grep -q .; then
    echo "ERROR: media stage must be empty; use a fresh .codex-scratch directory" >&2
    exit 3
fi
"$seven_zip" x -y "-o$stage" "$archive" >/dev/null
mapfile -d '' cue_files < <(find "$stage" -type f -iname '*.cue' -print0)
mapfile -d '' track_files < <(find "$stage" -type f \( -iname '*.bin' -o -iname '*.img' \) -print0)
if [[ "${#cue_files[@]}" -ne 1 || "${#track_files[@]}" -ne 1 ]]; then
    echo "ERROR: original archive must contain exactly one CUE and one BIN or IMG track image" >&2
    exit 5
fi
cue="${cue_files[0]}"
track_image="${track_files[0]}"

# Tsugaru's Linux CUI resolves ROM names with exact upper-case filenames,
# whereas verified user ROM sets commonly use lower-case names.  Build a
# private, symlink-only case-normalized view; never rename or copy firmware.
rom_stage="$out/rom-case-normalized"
mkdir -p "$rom_stage"
for expected_rom in FMT_SYS.ROM FMT_DOS.ROM FMT_FNT.ROM FMT_F20.ROM FMT_DIC.ROM; do
    mapfile -d '' matches < <(find "$rom_dir" -maxdepth 1 -type f -iname "$expected_rom" -print0)
    if [[ "${#matches[@]}" -ne 1 ]]; then
        echo "ERROR: ROM directory must contain exactly one case-insensitive $expected_rom" >&2
        exit 5
    fi
    ln -s "${matches[0]}" "$rom_stage/$expected_rom"
done

# Record each command before execution.  Tsugaru's CUI command interpreter
# runs concurrently with the VM, so timed `SS` requests sample the emulated
# framebuffer directly and do not depend on SDL/X11 focus or cursor state.
# Their timeline is intentionally recorded as host wall time, not guest time.
# CUI normally auto-starts, but send RUN as the first guest-side command so
# the capture transcript records an explicit run boundary.  Do not sleep or
# issue SS before that boundary: a paused/reset VM can otherwise produce a
# sequence of valid PNG files containing only the black CRTC surface.
previous=0
index=0
command_file="$out/tsugaru-capture-commands.txt"
{
    printf 'RUN\n'
    for entry in $timeline; do
        second="${entry%%:*}"
        label="${entry#*:}"
        if (( second < previous )); then
            echo "ERROR: capture timestamps must be nondecreasing" >&2
            exit 5
        fi
        index=$((index + 1))
        printf 'sleep %s\n' "$((second - previous))"
        if [[ "$diagnostics" == "1" ]]; then
            # Status includes the VM's current time/register state.  Keep it
            # in the private log so a host-time capture can be correlated
            # with guest progress without promoting either as parity data.
            printf 'STA\n'
            printf 'DUMP CRTC\n'
            printf 'DUMP CDROM\n'
        fi
        printf 'SS "%s/startup-%02d-%ss-%s.png"\n' "$out" "$index" "$second" "$label"
        previous="$second"
    done
    # Use the CUI's orderly VM shutdown after the last framebuffer command.
    # FORCEQUIT calls exit(0) directly from the command interpreter and has
    # been observed to race the VM thread on Linux; a signal or crash after
    # otherwise-written PNGs must never be promoted to capture evidence.
    printf 'QUIT\n'
} >"$command_file"

run_commands() {
    while IFS= read -r command; do
        case "$command" in
            sleep\ *) sleep "${command#sleep }" ;;
            *) printf '%s\n' "$command" ;;
        esac
    done <"$command_file"
}

# The Linux CUI initializes SDL even though `SS` reads Tsugaru's emulated
# framebuffer.  Give that initialization a private headless display; do not
# use it for screenshots.  The trap owns only this process and cannot affect
# unrelated developer X servers.
xvfb_display=":$xvfb_display_num"
Xvfb "$xvfb_display" -screen 0 1024x768x24 -nolisten tcp >"$out/xvfb.log" 2>&1 &
xvfb_pid=$!
cleanup_xvfb() {
    if [[ -n "${input_pid:-}" ]] && kill -0 "$input_pid" 2>/dev/null; then
        kill "$input_pid" 2>/dev/null || true
        wait "$input_pid" 2>/dev/null || true
    fi
    if [[ -n "${pointer_pid:-}" ]] && kill -0 "$pointer_pid" 2>/dev/null; then
        kill "$pointer_pid" 2>/dev/null || true
        wait "$pointer_pid" 2>/dev/null || true
    fi
    if kill -0 "$xvfb_pid" 2>/dev/null; then
        kill "$xvfb_pid" 2>/dev/null || true
        wait "$xvfb_pid" 2>/dev/null || true
    fi
}
trap cleanup_xvfb EXIT INT TERM
sleep 0.2
if ! kill -0 "$xvfb_pid" 2>/dev/null; then
    echo "ERROR: unable to start private Xvfb display $xvfb_display; see xvfb.log" >&2
    exit 6
fi

pointer_pid=""
input_pid=""
find_tsugaru_window() {
    local attempt window=""
    for attempt in $(seq 1 80); do
        window="$(DISPLAY="$xvfb_display" xdotool search --name 'Tsugaru' 2>/dev/null | head -n 1 || true)"
        [[ -n "$window" ]] && break
        sleep 0.1
    done
    [[ -n "$window" ]] || return 1
    printf '%s\n' "$window"
}
run_pointer_clicks() {
    local previous=0 entry timestamp point held_ms hold_seconds window
    local x y
    local -a entries
    read -r -a entries <<<"$pointer_clicks"
    for entry in "${entries[@]}"; do
        timestamp="${entry%%:*}"
        point="${entry#*:}"
        held_ms=0
        if [[ "$point" == *@* ]]; then
            held_ms="${point##*@}"
            point="${point%@*}"
        fi
        if (( timestamp < previous )); then
            echo "ERROR: pointer timestamps must be nondecreasing" >&2
            return 1
        fi
        sleep "$((timestamp - previous))"
        if ! window="$(find_tsugaru_window)"; then
            echo "ERROR: could not find Tsugaru window for pointer action" >&2
            return 1
        fi
        x="${point%,*}"; y="${point#*,}"
        # Tsugaru's CUI screenshots are 640x480 framebuffer coordinates.
        # `--window` makes xdotool interpret the supplied coordinates in the
        # SDL client area, avoiding host-desktop focus or scaling ambiguity.
        DISPLAY="$xvfb_display" xdotool windowfocus --sync "$window"
        DISPLAY="$xvfb_display" xdotool mousemove --window "$window" "$x" "$y"
        if (( held_ms > 0 )); then
            hold_seconds="$(awk -v milliseconds="$held_ms" 'BEGIN { printf "%.3f", milliseconds / 1000 }')"
            DISPLAY="$xvfb_display" xdotool mousedown 1
            sleep "$hold_seconds"
            DISPLAY="$xvfb_display" xdotool mouseup 1
        else
            DISPLAY="$xvfb_display" xdotool click 1
        fi
        printf 'pointer=%s:%s@%sms window=%s\n' "$timestamp" "$point" "$held_ms" "$window" >>"$out/pointer-actions.log"
        previous="$timestamp"
    done
}
run_input_events() {
    local previous=0 entry timestamp key window
    local -a entries
    read -r -a entries <<<"$input_timeline"
    for entry in "${entries[@]}"; do
        timestamp="${entry%%:*}"
        key="${entry#*:}"
        if (( timestamp < previous )); then
            echo "ERROR: input timestamps must be nondecreasing" >&2
            return 1
        fi
        sleep "$((timestamp - previous))"
        if ! window="$(find_tsugaru_window)"; then
            echo "ERROR: could not find Tsugaru window for keyboard action" >&2
            return 1
        fi
        DISPLAY="$xvfb_display" xdotool windowfocus --sync "$window"
        case "$key" in
            enter) DISPLAY="$xvfb_display" xdotool key --window "$window" Return ;;
            e) DISPLAY="$xvfb_display" xdotool key --window "$window" e ;;
            up) DISPLAY="$xvfb_display" xdotool key --window "$window" Up ;;
            down) DISPLAY="$xvfb_display" xdotool key --window "$window" Down ;;
            left) DISPLAY="$xvfb_display" xdotool key --window "$window" Left ;;
            right) DISPLAY="$xvfb_display" xdotool key --window "$window" Right ;;
        esac
        printf 'key=%s:%s window=%s\n' "$timestamp" "$key" "$window" >>"$out/input-actions.log"
        previous="$timestamp"
    done
}
if [[ -n "$pointer_clicks" ]]; then
    run_pointer_clicks &
    pointer_pid=$!
fi
if [[ -n "$input_timeline" ]]; then
    run_input_events &
    input_pid=$!
fi

set +e
# Tsugaru's first positional argument is its ROM directory.  It is not an
# option: passing a made-up -ROMDIR flag would silently turn that directory
# into an invalid option and leave a false failed-capture trail.
fidelity_args=()
if [[ "$high_fidelity" == "1" ]]; then fidelity_args=(-HIGHFIDELITY); fi
boot_args=()
if [[ -n "$boot_key" ]]; then boot_args+=(-BOOTKEY "$boot_key"); fi
# Keep independently selected boot options rather than overwriting the
# optional boot key.  The normal path intentionally passes no -BOOTKEY.
if [[ "$nowait_boot" == "1" ]]; then boot_args+=(-NOWAITBOOT); fi
if [[ "$no_wait" == "1" ]]; then boot_args+=(-NOWAIT); fi
if [[ "$frequency_mhz" != "0" ]]; then boot_args+=(-FREQ "$frequency_mhz"); fi
if [[ "$diff_mouse" == "1" ]]; then boot_args+=(-DIFFMOUSE); fi
run_commands | env DISPLAY="$xvfb_display" "$tsugaru" "$rom_stage" -CD "$cue" "${fidelity_args[@]}" "${boot_args[@]}" \
    -TOWNSTYPE "$towns_type" -FORCEQUITONPOFF >"$out/tsugaru.log" 2>&1
tsugaru_status=${PIPESTATUS[1]}
set -e
if [[ -n "$pointer_pid" ]]; then
    wait "$pointer_pid"
fi
if [[ -n "$input_pid" ]]; then
    wait "$input_pid"
fi
if [[ "$tsugaru_status" -ne 0 ]]; then
    echo "ERROR: Tsugaru exited with status $tsugaru_status; see tsugaru.log" >&2
    exit 6
fi
if grep -q 'VM Aborted!' "$out/tsugaru.log"; then
    echo "ERROR: Tsugaru aborted the VM; no frame from this attempt is evidence" >&2
    exit 6
fi

expected="$(wc -w <<<"$timeline" | tr -d ' ')"
actual="$(find "$out" -maxdepth 1 -type f -name 'startup-*.png' | wc -l | tr -d ' ')"
if [[ "$actual" -ne "$expected" ]]; then
    echo "ERROR: Tsugaru produced $actual/$expected framebuffer capture(s)" >&2
    exit 7
fi

python3 - "$out" "$expected" <<'PY'
import hashlib
from pathlib import Path
from PIL import Image
import sys

out = Path(sys.argv[1])
expected = int(sys.argv[2])
frames = sorted(out.glob("startup-*.png"))
if len(frames) != expected:
    raise SystemExit("ERROR: frame list changed during validation")
digests = {}
for frame in frames:
    with Image.open(frame) as image:
        if image.width < 1 or image.height < 1:
            raise SystemExit(f"ERROR: empty framebuffer image: {frame.name}")
        rgb = image.convert("RGB")
        colors = rgb.getcolors(maxcolors=257)
        if not colors or len(colors) <= 1 or all(pixel == (0, 0, 0) for _, pixel in colors):
            raise SystemExit(f"ERROR: blank/stale framebuffer image is not original capture evidence: {frame.name}")
    digest = hashlib.sha256(frame.read_bytes()).hexdigest()
    if digest in digests:
        raise SystemExit(
            "ERROR: duplicate framebuffer samples are not temporal capture evidence: "
            f"{digests[digest]} and {frame.name}")
    digests[digest] = frame.name
PY

# `STA` is deliberately emitted only in diagnostics mode.  Preserve its last
# guest-clock sample in the receipt when available; otherwise state that the
# clock was not sampled instead of deriving guest time from host delays.
guest_time_ns="$(awk '/Towns TIME \(Nano-Seconds\):/ { value=$NF } END { print value }' "$out/tsugaru.log")"
if [[ ! "$guest_time_ns" =~ ^[0-9]+$ ]]; then
    guest_time_ns="not-recorded"
fi

{
    printf 'schema=firestaff.fmtowns.original.capture.v1\n'
    printf 'scope=original Tsugaru framebuffer capture; no Firestaff parity claim\n'
    printf 'game=%s\n' "$game"
    printf 'capture_backend=tsugaru-cui-SS\n'
    printf 'timeline_clock=host_wall_seconds_after_RUN; not_guest_time\n'
    printf 'timeline_requested=%s\n' "$timeline"
    printf 'input_timeline_requested=%s\n' "${input_timeline:-none}"
    printf 'pointer_timeline_requested=%s\n' "${pointer_clicks:-none}"
    printf 'cursor_policy=host_cursor_excluded_by_emulated_framebuffer_capture\n'
    printf 'towns_type=%s\n' "$towns_type"
    printf 'boot_key=%s\n' "${boot_key:-normal}"
    printf 'high_fidelity=%s\n' "$high_fidelity"
    printf 'nowait_boot=%s\n' "$nowait_boot"
    printf 'nowait=%s\n' "$no_wait"
    printf 'frequency_mhz=%s\n' "$frequency_mhz"
    printf 'diagnostics=%s\n' "$diagnostics"
    printf 'differential_mouse=%s\n' "$diff_mouse"
    printf 'guest_time_ns_at_last_diagnostic=%s\n' "$guest_time_ns"
    printf 'archive_sha256=%s\n' "$(sha256sum "$archive" | awk '{print $1}')"
    printf 'cue_sha256=%s\n' "$(sha256sum "$cue" | awk '{print $1}')"
    printf 'track_image_sha256=%s\n' "$(sha256sum "$track_image" | awk '{print $1}')"
    printf 'tsugaru_sha256=%s\n' "$(sha256sum "$(command -v "$tsugaru")" | awk '{print $1}')"
    while IFS= read -r -d '' rom; do
        printf 'rom_sha256=%s\n' "$(sha256sum "$rom" | awk '{print $1}')"
    done < <(find "$rom_dir" -maxdepth 1 -type f -name '*.rom' -print0 | sort -z)
    for frame in "$out"/startup-*.png; do
        printf 'frame=%s sha256=%s\n' "$(basename "$frame")" \
            "$(sha256sum "$frame" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

echo "PASS: wrote $actual original $game FM Towns framebuffer capture(s)"
