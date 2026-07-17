#!/usr/bin/env bash
# Read-only local check for NXSVDP1C. It validates metadata only; payload,
# PRS3, palette, command, and texture semantics remain outside this script.
set -euo pipefail
usage() { echo "usage: $0 --plan PATH --bios PATH --disc PATH --capture PATH" >&2; }
hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
value() { local k=$1 l; [[ $(grep -Ec "^${k}=" "$plan") -eq 1 ]] || return 1; l=$(grep -E "^${k}=" "$plan"); printf '%s' "${l#*=}"; }
hex_at() { od -An -tx1 -j "$2" -N "$3" "$1" | tr -d ' \n' | tr '[:upper:]' '[:lower:]'; }
be64() { hex_at "$1" "$2" 8; }
be32() { hex_at "$1" "$2" 4; }
norm_hex() { local v; v=$(lower "$1"); [[ "$v" =~ ^[[:xdigit:]]{1,16}$ ]] || return 1; printf '%016s' "$v" | tr ' ' '0'; }
norm_dec() { [[ "$1" =~ ^[0-9]+$ ]] || return 1; printf '%016x' "$1"; }
while (($#)); do case "$1" in --plan|--bios|--disc|--capture) (($# >= 2)) || { usage; exit 2; }; k=${1#--}; printf -v "$k" '%s' "$2"; shift 2;; *) usage; exit 2;; esac; done
: "${plan:?}" "${bios:?}" "${disc:?}" "${capture:?}"
[[ -f "$plan" && -f "$bios" && -f "$disc" && -f "$capture" && $(wc -c < "$capture") -ge 136 ]] || exit 1
[[ $(head -c 8 "$capture") == NXSVDP1C && $(be32 "$capture" 8) == 00000001 && $(be32 "$capture" 12) == 00000088 ]] || exit 1
[[ $(lower "$(hash_file "$bios")") == $(lower "$(value bios_sha256)") && $(lower "$(hash_file "$disc")") == $(lower "$(value disc_sha256)") ]] || exit 1
[[ $(value capture_magic) == NXSVDP1C && $(value capture_version) == 1 && $(value capture_header_bytes) == 136 ]] || exit 1
check_hex() { [[ $(be64 "$capture" "$2") == $(norm_hex "$(value "$1")") ]]; }
check_dec() { [[ $(be64 "$capture" "$2") == $(norm_dec "$(value "$1")") ]]; }
check_dec route_epoch 16 && check_hex package_fnv1a64 24 && check_hex card_fnv1a64 32 && check_hex dgn_fnv1a64 40 && check_dec dgn_size 48 && check_hex face_fnv1a64 56 && check_hex descriptor_fnv1a64 64 && check_hex image_candidate_fnv1a64 72 && check_hex palette_candidate_fnv1a64 80 || exit 1
po=$(be32 "$capture" 88); pl=$(be32 "$capture" 92)
[[ $((16#$po)) -eq 136 && $((16#$pl)) -gt 0 && $((16#$po + 16#$pl)) -le $(wc -c < "$capture") && $(be64 "$capture" 104) != 0000000000000000 && $(be64 "$capture" 112) != 0000000000000000 && $(be64 "$capture" 120) != 0000000000000000 && $(be64 "$capture" 128) != 0000000000000000 ]] || exit 1
printf 'NXSVDP1C local artifact metadata: PASS\n'
