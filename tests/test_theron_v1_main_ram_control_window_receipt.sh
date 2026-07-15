#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
trap 'rm -f "$trace" "$trace.game"' EXIT

cat >"$trace" <<'EOF'
main_ram_control_read sequence=0 logical_address=21fb physical_address=1f01fb value=ff reader_pc=cb18 reader_physical_pc=002b18
main_ram_control_read sequence=1 logical_address=21f9 physical_address=1f01f9 value=ff reader_pc=e8e3 reader_physical_pc=0008e3
main_ram_control_read sequence=2 logical_address=21fa physical_address=1f01fa value=e8 reader_pc=e056 reader_physical_pc=1fe056
EOF
perl "$repo/scripts/verify_theron_main_ram_control_window_receipt.pl" "$trace"

sed 's/reader_physical_pc=002b18/reader_physical_pc=1f1173/' "$trace" >"$trace.game"
if perl "$repo/scripts/verify_theron_main_ram_control_window_receipt.pl" "$trace.game"; then
    echo "FAIL: accepted a game-owned control-window reader" >&2
    exit 1
fi
