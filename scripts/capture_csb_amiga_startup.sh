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

All original ADFs are mounted write-protected.  The output holds only host
captures, an ephemeral FS-UAE config and SHA-256 receipt.  A produced image
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
for required in "$fsuae" Xvfb scrot sha256sum; do
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
previous=0
index=0
for second in $capture_seconds; do
    sleep "$((second - previous))"
    index=$((index + 1))
    DISPLAY="$display" scrot --overwrite "$out/startup-${index}-${second}s.png"
    previous=$second
done

{
    printf 'schema=firestaff.csb.amiga.startup.capture.v1\n'
    printf 'scope=original FS-UAE startup capture; no Firestaff parity claim\n'
    printf 'kickstart_sha256=%s\n' "$(sha256sum "$kickstart" | awk '{print $1}')"
    printf 'disk1_sha256=%s\n' "$(sha256sum "$disk1" | awk '{print $1}')"
    printf 'disk2_sha256=%s\n' "$(sha256sum "$disk2" | awk '{print $1}')"
    printf 'disk3_sha256=%s\n' "$(sha256sum "$disk3" | awk '{print $1}')"
    for image in "$out"/startup-*.png; do
        printf 'frame_sha256=%s\n' "$(sha256sum "$image" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

echo "PASS: wrote $(find "$out" -maxdepth 1 -name 'startup-*.png' -type f | wc -l | tr -d ' ') original CSB Amiga startup frame(s)"
