#!/usr/bin/env bash
set -euo pipefail
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
trap 'rm -f "$trace"' EXIT
cat >"$trace" <<'EOF'
main_ram_loader_write sequence=1 dispatch_sequence=0 logical_destination=300b physical_destination=1f100b value=00 writer_pc=317a writer_physical_pc=1f117a
main_ram_loader_write sequence=2 dispatch_sequence=0 logical_destination=30a3 physical_destination=1f10a3 value=ff writer_pc=3190 writer_physical_pc=1f1190
EOF
perl "$repo/scripts/verify_theron_main_ram_loader_init_receipt.pl" "$trace"
