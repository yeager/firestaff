#!/usr/bin/env bash
# Operator-only external NXSPRS3M launch plan. It never creates a capture.
set -euo pipefail
hash() { shasum -a 256 "$1" | awk '{print $1}'; }; low() { printf %s "$1" | tr '[:upper:]' '[:lower:]'; }
usage() { echo "usage: $0 [--operator-only --launch] --mednafen PATH --bios PATH --bios-sha256 HEX --bios-region us|jp|eu --disc PATH --disc-sha256 HEX --capture PATH --manifest PATH --route-epoch DEC --package-fnv HEX --card-fnv HEX --entry-index DEC --compressed-offset DEC --compressed-length DEC --compressed-fnv HEX --declared-output DEC" >&2; }
checkhash() { [[ -f "$1" && "$2" =~ ^[[:xdigit:]]{64}$ && $(low "$(hash "$1")") == $(low "$2") ]]; }; hx() { [[ "$1" =~ ^[[:xdigit:]]{1,16}$ && "$1" != 0 ]]; }; dec() { [[ "$1" =~ ^[0-9]+$ ]]; }
launch=0; operator_only=0
while (($#)); do case "$1" in --launch) launch=1;shift;; --operator-only) operator_only=1;shift;; --mednafen|--bios|--bios-sha256|--bios-region|--disc|--disc-sha256|--capture|--manifest|--route-epoch|--package-fnv|--card-fnv|--entry-index|--compressed-offset|--compressed-length|--compressed-fnv|--declared-output) (($#>=2))||{ usage;exit 2;}; k=${1#--};k=${k//-/_};printf -v "$k" %s "$2";shift 2;; *) usage;exit 2;; esac;done
: "${mednafen:?}" "${bios:?}" "${bios_sha256:?}" "${bios_region:?}" "${disc:?}" "${disc_sha256:?}" "${capture:?}" "${manifest:?}" "${route_epoch:?}" "${package_fnv:?}" "${card_fnv:?}" "${entry_index:?}" "${compressed_offset:?}" "${compressed_length:?}" "${compressed_fnv:?}" "${declared_output:?}"
[[ -x "$mednafen" ]] && checkhash "$bios" "$bios_sha256" && checkhash "$disc" "$disc_sha256" || exit 1
for n in route_epoch entry_index compressed_offset compressed_length declared_output; do dec "${!n}" || exit 1; done
((route_epoch>0 && compressed_length>0 && declared_output>0)) || exit 1; hx "$package_fnv" && hx "$card_fnv" && hx "$compressed_fnv" || exit 1
case "$bios_region" in us|eu) opt=-ss.bios_na_eu;;jp) opt=-ss.bios_jp_path;;*) exit 1;;esac
[[ ! -e "$capture" && ! -e "$manifest" && -d "$(dirname "$capture")" && -d "$(dirname "$manifest")" ]] || exit 1
{
 printf 'FIRESTAFF_NEXUS_MEDNAFEN_PRS3_MATERIAL_CAPTURE_PLAN_V1\nNXSPRS3M\n';
 printf 'bios_sha256=%s\nbios_region=%s\ndisc_sha256=%s\nroute_epoch=%s\npackage_fnv1a64=%s\ncard_fnv1a64=%s\nentry_index=%s\ncompressed_offset=%s\ncompressed_length=%s\ncompressed_fnv1a64=%s\ndeclared_output_bytes=%s\n' "$(low "$bios_sha256")" "$bios_region" "$(low "$disc_sha256")" "$route_epoch" "$(low "$package_fnv")" "$(low "$card_fnv")" "$entry_index" "$compressed_offset" "$compressed_length" "$(low "$compressed_fnv")" "$declared_output";
} > "$manifest"
printf 'command=%q %s %q %q\n' "$mednafen" "$opt" "$bios" "$disc"
((launch)) || exit 0; ((operator_only)) || exit 1
FIRESTAFF_NEXUS_PRS3_MATERIAL_CAPTURE_OUTPUT="$capture" FIRESTAFF_NEXUS_PRS3_MATERIAL_CAPTURE_MAGIC=NXSPRS3M "$mednafen" "$opt" "$bios" "$disc"
[[ -s "$capture" && $(head -c 8 "$capture") == NXSPRS3M ]] || exit 1
