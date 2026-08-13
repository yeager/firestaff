#!/usr/bin/env bash
# External-only launcher for the raw VDP1/VDP2 witness producer.
# It never copies BIOS, discs, or capture bytes into the repository.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --trace PATH --validator PATH --manifest PATH [--mednafen-home PATH] [--trace-session TOKEN] [--no-waiting] [--require-input-window] [--timeout-seconds DEC] [--skip-frames DEC] [--frame-limit DEC] [--press-start-frame DEC] [--press-start-length DEC] [--press-button-mask DEC/HEX]" >&2
}

hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
append_trace_receipts() {
  if [[ -n "${FIRESTAFF_NEXUS_TRACE_VDP1_WRITES:-}" &&
        -s "$FIRESTAFF_NEXUS_TRACE_VDP1_WRITES" ]] &&
     ! grep -q '^vdp1_write_trace_sha256=' "$manifest"; then
    printf 'vdp1_write_trace_sha256=%s\n' \
      "$(lower "$(hash_file "$FIRESTAFF_NEXUS_TRACE_VDP1_WRITES")")" >> "$manifest"
  fi
  if [[ -n "${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE:-}" &&
        -s "$FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE" ]] &&
     ! grep -q '^vdp1_writer_code_trace_sha256=' "$manifest"; then
    printf 'vdp1_writer_code_trace_sha256=%s\n' \
      "$(lower "$(hash_file "$FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE")")" >> "$manifest"
  fi
  if [[ -n "${FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT:-}" &&
        -s "$FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT" ]] &&
     ! grep -q '^vdp1_snapshot_sha256=' "$manifest"; then
    printf 'vdp1_snapshot_sha256=%s\n' \
      "$(lower "$(hash_file "$FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT")")" >> "$manifest"
  fi
  for trace_var in \
    FIRESTAFF_NEXUS_TRACE_VDP1_REGS \
    FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP \
    FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS \
    FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE \
    FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES \
    FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES; do
    trace_path="${!trace_var:-}"
    if [[ -n "$trace_path" && -s "$trace_path" ]] &&
       ! grep -q "^${trace_var}_sha256=" "$manifest"; then
      printf '%s_sha256=%s\n' "$trace_var" \
        "$(lower "$(hash_file "$trace_path")")" >> "$manifest"
    fi
  done
  for trace_var in \
    FIRESTAFF_NEXUS_TRACE_SH2_PC \
    FIRESTAFF_NEXUS_TRACE_SH2_PC_TRACE \
    FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT \
    FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS; do
    trace_path="${!trace_var:-}"
    if [[ -n "$trace_path" && -s "$trace_path" ]] &&
       ! grep -q "^${trace_var}_sha256=" "$manifest"; then
      printf '%s_sha256=%s\n' "$trace_var" \
        "$(lower "$(hash_file "$trace_path")")" >> "$manifest"
    fi
  done
  for trace_var in \
    FIRESTAFF_NEXUS_TRACE_SCSP_WRITES \
    FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES \
    FIRESTAFF_NEXUS_TRACE_SCSP_READS \
    FIRESTAFF_NEXUS_TRACE_VDP2_WRITES \
    FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_CODE \
    FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS \
    FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT; do
    trace_path="${!trace_var:-}"
    if [[ -n "$trace_path" && -s "$trace_path" ]] &&
       ! grep -q "^${trace_var}_sha256=" "$manifest"; then
      printf '%s_sha256=%s\n' "$trace_var" \
        "$(lower "$(hash_file "$trace_path")")" >> "$manifest"
    fi
  done
}
capture_manifest_finalized=0
finalize_capture_manifest() {
  local status="${1:-1}"
  [[ -n "${manifest:-}" && -f "$manifest" ]] || return 0
  if ((capture_manifest_finalized)); then return 0; fi
  capture_manifest_finalized=1
  if ! grep -q '^capture_exit_status=' "$manifest"; then
    printf 'capture_exit_status=%s\n' "$status" >> "$manifest"
  fi
  append_trace_receipts || true
}
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

