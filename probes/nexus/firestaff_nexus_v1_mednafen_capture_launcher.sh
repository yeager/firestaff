#!/usr/bin/env bash
# External-only Nexus Saturn capture launcher. It never copies BIOS, disc, or
# trace bytes; a capture-capable Mednafen build must create the trace itself.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --disc PATH --disc-sha256 HEX --menu-bpk PATH --menu-bpk-sha256 HEX --dm-bin PATH --dm-bin-sha256 HEX --dgn PATH --dgn-sha256 HEX --trace PATH --validator PATH --manifest PATH --replay-trace-fnv HEX --replay-dgn-fnv HEX --replay-bitmap-fnv HEX --replay-epoch DEC" >&2
}

hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
require_file_hash() {
  local path=$1 expected=$2 actual actual_lower expected_lower
  [[ -f "$path" && "$expected" =~ ^[[:xdigit:]]{64}$ ]] || return 1
  actual=$(hash_file "$path")
  actual_lower=$(printf '%s' "$actual" | tr '[:upper:]' '[:lower:]')
  expected_lower=$(printf '%s' "$expected" | tr '[:upper:]' '[:lower:]')
  [[ "$actual_lower" == "$expected_lower" ]]
}

require_capture_hook() {
  local marker=$1
  strings "$mednafen" | grep -Fq "$marker" || {
    echo "ERROR: Mednafen binary does not advertise the Firestaff capture hook: $marker" >&2
    echo "       stock Mednafen cannot produce a Nexus Saturn trace; use an instrumented build" >&2
    return 1
  }
}

launch=0
operator_only=0
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--disc|--disc-sha256|--trace|--validator|--menu-bpk|--menu-bpk-sha256|--dm-bin|--dm-bin-sha256|--dgn|--dgn-sha256|--manifest|--replay-trace-fnv|--replay-dgn-fnv|--replay-bitmap-fnv|--replay-epoch)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${disc:?}" "${disc_sha256:?}" \
  "${trace:?}" "${validator:?}" "${menu_bpk:?}" "${menu_bpk_sha256:?}" "${dm_bin:?}" "${dm_bin_sha256:?}" \
  "${dgn:?}" "${dgn_sha256:?}" "${manifest:?}" "${replay_trace_fnv:?}" "${replay_dgn_fnv:?}" "${replay_bitmap_fnv:?}" "${replay_epoch:?}"
[[ -x "$mednafen" && -x "$validator" ]] || exit 1
require_file_hash "$bios" "$bios_sha256" || exit 1
require_file_hash "$disc" "$disc_sha256" || exit 1
require_file_hash "$menu_bpk" "$menu_bpk_sha256" || exit 1
require_file_hash "$dm_bin" "$dm_bin_sha256" || exit 1
require_file_hash "$dgn" "$dgn_sha256" || exit 1
[[ "$replay_trace_fnv" =~ ^[[:xdigit:]]+$ && "$replay_dgn_fnv" =~ ^[[:xdigit:]]+$ && "$replay_bitmap_fnv" =~ ^[[:xdigit:]]+$ && "$replay_epoch" =~ ^[1-9][0-9]*$ ]] || exit 1
[[ ! -e "$trace" && ! -e "$manifest" && -d "$(dirname "$trace")" && -d "$(dirname "$manifest")" ]] || exit 1
if ((launch)); then
  require_capture_hook FIRESTAFF_NEXUS_TRACE_OUTPUT || exit 78
fi

manifest_tmp="$manifest.tmp.$$"
umask 077
{
  printf 'FIRESTAFF_NEXUS_MEDNAFEN_PRS3_REPLAY_MANIFEST_V1\n'
  printf 'bios_sha256=%s\ndisc_sha256=%s\nmenu_bpk_sha256=%s\ndm_bin_sha256=%s\ndgn_sha256=%s\n' "$bios_sha256" "$disc_sha256" "$menu_bpk_sha256" "$dm_bin_sha256" "$dgn_sha256"
  printf 'replay_trace_fnv1a64=%s\nreplay_dgn_fnv1a64=%s\nreplay_bitmap_fnv1a64=%s\nreplay_epoch=%s\n' "$replay_trace_fnv" "$replay_dgn_fnv" "$replay_bitmap_fnv" "$replay_epoch"
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios=%q\n' "$bios"
printf 'disc=%q\n' "$disc"
printf 'trace=%q\n' "$trace"
printf 'manifest=%q\n' "$manifest"
printf 'command=%q -ss.bios_us_path %q %q\n' "$mednafen" "$bios" "$disc"
((launch)) || exit 0
((operator_only)) || exit 1

FIRESTAFF_NEXUS_TRACE_OUTPUT="$trace" "$mednafen" -ss.bios_us_path "$bios" "$disc"
[[ -s "$trace" ]] || exit 1
"$validator" "$trace" "$menu_bpk" "$dm_bin"
