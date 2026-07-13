#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
patch_file=$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch
input_patch_file=$repo/scripts/mednafen_1.32.1_theron_input_trace.patch
state_patch_file=$repo/scripts/mednafen_1.32.1_theron_pcecd_state_trace.patch
input_state_patch_file=$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch
host_input_patch_file=$repo/scripts/mednafen_1.32.1_theron_host_input_trace.patch

if ! grep -Fq 'system_card_controller_state_write pc=%04x physical_pc=%08x address=2241 accumulator=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_state_store pc=%04x physical_pc=%08x opcode=%02x accumulator=%02x state_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_cd_register_store pc=%04x physical_pc=%08x opcode=%02x address=%04x accumulator=%02x value_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_result_store pc=%04x physical_pc=%08x opcode=%02x accumulator=%02x value_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_result_memory_write logical=222d physical=%08x value=%02x' "$patch_file" ||
   ! grep -Fq 'cd_interface_raw_sector_read lba=%d bytes=2352 span_offset=0 span_bytes=32 span_fnv1a=%08x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_wait_sample callback=%llu state_2241=%02x state_write_count=%u cd_1800=%02x' "$patch_file" ||
   ! grep -Fq 'TheronIrq2TraceWatchedPC' "$patch_file" ||
   ! grep -Fq 'stage2_4090=%02x%02x%02x stage2_fc=%02x' "$patch_file" ||
   ! grep -Fq 'stage2_disassembly_pc=%04x instruction=%s' "$patch_file" ||
   ! grep -Fq 'distort CD timing' "$patch_file" ||
   ! grep -Fq 'address == 0x2241' "$patch_file"; then
    printf 'FAIL: Mednafen patch no longer retains bounded controller-state evidence\n' >&2
    exit 1
fi

if ! grep -Fq 'pce_cd_register_read cpu_pc=%04x physical=%08x data=%02x peek=%u' "$state_patch_file" ||
   ! grep -Fq 'pce_cd_irq cpu_pc=%04x type=%04x port2=%02x port3=%02x' "$state_patch_file" ||
   ! grep -Fq '!PeekMode && (A & 0xf) <= 4' "$state_patch_file" ||
   ! grep -Fq 'source=mednafen-pce-instrumented-cd-state' "$state_patch_file"; then
    printf 'FAIL: Mednafen state patch no longer retains raw PCECD read/IRQ evidence\n' >&2
    exit 1
fi

if ! grep -Fq 'source=mednafen-pce-instrumented-input' "$input_patch_file" ||
   ! grep -Fq 'pce_input_port0 raw=%04x' "$input_patch_file" ||
   ! grep -Fq 'MDFN_de16lsb(data_ptr[0])' "$input_patch_file"; then
    printf 'FAIL: Mednafen input patch no longer retains raw port-0 evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'pce_input_read register=%04x raw=%04x sel=%u clr=%u index=%u' "$input_state_patch_file" ||
   ! grep -Fq 'pce_input_write register=%04x data=%02x sel_before=%u clr_before=%u index=%u' "$input_state_patch_file"; then
    printf 'FAIL: Mednafen input-state patch no longer retains raw port transaction evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'source=mednafen-host-input-events' "$host_input_patch_file" ||
   ! grep -Fq 'host_key_event type=%s scancode=%u repeat=%u' "$host_input_patch_file"; then
    printf 'FAIL: Mednafen host-input patch no longer retains raw SDL key-event evidence\n' >&2
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
patch -d "$scratch/source" -p1 --batch --forward <"$input_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$state_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$input_state_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$host_input_patch_file"
printf 'PASS: Mednafen patches dry-run with controller, host/raw input, PCECD, and port transaction evidence\n'
