#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_script="$repo/scripts/build_mednafen_theron_irq2_trace.sh"
capture_script="$repo/scripts/capture_theron_mednafen_live_trace.sh"
patch_file="$repo/scripts/mednafen_1.32.1_theron_pce_fast_vram_commit_trace.patch"
cmake_file="$repo/CMakeLists.txt"

if ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V2' "$patch_file" ||
   ! grep -Fq 'source=mednafen-1.32.1-pce-fast-vdc-vwr-commit-mpr-code-window' "$patch_file" ||
   ! grep -Fq 'scope=cpu-port-vwr-commits-only;dma-writes-excluded' "$patch_file" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_LIMIT' "$patch_file" ||
   ! grep -Fq 'parsed >= 1 && parsed <= 1048576' "$patch_file" ||
   ! grep -Fq 'fopen(path, "wx")' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPhysicalPC' "$patch_file" ||
   ! grep -Fq 'HuCPU.MPR[mpr_slot]' "$patch_file" ||
   ! grep -Fq 'TheronPCEFastReadCodeWindow(logical_pc, code_bytes, code_mpr_banks);' "$patch_file" ||
   ! grep -Fq 'HuCPU.FastPageR[slot] + address' "$patch_file" ||
   ! grep -Fq 'vram_commit sequence=%u chip=%d display_counter=%u logical_pc=%04x physical_pc=%06x mpr_slot=%u mpr_bank=%02x code_mpr_banks=%02x%02x%02x%02x%02x%02x%02x%02x code_bytes=%02x%02x%02x%02x%02x%02x%02x%02x address=%04x value=%04x low=%02x high=%02x' "$patch_file" ||
   ! grep -Fq 'TheronTracePCEFastVRAMCommit(chip, vdc->display_counter, vdc->MAWR, (V << 8) | vdc->write_latch);' "$patch_file" ||
   ! grep -Fq 'THERON_CAPTURE_PCE_FAST_VRAM_COMMIT_TRACE' "$capture_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE="$pce_fast_vram_commit_trace_env"' "$capture_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_V2' "$capture_script" ||
   ! grep -Fq 'code_bytes_field[1] != "code_bytes" || !is_hex(code_bytes_field[2], 16)' "$capture_script" ||
   ! grep -Fq 'code_mpr_banks_field[1] != "code_mpr_banks" || !is_hex(code_mpr_banks_field[2], 16)' "$capture_script" ||
   ! grep -Fq 'mpr_slot_field[2] != int(logical_pc_value / 8192)' "$capture_script" ||
   ! grep -Fq 'physical_pc_field[2] != sprintf("%06x", (mpr_bank_value * 8192) + (logical_pc_value % 8192))' "$capture_script" ||
   ! grep -Fq 'scope=cpu-port-vwr-commits-only;dma-writes-excluded' "$capture_script" ||
   ! grep -Fq 'pce_fast_vram_commit_trace_requested=%s' "$capture_script" ||
   ! grep -Fq 'pce_fast_vram_commit_trace_limit=%s' "$capture_script" ||
   ! grep -Fq 'pce_fast_vram_commit_trace_sha256=%s' "$capture_script" ||
   ! grep -Fq 'FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_SUPPORT' "$build_script" ||
   ! grep -Fq 'mednafen_1.32.1_theron_pce_fast_vram_commit_trace.patch' "$build_script" ||
   ! grep -Fq 'NAME theron_v1_mednafen_pce_fast_vram_commit_patch' "$cmake_file" ||
   ! grep -Fq 'tests/test_theron_v1_mednafen_pce_fast_vram_commit_patch.sh' "$cmake_file" ||
   ! grep -Fq 'pce_fast_snapshot" == 1 || "$pce_fast_vram_commit_trace" == 1' "$build_script"; then
    printf '%s\n' 'FAIL: PCE Fast VWR commit trace must be bounded, opt-in, and tied to instruction PCs' >&2
    exit 1
fi

if [[ -z ${MEDNAFEN_SOURCE:-} ]]; then
    printf '%s\n' 'SKIP: MEDNAFEN_SOURCE is required for patch-application verification'
    exit 0
fi

scratch_root=${THERON_CAPTURE_SCRATCH_ROOT:-"$repo/.codex-scratch"}
mkdir -p "$scratch_root"
scratch=$(mktemp -d "$scratch_root/firestaff-theron-pce-fast-vram-commit.XXXXXX")
trap 'rm -rf "$scratch"' EXIT
FIRESTAFF_MEDNAFEN_BUILD_ROOT="$scratch/build" \
FIRESTAFF_MEDNAFEN_PATCH_ONLY=1 \
FIRESTAFF_THERON_PCE_FAST_VRAM_COMMIT_TRACE_SUPPORT=1 \
    "$build_script" "$MEDNAFEN_SOURCE"
