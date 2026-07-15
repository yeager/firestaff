#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-game-origin-ram.XXXXXX")
trap 'rm -f "$trace"' EXIT

printf '%s\n' \
  'pce_cd_origin_main_ram_receipt sequence=0 generation=7 source_lba=4847 source_offset=0 reader_pc=eb33 logical_destination=3000 physical_destination=1f1000 writer_pc=1840 writer_physical_pc=1f1840 value=7a' \
  >"$trace"
perl "$repo/scripts/verify_theron_game_owned_origin_ram_receipt.pl" "$trace"

printf '%s\n' \
  'pce_cd_origin_main_ram_receipt sequence=0 generation=4 source_lba=4233 source_offset=0 reader_pc=ea9c logical_destination=2256 physical_destination=1f0256 writer_pc=ea52 writer_physical_pc=000a52 value=78' \
  >"$trace"
if perl "$repo/scripts/verify_theron_game_owned_origin_ram_receipt.pl" "$trace"; then
    printf 'FAIL: verifier accepted a System Card writer\n' >&2
    exit 1
fi

printf 'PASS: game-owned source-backed Track02 FIFO-to-RAM receipt verifier\n'
