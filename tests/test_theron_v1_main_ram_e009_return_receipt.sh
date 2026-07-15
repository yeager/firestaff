#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
trace=$(mktemp)
bad=$(mktemp)
trap 'rm -f "$trace" "$bad"' EXIT

cat >"$trace" <<'EOF'
main_ram_loader_e009_dispatch sequence=4 logical_pc=3840 physical_pc=1f1840 a=20 x=ff y=04
main_ram_loader_e009_return sequence=4 logical_pc=3843 physical_pc=1f1843
EOF
perl "$repo/scripts/verify_theron_main_ram_e009_return_receipt.pl" "$trace"

sed 's/physical_pc=1f1843/physical_pc=1f1844/' "$trace" >"$bad"
if perl "$repo/scripts/verify_theron_main_ram_e009_return_receipt.pl" "$bad"; then
    printf 'FAIL: verifier accepted an incorrect e009 return address\n' >&2
    exit 1
fi
