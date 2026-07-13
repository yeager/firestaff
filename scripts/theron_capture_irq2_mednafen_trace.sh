#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 4 ]; then
    printf 'usage: %s CUE TRACK02 SYSCARD3 TRACE\n' "$0" >&2
    exit 2
fi

cue=$1
track02=$2
syscard=$3
trace=$4
repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
mednafen=${MEDNAFEN:-/opt/homebrew/bin/mednafen}

if [ ! -x "$mednafen" ]; then
    printf 'FAIL: Mednafen executable is unavailable: %s\n' "$mednafen" >&2
    exit 1
fi

track_md5=$(md5 -q "$track02")
syscard_md5=$(md5 -q "$syscard")
case "$track_md5" in
    b7afb338ad31be1025b53f9aff12d73a|f23601102138f87c33025877767ebf76) ;;
    *) printf 'FAIL: Track 02 MD5 is not an authenticated JP/US raw BIN\n' >&2; exit 1 ;;
esac
if [ "$syscard_md5" != ff1a674273fe3540ccef576376407d1d ]; then
    printf 'FAIL: System Card 3.0 MD5 mismatch\n' >&2
    exit 1
fi
if [ ! -s "$trace" ]; then
    printf 'TRACE REQUIRED: launch Mednafen debugger and export ordered key=value registers to %s\n' "$trace" >&2
    printf 'Required: source, variant, stage3_track02_record, cd_read_return_pc, irq2_entry_pc, cd_state_pc, cd_state_branch_pc, f5_after_cd_read, f5_at_irq2_entry, cd_status_1802, cd_status_1803, f2_before_merge, f2_at_branch\n' >&2
    printf 'Stock Mednafen trace logs (debugger key l) include CPU registers only, not RAM $f5, $f2, $1802, or $1803.\n' >&2
    printf 'Use scripts/build_mednafen_theron_irq2_trace.sh, then set MEDNAFEN to its output and rerun.\n' >&2
    printf 'The harness therefore rejects stock trace logs and does not invent register values.\n' >&2
    exec env FIRESTAFF_THERON_IRQ2_TRACE="$trace" "$mednafen" -sound 0 \
        -debugger.autostepmode 0 -pce.arcadecard 0 -pce.cdbios "$syscard" "$cue"
fi

out=${TMPDIR:-/tmp}/firestaff_theron_irq2_trace_harness
cc -std=c11 -Wall -Wextra -Werror -I"$repo/include" -I"$repo/src" \
  "$repo/src/theron/theron_v1_track02.c" \
  "$repo/src/theron/theron_v1_stage2_runtime_handoff.c" \
  "$repo/src/theron/theron_v1_stage3_manifest_evidence.c" \
  "$repo/src/theron/theron_v1_later_record_correlation.c" \
  "$repo/src/theron/theron_v1_stage3_irq2_dispatch.c" \
  "$repo/src/theron/theron_v1_stage3_mode1_header.c" \
  "$repo/src/theron/theron_v1_system_card_irq2_cd_state_gate.c" \
  "$repo/src/theron/theron_v1_irq2_live_trace_gate.c" \
  "$repo/src/theron/theron_v1_trace_v3_schema.c" \
  "$repo/probes/theron/firestaff_theron_v1_mednafen_irq2_trace_harness.c" \
  -o "$out"
"$out" "$track02" "$track_md5" "$syscard" "$trace"
