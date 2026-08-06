#!/usr/bin/env bash
# Operator-only, external Saturn VDP1 capture plan. It never copies a BIOS,
# disc, DGN, card, or capture payload. A capture-capable Mednafen build owns
# creation of the NXSVDP1C V1 file; this script merely binds its input route.
set -euo pipefail

usage() {
  echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --capture PATH --manifest PATH --route-epoch DEC --package-fnv HEX --card-fnv HEX --dgn-fnv HEX --dgn-size DEC --face-fnv HEX --descriptor-fnv HEX --image-fnv HEX --palette-fnv HEX" >&2
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
    echo "       stock Mednafen cannot produce NXSVDP1C; use an instrumented build" >&2
    return 1
  }
}

launch=0
operator_only=0
while (($#)); do
  case "$1" in
    --launch) launch=1; shift ;;
    --operator-only) operator_only=1; shift ;;
    --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--capture|--manifest|--route-epoch|--package-fnv|--card-fnv|--dgn-fnv|--dgn-size|--face-fnv|--descriptor-fnv|--image-fnv|--palette-fnv)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done

: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${bios_region:?}" \
  "${disc:?}" "${disc_sha256:?}" "${capture:?}" "${manifest:?}" \
  "${route_epoch:?}" "${package_fnv:?}" "${card_fnv:?}" "${dgn_fnv:?}" \
  "${dgn_size:?}" "${face_fnv:?}" "${descriptor_fnv:?}" "${image_fnv:?}" \
  "${palette_fnv:?}"

[[ -x "$mednafen" ]] || exit 1
require_file_hash "$bios" "$bios_sha256" || exit 1
require_file_hash "$disc" "$disc_sha256" || exit 1
[[ "$route_epoch" =~ ^[1-9][0-9]*$ && "$dgn_size" =~ ^[1-9][0-9]*$ ]] || exit 1
require_fnv "$package_fnv" && require_fnv "$card_fnv" && require_fnv "$dgn_fnv" && \
  require_fnv "$face_fnv" && require_fnv "$descriptor_fnv" && \
  require_fnv "$image_fnv" && require_fnv "$palette_fnv" || exit 1
case "$bios_region" in
  us) bios_option=-ss.bios_us_path ;;
  jp) bios_option=-ss.bios_jp_path ;;
  eu) bios_option=-ss.bios_eu_path ;;
  *) exit 1 ;;
esac
[[ ! -e "$capture" && ! -e "$manifest" && "$capture" != "$manifest" && \
   -d "$(dirname "$capture")" && -d "$(dirname "$manifest")" ]] || exit 1
if ((launch)); then
  require_capture_hook FIRESTAFF_NEXUS_VDP1_CAPTURE_OUTPUT || exit 78
fi
bios_sha256=$(lower "$bios_sha256")
disc_sha256=$(lower "$disc_sha256")
package_fnv=$(lower "$package_fnv")
card_fnv=$(lower "$card_fnv")
dgn_fnv=$(lower "$dgn_fnv")
face_fnv=$(lower "$face_fnv")
descriptor_fnv=$(lower "$descriptor_fnv")
image_fnv=$(lower "$image_fnv")
palette_fnv=$(lower "$palette_fnv")

manifest_tmp="$manifest.tmp.$$"
umask 077
{
  printf 'FIRESTAFF_NEXUS_MEDNAFEN_VDP1_CAPTURE_PLAN_V1\n'
  printf 'capture_magic=NXSVDP1C\ncapture_version=1\ncapture_header_bytes=136\n'
  printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\n' "$bios_sha256" "$bios_region" "$disc_sha256"
  printf 'route_epoch=%s\npackage_fnv1a64=%s\ncard_fnv1a64=%s\ndgn_fnv1a64=%s\ndgn_size=%s\n' \
    "$route_epoch" "$package_fnv" "$card_fnv" "$dgn_fnv" "$dgn_size"
  printf 'face_fnv1a64=%s\ndescriptor_fnv1a64=%s\nimage_candidate_fnv1a64=%s\npalette_candidate_fnv1a64=%s\n' \
    "$face_fnv" "$descriptor_fnv" "$image_fnv" "$palette_fnv"
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

FIRESTAFF_NEXUS_VDP1_CAPTURE_OUTPUT="$capture" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_MAGIC=NXSVDP1C \
FIRESTAFF_NEXUS_VDP1_CAPTURE_VERSION=1 \
FIRESTAFF_NEXUS_VDP1_CAPTURE_ROUTE_EPOCH="$route_epoch" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_PACKAGE_FNV1A64="$package_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_CARD_FNV1A64="$card_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_DGN_FNV1A64="$dgn_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_DGN_SIZE="$dgn_size" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_FACE_FNV1A64="$face_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_DESCRIPTOR_FNV1A64="$descriptor_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_IMAGE_FNV1A64="$image_fnv" \
FIRESTAFF_NEXUS_VDP1_CAPTURE_PALETTE_FNV1A64="$palette_fnv" \
  "$mednafen" "$bios_option" "$bios" "$disc"

# The engine importer validates the remaining payload interval/FNV and every
# identity against its live target. This is only a launch-side proof that the
# external producer emitted the agreed V1 artifact family.
[[ -s "$capture" ]] || exit 1
[[ "$(head -c 8 "$capture")" == NXSVDP1C ]] || exit 1
