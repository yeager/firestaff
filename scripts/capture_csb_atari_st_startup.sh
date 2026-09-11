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
  CSB_ATARI_POINTER_CLICKS='70:720,280'       timed emulator-relative clicks
  CSB_ATARI_KEYSTROKES='70:Return'             timed key presses (exclusive with clicks)
  CSB_ATARI_XVFB_DISPLAY=103                  dedicated X display number
  CSB_ATARI_SOUND_HZ=44100                    original-session audio frequency
  CSB_ATARI_CAPTURE_AUDIO=0|1                 record original WAV with Hatari (default: 0)
  HATARI=/path/to/hatari                       (default: hatari)

The script runs a write-protected STE session under a dedicated Xvfb display,
writes only capture images and a SHA-256 receipt to the chosen output folder,
then terminates only the Hatari/Xvfb processes it started. Frames are written
by Hatari's own screenshot facility, rather than by a host desktop grab, so
they exclude emulator chrome and status overlays. A produced image is an
original emulator capture, not a Firestaff pixel-parity claim.
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
pointer_clicks="${CSB_ATARI_POINTER_CLICKS:-}"
keystrokes="${CSB_ATARI_KEYSTROKES:-}"
sound_hz="${CSB_ATARI_SOUND_HZ:-44100}"
capture_audio="${CSB_ATARI_CAPTURE_AUDIO:-0}"

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
for required in "$hatari" Xvfb xdotool sha256sum; do
    command -v "$required" >/dev/null 2>&1 || {
        echo "ERROR: required capture tool is unavailable: $required" >&2
        exit 4
    }
done
if [[ ! "$capture_seconds" =~ ^[0-9]+(\ [0-9]+)*$ ]]; then
    echo "ERROR: CSB_ATARI_CAPTURE_SECONDS must be space-separated non-negative seconds" >&2
    exit 5
fi
if [[ ! "$sound_hz" =~ ^[0-9]+$ ]] || (( sound_hz < 6000 || sound_hz > 50066 )); then
    echo "ERROR: CSB_ATARI_SOUND_HZ must be a Hatari sound frequency from 6000 to 50066" >&2
    exit 5
fi
if [[ "$capture_audio" != "0" && "$capture_audio" != "1" ]]; then
    echo "ERROR: CSB_ATARI_CAPTURE_AUDIO must be 0 or 1" >&2
    exit 5
fi
if [[ -n "$pointer_clicks" ]] && ! [[ "$pointer_clicks" =~ ^[0-9]+:[0-9]+,[0-9]+(\ [0-9]+:[0-9]+,[0-9]+)*$ ]]; then
    echo "ERROR: CSB_ATARI_POINTER_CLICKS must use seconds:x,y entries separated by spaces" >&2
    exit 5
fi
if [[ -n "$keystrokes" ]] && ! [[ "$keystrokes" =~ ^[0-9]+:[A-Za-z0-9_+]+(\ [0-9]+:[A-Za-z0-9_+]+)*$ ]]; then
    echo "ERROR: CSB_ATARI_KEYSTROKES must use seconds:key entries separated by spaces" >&2
    exit 5
fi
if [[ -n "$pointer_clicks" && -n "$keystrokes" ]]; then
    echo "ERROR: use either CSB_ATARI_POINTER_CLICKS or CSB_ATARI_KEYSTROKES per capture run" >&2
    exit 5
fi

mkdir -p "$out"
out="$(cd "$out" && pwd)"
display_num="${CSB_ATARI_XVFB_DISPLAY:-103}"
if [[ ! "$display_num" =~ ^[0-9]+$ ]]; then
    echo "ERROR: CSB_ATARI_XVFB_DISPLAY must be a numeric display number" >&2
    exit 5
fi
display=":$display_num"

