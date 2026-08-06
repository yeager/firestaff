#!/usr/bin/env bash
# Operator-only, external Saturn SLEV/SAL capture plan. It never copies a
# BIOS, disc, task, audio asset, or payload. A capture-capable Mednafen build
# alone may create NXSLSC01; this script binds the input route and verifies
# only the artifact family after launch.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --capture PATH --manifest PATH --route-epoch DEC --package-fnv HEX --card-fnv HEX --task-trace-fnv HEX --task-source-fnv HEX --sal-descriptor-fnv HEX --map-table-fnv HEX --sddrvs-fnv HEX" >&2
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

require_capture_hook() {
  local marker=$1
  strings "$mednafen" | grep -Fq "$marker" || {
    echo "ERROR: Mednafen binary does not advertise the Firestaff capture hook: $marker" >&2
    echo "       stock Mednafen cannot produce NXSLSC01; use an instrumented build" >&2
    return 1
  }
}

launch=0
operator_only=0
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--capture|--manifest|--route-epoch|--package-fnv|--card-fnv|--task-trace-fnv|--task-source-fnv|--sal-descriptor-fnv|--map-table-fnv|--sddrvs-fnv)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${bios_region:?}" \
  "${disc:?}" "${disc_sha256:?}" "${capture:?}" "${manifest:?}" \
  "${route_epoch:?}" "${package_fnv:?}" "${card_fnv:?}" \
  "${task_trace_fnv:?}" "${task_source_fnv:?}" \
  "${sal_descriptor_fnv:?}" "${map_table_fnv:?}" "${sddrvs_fnv:?}"

[[ -x "$mednafen" ]] || exit 1
require_file_hash "$bios" "$bios_sha256" || exit 1
require_file_hash "$disc" "$disc_sha256" || exit 1
[[ "$route_epoch" =~ ^[1-9][0-9]*$ ]] || exit 1
require_fnv "$package_fnv" && require_fnv "$card_fnv" && \
  require_fnv "$task_trace_fnv" && require_fnv "$task_source_fnv" && \
  require_fnv "$sal_descriptor_fnv" && require_fnv "$map_table_fnv" && \
  require_fnv "$sddrvs_fnv" || exit 1
case "$bios_region" in
  us) bios_option=-ss.bios_us_path ;;
  jp) bios_option=-ss.bios_jp_path ;;
  eu) bios_option=-ss.bios_eu_path ;;
  *) exit 1 ;;
esac
[[ ! -e "$capture" && ! -e "$manifest" && "$capture" != "$manifest" && \
   -d "$(dirname "$capture")" && -d "$(dirname "$manifest")" ]] || exit 1
if ((launch)); then
  require_capture_hook FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_OUTPUT || exit 78
fi
bios_sha256=$(lower "$bios_sha256")
disc_sha256=$(lower "$disc_sha256")
package_fnv=$(lower "$package_fnv")
card_fnv=$(lower "$card_fnv")
task_trace_fnv=$(lower "$task_trace_fnv")
task_source_fnv=$(lower "$task_source_fnv")
sal_descriptor_fnv=$(lower "$sal_descriptor_fnv")
map_table_fnv=$(lower "$map_table_fnv")
sddrvs_fnv=$(lower "$sddrvs_fnv")

manifest_tmp="$manifest.tmp.$$"
umask 077
{
  printf 'FIRESTAFF_NEXUS_MEDNAFEN_SLEV_SAL_CAPTURE_PLAN_V1\n'
  printf 'capture_magic=NXSLSC01\ncapture_version=1\ncapture_header_bytes=96\n'
  printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\n' "$bios_sha256" "$bios_region" "$disc_sha256"
  printf 'route_epoch=%s\npackage_fnv1a64=%s\ncard_fnv1a64=%s\n' \
    "$route_epoch" "$package_fnv" "$card_fnv"
  printf 'task_trace_fnv1a64=%s\ntask_source_fnv1a64=%s\n' \
    "$task_trace_fnv" "$task_source_fnv"
  printf 'sal_descriptor_fnv1a64=%s\nmap_table_fnv1a64=%s\nsddrvs_fnv1a64=%s\n' \
    "$sal_descriptor_fnv" "$map_table_fnv" "$sddrvs_fnv"
} > "$manifest_tmp"
mv "$manifest_tmp" "$manifest"

printf 'mednafen=%q\n' "$mednafen"
printf 'bios_region=%s\n' "$bios_region"
printf 'bios=%q\n' "$bios"
printf 'disc=%q\n' "$disc"
printf 'capture=%q\n' "$capture"
printf 'manifest=%q\n' "$manifest"
printf 'command=%q %s %q %q\n' "$mednafen" "$bios_option" "$bios" "$disc"
((launch)) || exit 0
((operator_only)) || exit 1

FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_OUTPUT="$capture" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_MAGIC=NXSLSC01 \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_VERSION=1 \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_ROUTE_EPOCH="$route_epoch" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_PACKAGE_FNV1A64="$package_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_CARD_FNV1A64="$card_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_TASK_TRACE_FNV1A64="$task_trace_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_TASK_SOURCE_FNV1A64="$task_source_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_SAL_DESCRIPTOR_FNV1A64="$sal_descriptor_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_MAP_TABLE_FNV1A64="$map_table_fnv" \
FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_SDDRVS_FNV1A64="$sddrvs_fnv" \
  "$mednafen" "$bios_option" "$bios" "$disc"

# The C importer validates every header identity and opaque payload interval.
[[ -s "$capture" ]] || exit 1
[[ "$(head -c 8 "$capture")" == NXSLSC01 ]] || exit 1
