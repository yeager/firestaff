#!/usr/bin/env bash
# External-only launcher for the raw VDP1/VDP2 witness producer.
# It never copies BIOS, discs, or capture bytes into the repository.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --trace PATH --validator PATH --manifest PATH [--mednafen-home PATH] [--no-waiting] [--timeout-seconds DEC] [--skip-frames DEC] [--frame-limit DEC] [--press-start-frame DEC] [--press-start-length DEC] [--press-button-mask DEC/HEX]" >&2
}

hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
require_hash() {
  [[ -f "$1" && "${#2}" -eq 64 && "$2" =~ ^[[:xdigit:]]+$ ]] || return 1
  [[ "$(lower "$(hash_file "$1")")" == "$(lower "$2")" ]]
}
require_disc_container() {
  case "${1##*/}" in
    *.cue|*.ccd|*.toc|*.m3u) ;;
    *) echo "ERROR: Saturn capture requires a CUE/CCD/TOC/M3U container" >&2; return 1 ;;
  esac
}

run_validator() {
  case "$validator" in
    *.py) python3 "$validator" "$@" ;;
    *) "$validator" "$@" ;;
  esac
}

capture_child_pid=
capture_timeout_pid=
cleanup_capture_child() {
  local status=$?
  if [[ -n "$capture_timeout_pid" ]] && kill -0 "$capture_timeout_pid" 2>/dev/null; then
    kill -TERM "$capture_timeout_pid" 2>/dev/null || true
  fi
  if [[ -n "$capture_child_pid" ]] && kill -0 "$capture_child_pid" 2>/dev/null; then
    kill -TERM "$capture_child_pid" 2>/dev/null || true
    sleep 1
    kill -KILL "$capture_child_pid" 2>/dev/null || true
  fi
  capture_timeout_pid=
  capture_child_pid=
  return "$status"
}

launch=0
operator_only=0
skip_frames=0
frame_limit=2
press_start_frame=0
press_start_length=1
press_button_mask=0x10
timeout_seconds=0
mednafen_home=
no_waiting=0
bios_region=eu
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--trace|--validator|--manifest|--mednafen-home|--timeout-seconds|--skip-frames|--frame-limit|--press-start-frame|--press-start-length|--press-button-mask)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    --no-waiting) no_waiting=1; shift ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${disc:?}" "${disc_sha256:?}" \
  "${trace:?}" "${validator:?}" "${manifest:?}"
[[ -x "$mednafen" && -f "$validator" ]] || exit 1
if [[ -n "$mednafen_home" && ! -d "$mednafen_home" ]]; then
  echo "ERROR: --mednafen-home must name an existing directory" >&2
  exit 1
fi
require_hash "$bios" "$bios_sha256" || exit 1
require_hash "$disc" "$disc_sha256" || exit 1
require_disc_container "$disc" || exit 1
case "$bios_region" in
  us|eu) bios_option=-ss.bios_na_eu ;;
  jp) bios_option=-ss.bios_jp ;;
  *) echo "ERROR: --bios-region must be us, eu, or jp" >&2; exit 1 ;;
esac
[[ "$skip_frames" =~ ^[0-9]+$ && "$frame_limit" =~ ^[1-9][0-9]*$ &&
   "$timeout_seconds" =~ ^[0-9]+$ &&
   "$press_start_frame" =~ ^[0-9]+$ && "$press_start_length" =~ ^[1-9][0-9]*$ &&
   "$press_button_mask" =~ ^(0[xX])?[0-9a-fA-F]+$ ]] || exit 1
[[ ! -e "$trace" && ! -e "$manifest" && "$trace" != "$manifest" ]] || exit 1
[[ -d "$(dirname "$trace")" && -d "$(dirname "$manifest")" ]] || exit 1
if ((launch)); then
  # With pipefail, grep -q can make strings exit on SIGPIPE after the match.
  strings "$mednafen" | grep -F 'FIRESTAFF_NEXUS_TRACE_OUTPUT' >/dev/null || {
    echo "ERROR: instrumented Mednafen hook is missing" >&2
    exit 78
  }
fi