Xvfb "$display" -screen 0 1024x768x24 >"$out/xvfb.log" 2>&1 &
xvfb_pid=$!
hatari_pid=""
window=""
audio_capture_active=0
cleanup() {
    # Hatari writes the WAV header length when recording is toggled off.  Do
    # that before process termination whenever a native capture is active.
    if [[ "$audio_capture_active" == "1" && -n "$window" ]]; then
        DISPLAY="$display" xdotool key --window "$window" ISO_Level3_Shift+y \
            >/dev/null 2>&1 || true
        audio_capture_active=0
    fi
    if [[ -n "$hatari_pid" ]]; then kill "$hatari_pid" 2>/dev/null || true; fi
    kill "$xvfb_pid" 2>/dev/null || true
    if [[ -n "$hatari_pid" ]]; then wait "$hatari_pid" 2>/dev/null || true; fi
    wait "$xvfb_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

( cd "$out" && exec env DISPLAY="$display" "$hatari" \
    --confirm-quit no --machine ste --tos "$tos" \
    --disk-a "$stx" --protect-floppy on --sound "$sound_hz" --sound-sync on --fastfdc off \
    --statusbar false --drive-led false --borders false --crop true \
    --screenshot-dir "$out" --screenshot-format png ) \
    >"$out/hatari.log" 2>&1 &
hatari_pid=$!

find_hatari_window() {
    local attempt window
    for attempt in $(seq 1 50); do
        window="$(DISPLAY="$display" xdotool search --name Hatari 2>/dev/null | head -n 1 || true)"
        if [[ -n "$window" ]]; then
            printf '%s\n' "$window"
            return 0
        fi
        sleep 0.1
    done
    return 1
}

window="$(find_hatari_window || true)"
if [[ -z "$window" ]]; then
    echo "ERROR: could not find the Hatari window for native screenshots" >&2
    exit 6
fi

if [[ "$capture_audio" == "1" ]]; then
    # Hatari's documented AltGr+Y recorder writes ./hatari.wav.  The process
    # runs in the caller-selected capture folder, so this can neither escape
    # to a user configuration directory nor become a Firestaff runtime asset.
    if [[ -e "$out/hatari.wav" || -e "$out/startup-audio.wav" ]]; then
        echo "ERROR: audio capture output already exists; use a fresh capture directory" >&2
        exit 5
    fi
    DISPLAY="$display" xdotool key --window "$window" ISO_Level3_Shift+y
    audio_capture_active=1
fi

previous=0
index=0
click_index=0
click_entries=()
key_entries=()
if [[ -n "$pointer_clicks" ]]; then
    read -r -a click_entries <<<"$pointer_clicks"
fi
if [[ -n "$keystrokes" ]]; then
    read -r -a key_entries <<<"$keystrokes"
fi

run_clicks_through() {
    local target="$1" entry timestamp point x y
    while [[ "$click_index" -lt "${#click_entries[@]}" ]]; do
        entry="${click_entries[$click_index]}"
        timestamp="${entry%%:*}"
        point="${entry#*:}"
        if (( timestamp > target )); then
            break
        fi
        if (( timestamp < previous )); then
            echo "ERROR: pointer-click timestamps must be nondecreasing and cannot precede an emitted capture" >&2
            exit 5
        fi
        sleep "$((timestamp - previous))"
        x="${point%,*}"
        y="${point#*,}"
        DISPLAY="$display" xdotool mousemove --window "$window" "$x" "$y" click 1
        previous="$timestamp"
        click_index=$((click_index + 1))
    done
}

run_keys_through() {
    local target="$1" entry timestamp key
    while [[ "$click_index" -lt "${#key_entries[@]}" ]]; do
        entry="${key_entries[$click_index]}"
        timestamp="${entry%%:*}"
        key="${entry#*:}"
        if (( timestamp > target )); then
            break
        fi
        if (( timestamp < previous )); then
            echo "ERROR: key timestamps must be nondecreasing and cannot precede an emitted capture" >&2
            exit 5
        fi
        sleep "$((timestamp - previous))"
        DISPLAY="$display" xdotool key --window "$window" "$key"
        previous="$timestamp"
        click_index=$((click_index + 1))
    done
}

for second in $capture_seconds; do
    if [[ "${#key_entries[@]}" -gt 0 ]]; then
        run_keys_through "$second"
    else
        run_clicks_through "$second"
    fi
    sleep "$((second - previous))"
    index=$((index + 1))
    before_count="$(find "$out" -maxdepth 1 -type f -name 'grab*.png' | wc -l | tr -d ' ')"
    # Hatari documents AltGr+G as its screenshot shortcut.  This writes the
    # emulated framebuffer, avoiding an Xvfb root-window grab with unrelated
    # pixels or an emulator status bar.
    DISPLAY="$display" xdotool key --window "$window" ISO_Level3_Shift+g
    for attempt in $(seq 1 50); do
        latest="$(find "$out" -maxdepth 1 -type f -name 'grab*.png' -printf '%T@ %p\n' | sort -n | tail -n 1 | cut -d' ' -f2-)"
        after_count="$(find "$out" -maxdepth 1 -type f -name 'grab*.png' | wc -l | tr -d ' ')"
        if [[ "$after_count" -gt "$before_count" && -n "$latest" ]]; then
            mv "$latest" "$out/startup-${index}-${second}s.png"
            break
        fi
        sleep 0.1
    done
    if [[ ! -f "$out/startup-${index}-${second}s.png" ]]; then
        echo "ERROR: Hatari did not produce screenshot $index at ${second}s" >&2
        exit 7
    fi
    previous=$second
done

action_count="${#click_entries[@]}"
if [[ "${#key_entries[@]}" -gt 0 ]]; then action_count="${#key_entries[@]}"; fi
if [[ "$click_index" -ne "$action_count" ]]; then
    echo "ERROR: input-action timestamp exceeds the capture timeline" >&2
    exit 5
fi

if [[ "$audio_capture_active" == "1" ]]; then
    DISPLAY="$display" xdotool key --window "$window" ISO_Level3_Shift+y
    audio_capture_active=0
    if [[ ! -s "$out/hatari.wav" ]]; then
        echo "ERROR: Hatari did not produce the requested WAV capture" >&2
        exit 7
    fi
    mv "$out/hatari.wav" "$out/startup-audio.wav"
fi

audio_emulation_warning_count="$(grep -c 'sound samples were not correctly emulated' "$out/hatari.log" || true)"
if [[ "$capture_audio" == "1" && "$audio_emulation_warning_count" != "0" ]]; then
    echo "ERROR: Hatari reported dropped sound samples; reject this WAV as non-parity evidence" >&2
    exit 7
fi

{
    printf 'schema=firestaff.csb.atari.startup.capture.v1\n'
    printf 'scope=original Hatari startup capture; no Firestaff parity claim\n'
    printf 'audio_requested_hz=%s\n' "$sound_hz"
    if [[ -f "$out/startup-audio.wav" ]]; then
        printf 'audio_capture=hatari-wav\n'
        printf 'audio_sha256=%s\n' "$(sha256sum "$out/startup-audio.wav" | awk '{print $1}')"
        printf 'audio_bytes=%s\n' "$(wc -c < "$out/startup-audio.wav" | tr -d ' ')"
    else
        printf 'audio_capture=not-recorded\n'
    fi
    printf 'audio_emulation_warning_count=%s\n' "$audio_emulation_warning_count"
    printf 'tos_sha256=%s\n' "$(sha256sum "$tos" | awk '{print $1}')"
    printf 'stx_sha256=%s\n' "$(sha256sum "$stx" | awk '{print $1}')"
    for image in "$out"/startup-*.png; do
        printf 'frame_sha256=%s\n' "$(sha256sum "$image" | awk '{print $1}')"
    done
} >"$out/receipt.txt"

echo "PASS: wrote $(find "$out" -maxdepth 1 -name 'startup-*.png' -type f | wc -l | tr -d ' ') original CSB Atari startup frame(s)"
