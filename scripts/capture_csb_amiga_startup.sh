#!/usr/bin/env bash
# Capture original CSB Amiga startup frames with FS-UAE.
#
# Development-only reference tooling.  The caller supplies the Kickstart ROM
# and original ADFs; Firestaff neither ships nor loads either at runtime.

set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="${1:---prepare}"

usage() {
    cat <<'EOF'
Usage: capture_csb_amiga_startup.sh [--prepare|--run]

Required for --run:
  CSB_AMIGA_KICKSTART=/path/to/kickstart-1.3.rom
  CSB_AMIGA_DISK1=/path/to/CSB-disk-1.adf
  CSB_AMIGA_DISK2=/path/to/CSB-disk-2.adf
  CSB_AMIGA_DISK3=/path/to/CSB-disk-3.adf

Optional:
  CSB_AMIGA_CAPTURE_OUT=/path/to/output       (default: .codex-scratch)
  CSB_AMIGA_CAPTURE_SECONDS='32 52'           seconds after boot to capture
  CSB_AMIGA_XVFB_DISPLAY=106                  dedicated X display number
  FS_UAE=/path/to/fs-uae                       (default: fs-uae)

All original ADFs are mounted write-protected. The helper accepts only
FS-UAE-native emulator captures as evidence. If an Xvfb/SDL session rejects
the screenshot shortcut it writes a separately named diagnostic host image,
records the failed native request, and exits non-zero. A produced native image
is an original emulator capture, not a Firestaff pixel-parity claim.
EOF
}

case "$mode" in
    --prepare) usage; exit 0 ;;
    --run) ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac

kickstart="${CSB_AMIGA_KICKSTART:-}"
disk1="${CSB_AMIGA_DISK1:-}"
disk2="${CSB_AMIGA_DISK2:-}"
disk3="${CSB_AMIGA_DISK3:-}"
out="${CSB_AMIGA_CAPTURE_OUT:-$repo/.codex-scratch/csb-amiga-startup-capture}"
fsuae="${FS_UAE:-fs-uae}"
capture_seconds="${CSB_AMIGA_CAPTURE_SECONDS:-32 52}"
display_num="${CSB_AMIGA_XVFB_DISPLAY:-106}"

for required in "$kickstart" "$disk1" "$disk2" "$disk3"; do
    if [[ -z "$required" || ! -f "$required" ]]; then
        echo "ERROR: all CSB_AMIGA_KICKSTART and CSB_AMIGA_DISK{1,2,3} files are required" >&2
        exit 3
    fi
done
for required in "$fsuae" Xvfb xdotool scrot sha256sum; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "ERROR: required capture tool is unavailable: $required" >&2
        exit 4
    }
done
if [[ ! "$capture_seconds" =~ ^[0-9]+(\ [0-9]+)*$ ]] || [[ ! "$display_num" =~ ^[0-9]+$ ]]; then
    echo "ERROR: capture seconds and Xvfb display must be numeric" >&2
    exit 5
fi

mkdir -p "$out"
config="$out/csb.fs-uae"
cat >"$config" <<EOF
amiga_model = A500
kickstart_file = $kickstart
floppy_drive_0 = $disk1
floppy_drive_1 = $disk2
floppy_drive_2 = $disk3
floppy_write_protect = 1
fullscreen = 0
window_width = 800
window_height = 600
video_sync = 0
screenshots_output_dir = $out
save_states_dir = $out/save-states
EOF

