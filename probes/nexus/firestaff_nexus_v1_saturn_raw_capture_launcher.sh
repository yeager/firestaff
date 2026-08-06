#!/usr/bin/env bash
# External-only launcher for the raw VDP1/VDP2 witness producer.
# It never copies BIOS, discs, or capture bytes into the repository.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --disc PATH --disc-sha256 HEX --trace PATH --validator PATH --manifest PATH [--skip-frames DEC] [--frame-limit DEC]" >&2
}

hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
require_hash() {
  [[ -f "$1" && "$2" =~ ^[[:xdigit:]]{64}$ ]] || return 1
  [[ "$(lower "$(hash_file "$1")")" == "$(lower "$2")" ]]
}
require_disc_container() {
  case "${1##*/}" in
    *.cue|*.ccd|*.toc|*.m3u) ;;
    *) echo "ERROR: Saturn capture requires a CUE/CCD/TOC/M3U container" >&2; return 1 ;;
  esac
}

launch=0
operator_only=0
skip_frames=0
frame_limit=2
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--disc|--disc-sha256|--trace|--validator|--manifest|--skip-frames|--frame-limit)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${disc:?}" "${disc_sha256:?}" \
  "${trace:?}" "${validator:?}" "${manifest:?}"
[[ -x "$mednafen" && -x "$validator" ]] || exit 1
require_hash "$bios" "$bios_sha256" || exit 1
require_hash "$disc" "$disc_sha256" || exit 1
require_disc_container "$disc" || exit 1
[[ "$skip_frames" =~ ^[0-9]+$ && "$frame_limit" =~ ^[1-9][0-9]*$ ]] || exit 1
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
  printf 'bios_sha256=%s\ndisc_sha256=%s\nskip_frames=%s\nframe_limit=%s\n' \
    "$(lower "$bios_sha256")" "$(lower "$disc_sha256")" "$skip_frames" "$frame_limit"
  printf 'capture_magic=FIRESTAFF_NEXUS_SATURN_RUNTIME_CAPTURE_V1\n'
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios=%q\n' "$bios"
printf 'disc=%q\n' "$disc"
printf 'trace=%q\n' "$trace"
printf 'manifest=%q\n' "$manifest"
printf 'command=%q -filesys.untrusted_fip_check 0 -ss.bios_na_eu %q %q\n' "$mednafen" "$bios" "$disc"
((launch)) || exit 0
((operator_only)) || exit 1

FIRESTAFF_NEXUS_TRACE_OUTPUT="$trace" \
FIRESTAFF_NEXUS_TRACE_SKIP_FRAMES="$skip_frames" \
FIRESTAFF_NEXUS_TRACE_FRAME_LIMIT="$frame_limit" \
  "$mednafen" -filesys.untrusted_fip_check 0 -ss.bios_na_eu "$bios" "$disc"
[[ -s "$trace" ]] || exit 1
"$validator" "$trace" --require-frames "$frame_limit"
