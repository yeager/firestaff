#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
patch_file=$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch

if ! grep -Fq 'system_card_controller_state_write pc=%04x physical_pc=%08x address=2241 accumulator=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_state_store pc=%04x physical_pc=%08x opcode=%02x accumulator=%02x state_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_cd_register_store pc=%04x physical_pc=%08x opcode=%02x address=%04x accumulator=%02x value_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_wait_sample callback=%llu state_2241=%02x state_write_count=%u cd_1800=%02x' "$patch_file" ||
   ! grep -Fq 'address == 0x2241' "$patch_file"; then
    printf 'FAIL: Mednafen patch no longer retains bounded controller-state evidence\n' >&2
    exit 1
fi

if [[ -z ${MEDNAFEN_SOURCE:-} ]]; then
    printf 'SKIP: MEDNAFEN_SOURCE is required for patch dry-run\n'
    exit 0
fi

scratch=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-controller-patch.XXXXXX")
trap 'rm -rf "$scratch"' EXIT
cp -R "$MEDNAFEN_SOURCE/." "$scratch/source"
patch -d "$scratch/source" -p1 --batch --forward <"$patch_file"
printf 'PASS: Mednafen patch dry-runs with controller-state evidence\n'