manifest_tmp="$manifest.tmp.$$"
umask 077
{
  printf 'FIRESTAFF_NEXUS_SATURN_RAW_CAPTURE_PLAN_V1\n'
  printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\nskip_frames=%s\nframe_limit=%s\npress_start_frame=%s\npress_start_length=%s\npress_button_mask=%s\n' \
    "$(lower "$bios_sha256")" "$bios_region" "$(lower "$disc_sha256")" "$skip_frames" "$frame_limit" "$press_start_frame" "$press_start_length" "$press_button_mask"
  printf 'mednafen_home=%s\nno_waiting=%s\ntimeout_seconds=%s\n' "${mednafen_home:-}" "$no_waiting" "$timeout_seconds"
  printf 'capture_magic=FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n'
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios=%q\n' "$bios"
printf 'bios_region=%s\n' "$bios_region"
printf 'disc=%q\n' "$disc"
printf 'trace=%q\n' "$trace"
printf 'manifest=%q\n' "$manifest"
printf 'command=%q -filesys.untrusted_fip_check 0 %s %q %q\n' "$mednafen" "$bios_option" "$bios" "$disc"
((launch)) || exit 0
((operator_only)) || exit 1

if ((no_waiting)); then
  waiting_env=1
else
  waiting_env="${FIRESTAFF_NEXUS_NO_WAITING:-}"
fi
if [[ -n "$mednafen_home" ]]; then
  trap cleanup_capture_child INT TERM EXIT
  HOME="$mednafen_home" \
  FIRESTAFF_NEXUS_TRACE_OUTPUT="$trace" \
  FIRESTAFF_NEXUS_NO_WAITING="$waiting_env" \
  FIRESTAFF_NEXUS_TRACE_SKIP_FRAMES="$skip_frames" \
  FIRESTAFF_NEXUS_TRACE_FRAME_LIMIT="$frame_limit" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_FRAME="$press_start_frame" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_LENGTH="$press_start_length" \
  FIRESTAFF_NEXUS_TRACE_PRESS_BUTTON_MASK="$press_button_mask" \
    "$mednafen" -filesys.untrusted_fip_check 0 "$bios_option" "$bios" "$disc" &
  capture_child_pid=$!
else
  trap cleanup_capture_child INT TERM EXIT
  FIRESTAFF_NEXUS_TRACE_OUTPUT="$trace" \
  FIRESTAFF_NEXUS_NO_WAITING="$waiting_env" \
  FIRESTAFF_NEXUS_TRACE_SKIP_FRAMES="$skip_frames" \
  FIRESTAFF_NEXUS_TRACE_FRAME_LIMIT="$frame_limit" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_FRAME="$press_start_frame" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_LENGTH="$press_start_length" \
  FIRESTAFF_NEXUS_TRACE_PRESS_BUTTON_MASK="$press_button_mask" \
    "$mednafen" -filesys.untrusted_fip_check 0 "$bios_option" "$bios" "$disc" &
  capture_child_pid=$!
fi
if ((timeout_seconds > 0)); then
  (
    sleep "$timeout_seconds"
    if [[ -n "$capture_child_pid" ]] && kill -0 "$capture_child_pid" 2>/dev/null; then
      kill -TERM "$capture_child_pid" 2>/dev/null || true
      # Mednafen's signal handler flushes the capture but may not return
      # promptly. Give it a short grace period, then guarantee that the
      # operator launcher cannot remain attached to a dead capture session.
      sleep 2
      if kill -0 "$capture_child_pid" 2>/dev/null; then
        kill -KILL "$capture_child_pid" 2>/dev/null || true
      fi
    fi
  ) &
  capture_timeout_pid=$!
fi
set +e
wait "$capture_child_pid"
capture_status=$?
set -e
if [[ -n "$capture_timeout_pid" ]] && kill -0 "$capture_timeout_pid" 2>/dev/null; then
  kill -TERM "$capture_timeout_pid" 2>/dev/null || true
fi
capture_timeout_pid=
capture_child_pid=
trap - INT TERM EXIT
((capture_status == 0)) || exit "$capture_status"
[[ -s "$trace" ]] || exit 1
run_validator "$trace" --require-frames "$frame_limit"
raw_bytes=$(wc -c < "$trace" | tr -d '[:space:]')
printf 'raw_sha256=%s\nraw_bytes=%s\n' "$(lower "$(hash_file "$trace")")" "$raw_bytes" >> "$manifest"
