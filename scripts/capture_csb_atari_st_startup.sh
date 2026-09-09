#!/usr/bin/env bash
# Capture original CSB Atari ST startup frames with Hatari.
#
# This is development-time reference tooling only.  Firestaff never invokes
# Hatari or requires a TOS ROM at runtime.  The caller supplies both the TOS
# ROM and a hash-verified CSB STX image; neither is copied into this repository.

set -euo pipefail

repo="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mode="prepare"

usage() {
    cat <<'EOF'
Usage: capture_csb_atari_st_startup.sh [--prepare|--run]

Required for --run:
  CSB_ATARI_TOS=/path/to/tos162se.img
  CSB_ATARI_STX=/path/to/Chaos-Strikes-Back.stx

Optional:
  CSB_ATARI_CAPTURE_OUT=/path/to/output       (default: .codex-scratch)
  CSB_ATARI_CAPTURE_SECONDS='18 36'           seconds after boot to capture
  CSB_ATARI_XVFB_DISPLAY=103                  dedicated X display number
  HATARI=/path/to/hatari                       (default: hatari)

The script runs a write-protected STE session under a dedicated Xvfb display,
writes only capture images and a SHA-256 receipt to the chosen output folder,
then terminates only the Hatari/Xvfb processes it started.  A produced image is
an original emulator capture, not a Firestaff pixel-parity claim.
EOF
}

case "${1:---prepare}" in
    --prepare) mode="prepare" ;;
    --run) mode="run" ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac

tos="${CSB_ATARI_TOS:-}"
stx="${CSB_ATARI_STX:-}"
out="${CSB_ATARI_CAPTURE_OUT:-$repo/.codex-scratch/csb-atari-startup-capture}"
hatari="${HATARI:-hatari}"
capture_seconds="${CSB_ATARI_CAPTURE_SECONDS:-18 36}"

if [[ "$mode" == "prepare" ]]; then
    usage
    exit 0
fi

for required in "$tos" "$stx"; do
    if [[ -z "$required" || ! -f "$required" ]]; then
        echo "ERROR: CSB_ATARI_TOS and CSB_ATARI_STX must name readable caller-supplied files" >&2
        exit 3
    fi
done
for required in "$hatari" Xvfb scrot sha256sum; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "ERROR: required capture tool is unavailable: $required" >&2
        exit 4
    }
done
if [[ ! "$capture_seconds" =~ ^[0-9]+(\ [0-9]+)*$ ]]; then
    echo "ERROR: CSB_ATARI_CAPTURE_SECONDS must be space-separated non-negative seconds" >&2
    exit 5
fi

mkdir -p "$out"
display_num="${CSB_ATARI_XVFB_DISPLAY:-103}"
if [[ ! "$display_num" =~ ^[0-9]+$ ]]; then
    echo "ERROR: CSB_ATARI_XVFB_DISPLAY must be a numeric display number" >&2
    exit 5
fi
display=":$display_num"

Xvfb "$display" -screen 0 1024x768x24 >"$out/xvfb.log" 2>&1 &
xvfb_pid=$!
hatari_pid=""
cleanup() {
    if [[ -n "$hatari_pid" ]]; then kill "$hatari_pid" 2>/dev/null || true; fi
    kill "$xvfb_pid" 2>/dev/null || true
    if [[ -n "$hatari_pid" ]]; then wait "$hatari_pid" 2>/dev/null || true; fi
    wait "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

DISPLAY="$display" "$hatari" --confirm-quit no --machine ste --tos "$tos" \
    --disk-a "$stx" --protect-floppy on --sound off --fastfdc off \
    >"$out/hatari.log" 2>&1 &
hatari_pid=$!

previous=0
index=0
for second in $capture_seconds; do
    sleep "$((second - previous))"
    index=$((index + 1))
    DISPLAY="$display" scrot --overwrite "$out/startup-${index}-${second}s.png"
    previous=$second
done

{
    printf 'schema=firestaff.csb.atari.startup.capture.v1\n'
    printf 'scope=original Hatari startup capture; no Firestaff parity claim\n'
    printf 'tos_sha256=%s\n' "$(sha256sum "$tos" | awk '{print $1}')"
    printf 'stx_sha256=%s\n' "$(sha256sum "$stx" | awk '{print $1}')"
    for image in "$out"/startup-*.png; do
        printf 'frame_sha256=%s\n' "$(sha256sum "$image" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

echo "PASS: wrote $(find "$out" -maxdepth 1 -name 'startup-*.png' -type f | wc -l | tr -d ' ') original CSB Atari startup frame(s)"
