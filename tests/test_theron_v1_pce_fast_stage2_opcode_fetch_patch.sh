#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_script=$repo/scripts/build_mednafen_theron_irq2_trace.sh
patch_file=$repo/scripts/mednafen_1.32.1_theron_pce_fast_stage2_opcode_fetch_trace.patch

if ! grep -Fq 'TheronTraceInstructionPC >= 0x4002' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC < 0x40d5' "$patch_file" ||
   ! grep -Fq 'FIRESTAFF_THERON_STAGE2_OPCODE_FETCH_TRACE' "$patch_file" ||
   ! grep -Fq 'stage2_opcode_fetch sequence=%u logical_pc=%04x physical_pc=%06x mpr_slot=%u mpr_bank=%02x opcode=%02x' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPhysicalPC, mpr_slot, mpr_bank, b1);' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC >= 0x46b8' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC <= 0x46c9' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC >= 0x5e4d' "$patch_file" ||
   ! grep -Fq 'TheronTraceInstructionPC >= 0x5e7d' "$patch_file" ||
   ! grep -Fq 'FIRESTAFF_THERON_STAGE2_VCE_CALL_TRACE' "$patch_file" ||
   ! grep -Fq 'RdMem(0x1c)' "$patch_file" ||
   ! grep -Fq 'RdMem(0x1d)' "$patch_file" ||
   ! grep -Fq 'stream_pointer=%04x stream_pos=%04x window=' "$patch_file" ||
   ! grep -Fq 'FIRESTAFF_PATCH_TAB_CONTEXT' "$build_script"; then
    printf 'FAIL: Fast-core probe is missing opcode, VCE-call, or stream-cursor fields\n' >&2
    exit 1
fi

if ! awk '
    /^\+[[:space:]]*if\(TheronTraceInstructionPC >= 0x4002/ { in_window = 1 }
    /^FIRESTAFF_PATCH_TAB_CONTEXT[[:space:]]*b1 = RdAtPC\(\);$/ { if (in_window) exit 1; saw_fetch = 1 }
    /^\+[[:space:]]*fprintf\(stage2_opcode_trace,/ { if (!saw_fetch || !in_window) exit 1; saw_record = 1 }
    /^\+[[:space:]]*"stage2_opcode_fetch/ { if (!saw_record) exit 1; saw_format = 1 }
    /^\+[[:space:]]*stage2_opcode_sequence\+\+, TheronTraceInstructionPC,/ { if (!saw_format) exit 1; saw_pc = 1 }
    /^\+[[:space:]]*TheronTraceInstructionPhysicalPC, mpr_slot, mpr_bank, b1\);/ { if (!saw_pc) exit 1; saw_b1 = 1 }
    END { if (!saw_fetch || !in_window || !saw_record || !saw_format || !saw_pc || !saw_b1) exit 1 }
' "$patch_file"; then
    printf 'FAIL: probe must record b1 directly after the dispatch fetch and PC/MPR capture\n' >&2
    exit 1
fi

if ! awk '
    /mednafen_1\.32\.1_theron_pce_fast_instruction_pc_trace\.patch/ { instruction_pc = NR }
    /theron_stage2_mpr1_probe\.patch/ { mpr = NR }
    /mednafen_1\.32\.1_theron_pce_fast_stage2_selector_pc_trace\.patch/ { selector = NR }
    /mednafen_1\.32\.1_theron_pce_fast_stage2_opcode_fetch_trace\.patch/ { opcode = NR }
    END { if (!(instruction_pc && mpr && selector && opcode && instruction_pc < mpr && mpr < selector && selector < opcode)) exit 1 }
' "$build_script"; then
    printf 'FAIL: opcode probe must be applied after Fast-core PC/MPR capture patches\n' >&2
    exit 1
fi

printf 'PASS: bounded PCE Fast probe records dispatched opcodes and VCE handler stream-cursor windows\n'
printf 'LIMIT: the trace still needs source-origin and screen-state correlation before native presentation\n'
