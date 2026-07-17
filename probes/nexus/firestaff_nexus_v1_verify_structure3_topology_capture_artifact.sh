#!/usr/bin/env bash
# Read-only operator-side check for a locally produced NXS3TOP1 artifact.
# Live route revalidation and payload FNV remain owned by the C import bridge.
set -euo pipefail

usage() { echo "usage: $0 --plan PATH --bios PATH --disc PATH --capture PATH" >&2; }
hash_file() { shasum -a 256 "$1" | awk '{print $1}'; }
lower() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }
value() {
  local key=$1 line
  [[ $(grep -Ec "^${key}=" "$plan") -eq 1 ]] || return 1
  line=$(grep -E "^${key}=" "$plan")
  printf '%s' "${line#*=}"
}
hex_at() { od -An -tx1 -j "$2" -N "$3" "$1" | tr -d ' \n' | tr '[:upper:]' '[:lower:]'; }
norm_hex() { local v; v=$(lower "$1"); [[ "$v" =~ ^[[:xdigit:]]{1,16}$ ]] || return 1; printf '%016s' "$v" | tr ' ' '0'; }
norm_dec() { [[ "$1" =~ ^[0-9]+$ ]] || return 1; printf '%016x' "$1"; }
be64() { hex_at "$1" "$2" 8; }
be32() { hex_at "$1" "$2" 4; }

while (($#)); do
  case "$1" in
    --plan|--bios|--disc|--capture) (($# >= 2)) || { usage; exit 2; }; key=${1#--}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done
: "${plan:?}" "${bios:?}" "${disc:?}" "${capture:?}"
[[ -f "$plan" && -f "$bios" && -f "$disc" && -f "$capture" && $(wc -c < "$capture") -ge 124 ]] || exit 1
[[ $(head -c 8 "$capture") == NXS3TOP1 && $(be32 "$capture" 8) == 00000001 && $(be32 "$capture" 12) == 0000007c ]] || exit 1
[[ $(lower "$(hash_file "$bios")") == $(lower "$(value bios_sha256)") ]] || exit 1
[[ $(lower "$(hash_file "$disc")") == $(lower "$(value disc_sha256)") ]] || exit 1
[[ $(value capture_magic) == NXS3TOP1 && $(value capture_version) == 1 && $(value capture_header_bytes) == 124 ]] || exit 1

check64() { [[ $(be64 "$capture" "$2") == $(norm_hex "$(value "$1")") ]]; }
check64_dec() { [[ $(be64 "$capture" "$2") == $(norm_dec "$(value "$1")") ]]; }
check32() { [[ $(be32 "$capture" "$2") == $(printf '%08x' "$(value "$1")") ]]; }
check64_dec route_epoch 16 && check64 package_fnv1a64 24 && check64 card_fnv1a64 32 &&
check64 dgn_fnv1a64 40 && check64_dec dgn_size 48 && check32 structure1f_entry_index 56 &&
check32 structure3_entry_index 60 && check32 face_ordinal 64 &&
check32 vertex_table_offset 68 && check32 vertex_table_length 72 &&
check64 vertex_table_fnv1a64 76 && check64 referenced_vertex_rows_fnv1a64 84 &&
check32 normal_offset 92 && check32 normal_length 96 && check64 normal_fnv1a64 100 || exit 1

payload_offset=$(be32 "$capture" 108)
payload_length=$(be32 "$capture" 112)
[[ $((16#$payload_offset)) -eq 124 && $((16#$payload_length)) -gt 0 &&
   $((16#$payload_offset + 16#$payload_length)) -le $(wc -c < "$capture") ]] || exit 1
printf 'NXS3TOP1 local artifact metadata: PASS\n'