validate_press_sequence() {
  local sequence="${FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE:-}"
  local entry frame length mask extra mask_value end
  local -a entries
  [[ -z "$sequence" ]] && return 0
  IFS=',' read -r -a entries <<< "$sequence"
  ((${#entries[@]} <= 16)) || {
    echo "ERROR: FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE supports at most 16 entries" >&2
    return 1
  }
  for entry in "${entries[@]}"; do
    IFS=':' read -r frame length mask extra <<< "$entry"
    [[ -n "$frame" && -n "$length" && -n "$mask" && -z "$extra" &&
       "$frame" =~ ^[0-9]+$ && "$length" =~ ^[1-9][0-9]*$ &&
       "$mask" =~ ^(0[xX])?[0-9a-fA-F]+$ ]] || {
      echo "ERROR: invalid press sequence entry: $entry (expected frame:length:mask)" >&2
      return 1
    }
    if [[ "$mask" =~ ^0[xX] ]]; then
      mask_value=$((16#${mask:2}))
    else
      mask_value=$((10#$mask))
    fi
    ((mask_value >= 0 && mask_value <= 0x1fff)) || {
      echo "ERROR: press sequence mask outside Saturn pad range: $entry" >&2
      return 1
    }
    if ((require_input_window)); then
      end=$((frame + length))
      ((frame >= skip_frames && end <= skip_frames + frame_limit)) || {
        echo "ERROR: press sequence entry outside the captured frame window: $entry" >&2
        return 1
      }
    fi
  done
}

run_validator() {
  case "$validator" in
    *.py) python3 "$validator" "$@" ;;
    *) "$validator" "$@" ;;
  esac
}

capture_child_pid=
capture_process_group_pid=
capture_timeout_pid=
terminate_capture_process() {
  local pid="${1:-}"
  [[ -n "$pid" ]] || return 0
  if [[ -n "${capture_process_group_pid:-}" && "$capture_process_group_pid" == "$pid" ]]; then
    kill -TERM -- "-$pid" 2>/dev/null || true
    sleep 1
    kill -KILL -- "-$pid" 2>/dev/null || true
  else
    kill -TERM "$pid" 2>/dev/null || true
    sleep 1
    kill -KILL "$pid" 2>/dev/null || true
  fi
}
cleanup_capture_child() {
  local status=$?
  if [[ -n "$capture_timeout_pid" ]] && kill -0 "$capture_timeout_pid" 2>/dev/null; then
    kill -TERM "$capture_timeout_pid" 2>/dev/null || true
  fi
  if [[ -n "$capture_child_pid" ]] && kill -0 "$capture_child_pid" 2>/dev/null; then
    terminate_capture_process "$capture_child_pid"
  fi
  capture_timeout_pid=
  capture_child_pid=
  capture_process_group_pid=
  finalize_capture_manifest "$status" || true
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
require_input_window=0
bios_region=eu
mednafen_options=()
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--trace|--validator|--manifest|--mednafen-home|--trace-session|--timeout-seconds|--skip-frames|--frame-limit|--press-start-frame|--press-start-length|--press-button-mask)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    --no-waiting) no_waiting=1; shift ;;
    --require-input-window) require_input_window=1; shift ;;
    *) usage; exit 2 ;;
  esac
done

# Optional emulator-only tuning for long raw captures.  Keep this separate
# from Firestaff runtime configuration and record it in the manifest so a
# faster headless capture remains reproducible.
if [[ -n "${FIRESTAFF_NEXUS_MEDNAFEN_OPTIONS:-}" ]]; then
  read -r -a mednafen_options <<< "$FIRESTAFF_NEXUS_MEDNAFEN_OPTIONS"
fi
mednafen_command=("$mednafen")
if ((${#mednafen_options[@]} > 0)); then
  mednafen_command+=("${mednafen_options[@]}")
fi
capture_session_launcher=()
if command -v setsid >/dev/null 2>&1; then
  capture_session_launcher=(setsid)
elif command -v python3 >/dev/null 2>&1; then
  # macOS does not ship setsid.  The validator already requires Python for
  # .py validators, so use a tiny external-session wrapper there as well.
  capture_session_launcher=(python3 -c 'import os,sys; os.setsid(); os.execvp(sys.argv[1],sys.argv[1:])')
fi
launch_capture_process() {
  if ((${#capture_session_launcher[@]})); then
    exec "${capture_session_launcher[@]}" "$@"
  else
    exec "$@"
  fi
}

if [[ -z "${trace_session:-}" ]]; then
  trace_session="${FIRESTAFF_NEXUS_TRACE_SESSION:-nexus-$(date -u +%Y%m%dT%H%M%SZ)-$$}"
fi
: "${trace_session:?}"
[[ "$trace_session" =~ ^[A-Za-z0-9._-]+$ ]] || {
  echo "ERROR: --trace-session must contain only letters, digits, '.', '_' or '-'" >&2
  exit 2
}

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
validate_press_sequence || exit 1
if ((require_input_window)) && ((press_start_frame < skip_frames ||
    press_start_frame + press_start_length > skip_frames + frame_limit)); then
  echo "ERROR: requested input window is outside the captured frame window" >&2
  exit 1
fi
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
  printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\nskip_frames=%s\nframe_limit=%s\npress_start_frame=%s\npress_start_length=%s\npress_button_mask=%s\npress_sequence=%s\n' \
    "$(lower "$bios_sha256")" "$bios_region" "$(lower "$disc_sha256")" "$skip_frames" "$frame_limit" "$press_start_frame" "$press_start_length" "$press_button_mask" "${FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE:-}"
  printf 'mednafen_home=%s\ntrace_session=%s\ncapture_session_launcher=%s\nno_waiting=%s\nrequire_input_window=%s\ntimeout_seconds=%s\n' "${mednafen_home:-}" "$trace_session" "${capture_session_launcher[*]-}" "$no_waiting" "$require_input_window" "$timeout_seconds"
  printf 'vdp1_reg_pc_list=%s\n' "${FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST:-}"
  printf 'mednafen_options=%q\n' "${mednafen_options[*]-}"
  printf 'capture_magic=FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n'
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios=%q\n' "$bios"
printf 'bios_region=%s\n' "$bios_region"
printf 'disc=%q\n' "$disc"
printf 'trace=%q\n' "$trace"
printf 'manifest=%q\n' "$manifest"
printf 'command=%q %q -filesys.untrusted_fip_check 0 %s %q %q\n' "$mednafen" "${mednafen_options[*]-}" "$bios_option" "$bios" "$disc"
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
  FIRESTAFF_NEXUS_TRACE_SESSION="$trace_session" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC="${FIRESTAFF_NEXUS_TRACE_SH2_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_TRACE="${FIRESTAFF_NEXUS_TRACE_SH2_PC_TRACE:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_PC_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_WINDOW="${FIRESTAFF_NEXUS_TRACE_SH2_PC_WINDOW:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_SH2_PC_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES="${FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES:-}" \
  FIRESTAFF_NEXUS_NO_WAITING="$waiting_env" \
  FIRESTAFF_NEXUS_TRACE_SKIP_FRAMES="$skip_frames" \
  FIRESTAFF_NEXUS_TRACE_FRAME_LIMIT="$frame_limit" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_FRAME="$press_start_frame" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_LENGTH="$press_start_length" \
  FIRESTAFF_NEXUS_TRACE_PRESS_BUTTON_MASK="$press_button_mask" \
  FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE="${FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_START="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_START:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITES="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_REGS="${FIRESTAFF_NEXUS_TRACE_VDP1_REGS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST="${FIRESTAFF_NEXUS_TRACE_VDP1_REG_PC_LIST:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_REGISTER="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_REGISTER:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE="${FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_TRANSFORM_CODE_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITES="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READS="${FIRESTAFF_NEXUS_TRACE_SCSP_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_MIN="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_MAX="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_PC="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_CODE="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_CODE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGS="${FIRESTAFF_NEXUS_TRACE_VDP2_REGS:-${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS:-}}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_NONZERO="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_NONZERO:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_NONZERO="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_NONZERO:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READS="${FIRESTAFF_NEXUS_TRACE_CD_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_CD_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA="${FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_MAX_LBA="${FIRESTAFF_NEXUS_TRACE_CD_READ_MAX_LBA:-}" \
  SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-}" \
    launch_capture_process "${mednafen_command[@]}" \
      -filesys.untrusted_fip_check 0 "$bios_option" "$bios" "$disc" &
  capture_child_pid=$!
  capture_process_group_pid="$(ps -o pgid= -p "$capture_child_pid" 2>/dev/null | tr -d ' ')"
  [[ "$capture_process_group_pid" == "$capture_child_pid" ]] || capture_process_group_pid=
else
  trap cleanup_capture_child INT TERM EXIT
  FIRESTAFF_NEXUS_TRACE_OUTPUT="$trace" \
  FIRESTAFF_NEXUS_TRACE_SESSION="$trace_session" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC="${FIRESTAFF_NEXUS_TRACE_SH2_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_TRACE="${FIRESTAFF_NEXUS_TRACE_SH2_PC_TRACE:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_PC_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_WINDOW="${FIRESTAFF_NEXUS_TRACE_SH2_PC_WINDOW:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_PC_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_SH2_PC_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES="${FIRESTAFF_NEXUS_TRACE_SH2_MEMORY_SNAPSHOT_FRAMES:-}" \
  FIRESTAFF_NEXUS_NO_WAITING="$waiting_env" \
  FIRESTAFF_NEXUS_TRACE_SKIP_FRAMES="$skip_frames" \
  FIRESTAFF_NEXUS_TRACE_FRAME_LIMIT="$frame_limit" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_FRAME="$press_start_frame" \
  FIRESTAFF_NEXUS_TRACE_PRESS_START_LENGTH="$press_start_length" \
  FIRESTAFF_NEXUS_TRACE_PRESS_BUTTON_MASK="$press_button_mask" \
  FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE="${FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_START="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITER_CODE_START:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITES="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_SNAPSHOT_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_REGS="${FIRESTAFF_NEXUS_TRACE_VDP1_REGS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_DUMP_SIZE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_SOURCE_READ_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP1_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITES="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCSP_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READS="${FIRESTAFF_NEXUS_TRACE_SCSP_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_MIN="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_MAX="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCSP_READ_PC="${FIRESTAFF_NEXUS_TRACE_SCSP_READ_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITES="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_CODE="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_CODE:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGS="${FIRESTAFF_NEXUS_TRACE_VDP2_REGS:-${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REGS:-}}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITER_REG_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_REGISTER_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_WRITE_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_AT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC="${FIRESTAFF_NEXUS_TRACE_VDP2_POST_SNAPSHOT_PC:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_VDP2_SOURCE_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_NONZERO="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_DMA_WRITE_NONZERO:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_NONZERO="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_WRITE_NONZERO:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_SOURCE_WRITE_PC_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_SH2_RAM_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_WRITES:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_SOURCE_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MIN:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_DESTINATION_MAX:-}" \
  FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT="${FIRESTAFF_NEXUS_TRACE_SCU_DMA_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READS="${FIRESTAFF_NEXUS_TRACE_CD_READS:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_LIMIT="${FIRESTAFF_NEXUS_TRACE_CD_READ_LIMIT:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA="${FIRESTAFF_NEXUS_TRACE_CD_READ_MIN_LBA:-}" \
  FIRESTAFF_NEXUS_TRACE_CD_READ_MAX_LBA="${FIRESTAFF_NEXUS_TRACE_CD_READ_MAX_LBA:-}" \
  SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-}" \
    launch_capture_process "${mednafen_command[@]}" \
      -filesys.untrusted_fip_check 0 "$bios_option" "$bios" "$disc" &
  capture_child_pid=$!
  capture_process_group_pid="$(ps -o pgid= -p "$capture_child_pid" 2>/dev/null | tr -d ' ')"
  [[ "$capture_process_group_pid" == "$capture_child_pid" ]] || capture_process_group_pid=
fi
if ((timeout_seconds > 0)); then
  (
    sleep "$timeout_seconds"
    if [[ -n "$capture_child_pid" ]] && kill -0 "$capture_child_pid" 2>/dev/null; then
      # Mednafen's signal handler flushes the capture but may not return
      # promptly. Give it a short grace period, then guarantee that the
      # operator launcher cannot remain attached to a dead capture session.
      terminate_capture_process "$capture_child_pid"
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
capture_process_group_pid=
trap - INT TERM EXIT
# `frame_limit` stops the capture hook, not necessarily the emulator. The
# operator timeout therefore commonly ends Mednafen with SIGTERM (143) after
# the requested frame window has already been flushed. Validate the raw
# witness before turning that expected process status into a failure. A
# truncated witness still fails the validator and retains the real status.
[[ -s "$trace" ]] || {
  # A clean emulator exit is not evidence that the requested witness was
  # produced. Keep this gate fail-closed so an empty capture cannot pass.
  if [[ "$capture_status" -eq 0 ]]; then
    capture_status=1
  fi
  finalize_capture_manifest "$capture_status"
  exit 1
}
set +e
run_validator "$trace" --require-frames "$frame_limit"
validator_status=$?
set -e
if ((validator_status == 0)); then
  capture_status=0
else
  # A zero emulator exit status never overrides a failed raw-layout check.
  capture_status=1
fi
finalize_capture_manifest "$capture_status"
((capture_status == 0)) || exit "$capture_status"
raw_bytes=$(wc -c < "$trace" | tr -d '[:space:]')
printf 'raw_sha256=%s\nraw_bytes=%s\n' "$(lower "$(hash_file "$trace")")" "$raw_bytes" >> "$manifest"
