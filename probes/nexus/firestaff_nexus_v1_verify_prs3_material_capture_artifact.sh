#!/usr/bin/env bash
# Read-only NXSPRS3M structural verifier. It does not decode the captured body.
set -euo pipefail
usage() { echo "usage: $0 --plan PATH --capture PATH" >&2; }
value() { local k=$1 l; [[ $(grep -Ec "^${k}=" "$plan") -eq 1 ]] || return 1; l=$(grep -E "^${k}=" "$plan"); printf '%s' "${l#*=}"; }
hex() { od -An -tx1 -j "$2" -N "$3" "$1" | tr -d ' \n' | tr '[:upper:]' '[:lower:]'; }
u32() { hex "$1" "$2" 4; }; u64() { hex "$1" "$2" 8; }
hx() { local v; [[ "$1" =~ ^[[:xdigit:]]{1,16}$ ]] || return 1; v=$(printf '%s' "$1" | tr '[:upper:]' '[:lower:]'); printf '%016s' "$v" | tr ' ' '0'; }
dc() { [[ "$1" =~ ^[0-9]+$ ]] || return 1; printf '%016x' "$1"; }
while (($#)); do case "$1" in --plan|--capture) (($#>=2)) || { usage; exit 2; }; k=${1#--}; printf -v "$k" %s "$2"; shift 2;; *) usage; exit 2;; esac; done
: "${plan:?}" "${capture:?}"; [[ -f "$plan" && -f "$capture" && $(wc -c < "$capture") -ge 96 ]] || exit 1
[[ $(head -c 8 "$capture") == NXSPRS3M && $(u32 "$capture" 8) == 00000001 && $(u32 "$capture" 12) == 00000060 ]] || exit 1
chex() { [[ $(u64 "$capture" "$2") == $(hx "$(value "$1")") ]]; }; cdec() { [[ $(u64 "$capture" "$2") == $(dc "$(value "$1")") ]]; }; c32() { [[ $(u32 "$capture" "$2") == $(printf '%08x' "$(value "$1")") ]]; }
cdec route_epoch 16 && chex package_fnv1a64 24 && chex card_fnv1a64 32 && c32 entry_index 40 && c32 compressed_offset 44 && c32 compressed_length 48 && c32 declared_output_bytes 52 && chex compressed_fnv1a64 56 || exit 1
po=$(u32 "$capture" 64); pl=$(u32 "$capture" 68)
[[ $((16#$po)) -eq 96 && $((16#$pl)) -gt 0 && $((16#$po + 16#$pl)) -le $(wc -c < "$capture") && $(u64 "$capture" 80) != 0000000000000000 && $(u64 "$capture" 88) != 0000000000000000 ]] || exit 1
printf 'NXSPRS3M local artifact metadata: PASS\n'
