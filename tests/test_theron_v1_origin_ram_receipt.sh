#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-origin-ram.XXXXXX")
trap 'rm -f "$trace"' EXIT

printf '%s\n' \
  'pce_cd_origin_ram_receipt generation=4 source_lba=4233 source_offset=0 reader_pc=ea9c writer_pc=ea52 logical_destination=2256 physical_destination=001f0256 value=78' \
  >"$trace"
perl "$repo/scripts/verify_theron_origin_ram_receipt.pl" "$trace"

if printf '%s\n' \
  'pce_cd_origin_ram_receipt generation=4 source_lba=4233 source_offset=2048 reader_pc=ea9c writer_pc=ea52 logical_destination=2256 physical_destination=001f0256 value=78' \
  >"$trace" && perl "$repo/scripts/verify_theron_origin_ram_receipt.pl" "$trace"; then
    printf 'FAIL: verifier accepted an out-of-sector offset\n' >&2
    exit 1
fi

printf 'PASS: source-backed Track02 FIFO-to-RAM receipt verifier\n'
