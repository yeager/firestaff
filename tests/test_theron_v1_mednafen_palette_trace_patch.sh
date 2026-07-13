#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
patch_file=$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch
source_tree=${MEDNAFEN_SOURCE:-}

if [[ ! -f "$patch_file" ]]; then
    printf 'FAIL: Mednafen trace patch is missing\n' >&2
    exit 1
fi
if ! grep -Fq 'dynamic_huc6260_palette_store pc=%04x physical_pc=%08x opcode=8d address=%04x accumulator=%02x' "$patch_file" ||
   ! grep -Fq 'address >= 0x0402 && address <= 0x0405' "$patch_file" ||
   ! grep -Fq 'TheronIrq2TraceDynamicControllerStateSeen' "$patch_file"; then
    printf 'FAIL: patch no longer records bounded, post-controller HuC6260 stores\n' >&2
    exit 1
fi

if [[ -z "$source_tree" || ! -d "$source_tree" ]]; then
    printf 'SKIP: MEDNAFEN_SOURCE is required for patch dry-run\n'
    exit 0
fi

patch --dry-run --forward -p1 -d "$source_tree" < "$patch_file"
printf 'PASS: Mednafen patch dry-runs and retains raw HuC6260 store receipts\n'