display=":$display_num"
Xvfb "$display" -screen 0 1024x768x24 >"$out/xvfb.log" 2>&1 &
xvfb_pid=$!
uae_pid=""
cleanup() {
    if [[ -n "$uae_pid" ]]; then kill "$uae_pid" 2>/dev/null || true; fi
    kill "$xvfb_pid" 2>/dev/null || true
    if [[ -n "$uae_pid" ]]; then wait "$uae_pid" 2>/dev/null || true; fi
    wait "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

DISPLAY="$display" "$fsuae" --stdout "$config" >"$out/fs-uae.log" 2>&1 &
uae_pid=$!

find_fsuae_window() {
    local attempt window
    for attempt in $(seq 1 50); do
        window="$(DISPLAY="$display" xdotool search --name 'FS-UAE' 2>/dev/null | head -n 1 || true)"
        if [[ -n "$window" ]]; then
            printf '%s\n' "$window"
            return 0
        fi
        sleep 0.1
    done
    return 1
}

window="$(find_fsuae_window || true)"
if [[ -z "$window" ]]; then
    echo "ERROR: could not find the FS-UAE window for native screenshots" >&2
    exit 6
fi

previous=0
index=0
expected_capture_count=0
native_capture_count=0
for second in $capture_seconds; do
    expected_capture_count=$((expected_capture_count + 1))
    sleep "$((second - previous))"
    index=$((index + 1))
    # FS-UAE emits a full window, a crop and its emulated "real" framebuffer
    # for one screenshot request.  Only the latter is a useful native frame;
    # do not race the writer and rename the first (host/window) file instead.
    before_count="$(find "$out" -maxdepth 1 -type f -name 'fs-uae-real-*.png' | wc -l | tr -d ' ')"
    # F12+S is FS-UAE's documented screenshot shortcut.  It records the
    # emulated Amiga frame and therefore avoids a host Xvfb-root capture.
    DISPLAY="$display" xdotool key --window "$window" F12+s
    for attempt in $(seq 1 50); do
        latest="$(find "$out" -maxdepth 1 -type f -name 'fs-uae-real-*.png' -printf '%T@ %p\n' | sort -n | tail -n 1 | cut -d' ' -f2-)"
        after_count="$(find "$out" -maxdepth 1 -type f -name 'fs-uae-real-*.png' | wc -l | tr -d ' ')"
        if [[ "$after_count" -gt "$before_count" && -n "$latest" ]]; then
            mv "$latest" "$out/startup-${index}-${second}s.png"
            native_capture_count=$((native_capture_count + 1))
            break
        fi
        sleep 0.1
    done
    if [[ ! -f "$out/startup-${index}-${second}s.png" ]]; then
        # Some SDL/Xvfb combinations do not deliver XTest host shortcuts to
        # FS-UAE while its keyboard is grabbed. Keep a diagnostic image for
        # maintainers, but it is deliberately outside the startup-* evidence
        # set and never passes as an original-emulator capture.
        DISPLAY="$display" scrot --overwrite "$out/diagnostic-host-root-${index}-${second}s.png"
    fi
    previous=$second
done

# A partial session is useful for diagnosis, but it is not a successful
# capture of the caller's requested timeline.  In particular, do not allow an
# emulator that exited between two requested timestamps to be reported as a
# complete original-capture result merely because the first host fallback was
# written successfully.
actual_capture_count="$(find "$out" -maxdepth 1 -type f -name 'startup-*.png' | wc -l | tr -d ' ')"

{
    printf 'schema=firestaff.csb.amiga.startup.capture.v1\n'
    printf 'scope=original FS-UAE startup capture; no Firestaff parity claim\n'
    printf 'capture_backend=fs-uae-native\n'
    printf 'requested_native_frames=%s\n' "$expected_capture_count"
    printf 'captured_native_frames=%s\n' "$native_capture_count"
    printf 'kickstart_sha256=%s\n' "$(sha256sum "$kickstart" | awk '{print $1}')"
    printf 'disk1_sha256=%s\n' "$(sha256sum "$disk1" | awk '{print $1}')"
    printf 'disk2_sha256=%s\n' "$(sha256sum "$disk2" | awk '{print $1}')"
    printf 'disk3_sha256=%s\n' "$(sha256sum "$disk3" | awk '{print $1}')"
    for image in "$out"/startup-*.png; do
        printf 'frame_sha256=%s\n' "$(sha256sum "$image" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

if [[ "$actual_capture_count" -ne "$expected_capture_count" ]]; then
    echo "ERROR: requested ${expected_capture_count} CSB Amiga native startup frame(s), captured ${actual_capture_count}; host diagnostics are not evidence" >&2
    exit 7
fi

echo "PASS: wrote $(find "$out" -maxdepth 1 -name 'startup-*.png' -type f | wc -l | tr -d ' ') original CSB Amiga startup frame(s)"
