#!/usr/bin/env bash
# Operator-only local launch plan for an external NXS3TOP1 producer. This
# neither copies media nor fabricates a capture payload.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --capture PATH --manifest PATH --route-epoch DEC --package-fnv HEX --card-fnv HEX --dgn-fnv HEX --dgn-size DEC --structure1f-index DEC --structure3-index DEC --face-ordinal DEC --vertex-offset DEC --vertex-length DEC --vertex-fnv HEX --vertex-rows-fnv HEX --normal-offset DEC --normal-length DEC --normal-fnv HEX" >&2
}

hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
require_file_hash() {
  local path=$1 expected=$2 actual
  [[ -f "$path" && "$expected" =~ ^[[:xdigit:]]{64}$ ]] || return 1
  actual=$(hash_file "$path")
  [[ "$(lower "$actual")" == "$(lower "$expected")" ]]
}
require_fnv() { [[ "$1" =~ ^[[:xdigit:]]{1,16}$ && "$1" != 0 ]]; }

require_saturn_disc_container() {
  case "${1##*/}" in
    *.cue|*.ccd|*.toc|*.m3u) return 0 ;;
    *)
      echo "ERROR: Saturn capture requires a CUE/CCD/TOC/M3U disc container; raw ISO/BIN lacks CDDA layout" >&2
      return 1 ;;
  esac
}
require_uint() { [[ "$1" =~ ^[0-9]+$ ]]; }

launch=0
operator_only=0
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--capture|--manifest|--route-epoch|--package-fnv|--card-fnv|--dgn-fnv|--dgn-size|--structure1f-index|--structure3-index|--face-ordinal|--vertex-offset|--vertex-length|--vertex-fnv|--vertex-rows-fnv|--normal-offset|--normal-length|--normal-fnv)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${bios_region:?}" \
  "${disc:?}" "${disc_sha256:?}" "${capture:?}" "${manifest:?}" \
  "${route_epoch:?}" "${package_fnv:?}" "${card_fnv:?}" "${dgn_fnv:?}" \
  "${dgn_size:?}" "${structure1f_index:?}" "${structure3_index:?}" \
  "${face_ordinal:?}" "${vertex_offset:?}" "${vertex_length:?}" \
  "${vertex_fnv:?}" "${vertex_rows_fnv:?}" "${normal_offset:?}" \
  "${normal_length:?}" "${normal_fnv:?}"

[[ -x "$mednafen" ]] || exit 1
require_file_hash "$bios" "$bios_sha256" || exit 1
require_file_hash "$disc" "$disc_sha256" || exit 1
require_saturn_disc_container "$disc" || exit 1
require_uint "$route_epoch" && ((route_epoch > 0)) || exit 1
for value in "$dgn_size" "$structure1f_index" "$structure3_index" "$face_ordinal" "$vertex_offset" "$vertex_length" "$normal_offset" "$normal_length"; do
  require_uint "$value" || exit 1
done
((dgn_size > 0 && vertex_length > 0 && normal_length > 0)) || exit 1
require_fnv "$package_fnv" && require_fnv "$card_fnv" && require_fnv "$dgn_fnv" && \
  require_fnv "$vertex_fnv" && require_fnv "$vertex_rows_fnv" && \
  require_fnv "$normal_fnv" || exit 1
case "$bios_region" in
  us) bios_option=-ss.bios_na_eu ;;
  jp) bios_option=-ss.bios_jp_path ;;
  eu) bios_option=-ss.bios_na_eu ;;
  *) exit 1 ;;
esac
[[ ! -e "$capture" && ! -e "$manifest" && "$capture" != "$manifest" && \
   -d "$(dirname "$capture")" && -d "$(dirname "$manifest")" ]] || exit 1

bios_sha256=$(lower "$bios_sha256")
disc_sha256=$(lower "$disc_sha256")
for name in package_fnv card_fnv dgn_fnv vertex_fnv vertex_rows_fnv normal_fnv; do
  printf -v "$name" '%s' "$(lower "${!name}")"
done

manifest_tmp="$manifest.tmp.$$"
umask 077
{
  printf 'FIRESTAFF_NEXUS_MEDNAFEN_STRUCTURE3_TOPOLOGY_CAPTURE_PLAN_V1\n'
  printf 'capture_magic=NXS3TOP1\ncapture_version=1\ncapture_header_bytes=124\n'
  printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\n' "$bios_sha256" "$bios_region" "$disc_sha256"
  printf 'route_epoch=%s\npackage_fnv1a64=%s\ncard_fnv1a64=%s\ndgn_fnv1a64=%s\ndgn_size=%s\n' "$route_epoch" "$package_fnv" "$card_fnv" "$dgn_fnv" "$dgn_size"
  printf 'structure1f_entry_index=%s\nstructure3_entry_index=%s\nface_ordinal=%s\n' "$structure1f_index" "$structure3_index" "$face_ordinal"
  printf 'vertex_table_offset=%s\nvertex_table_length=%s\nvertex_table_fnv1a64=%s\nreferenced_vertex_rows_fnv1a64=%s\n' "$vertex_offset" "$vertex_length" "$vertex_fnv" "$vertex_rows_fnv"
  printf 'normal_offset=%s\nnormal_length=%s\nnormal_fnv1a64=%s\n' "$normal_offset" "$normal_length" "$normal_fnv"
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios_region=%s\nbios=%q\ndisc=%q\ncapture=%q\nmanifest=%q\n' "$bios_region" "$bios" "$disc" "$capture" "$manifest"
printf 'command=%q %s %q %q\n' "$mednafen" "$bios_option" "$bios" "$disc"
((launch)) || exit 0
((operator_only)) || exit 1

FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_OUTPUT="$capture" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_MAGIC=NXS3TOP1 \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_VERSION=1 \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_ROUTE_EPOCH="$route_epoch" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_PACKAGE_FNV1A64="$package_fnv" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_CARD_FNV1A64="$card_fnv" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_DGN_FNV1A64="$dgn_fnv" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_DGN_SIZE="$dgn_size" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_STRUCTURE1F_ENTRY_INDEX="$structure1f_index" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_STRUCTURE3_ENTRY_INDEX="$structure3_index" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_FACE_ORDINAL="$face_ordinal" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_VERTEX_TABLE_OFFSET="$vertex_offset" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_VERTEX_TABLE_LENGTH="$vertex_length" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_VERTEX_TABLE_FNV1A64="$vertex_fnv" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_VERTEX_ROWS_FNV1A64="$vertex_rows_fnv" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_NORMAL_OFFSET="$normal_offset" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_NORMAL_LENGTH="$normal_length" \
FIRESTAFF_NEXUS_STRUCTURE3_TOPOLOGY_CAPTURE_NORMAL_FNV1A64="$normal_fnv" \
  "$mednafen" "$bios_option" "$bios" "$disc"

[[ -s "$capture" && "$(head -c 8 "$capture")" == NXS3TOP1 ]] || exit 1
