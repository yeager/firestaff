#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
patch_file=$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch
cd_register_patch_file=$repo/scripts/mednafen_1.32.1_theron_pcecd_trace.patch
input_patch_file=$repo/scripts/mednafen_1.32.1_theron_input_trace.patch
state_patch_file=$repo/scripts/mednafen_1.32.1_theron_pcecd_state_trace.patch
input_state_patch_file=$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch
host_input_patch_file=$repo/scripts/mednafen_1.32.1_theron_host_input_trace.patch
transfer_patch_file=$repo/scripts/mednafen_1.32.1_theron_cd_transfer_trace.patch
caller_patch_file=$repo/scripts/mednafen_1.32.1_theron_cd_caller_trace.patch
execution_patch_file=$repo/scripts/mednafen_1.32.1_theron_post_stage2_execution_trace.patch
e009_patch_file=$repo/scripts/mednafen_1.32.1_theron_post_stage2_e009_trace.patch
e00f_patch_file=$repo/scripts/mednafen_1.32.1_theron_post_stage2_e00f_trace.patch
later_raw_patch_file=$repo/scripts/mednafen_1.32.1_theron_later_raw_sector_trace.patch
later_fifo_patch_file=$repo/scripts/mednafen_1.32.1_theron_later_fifo_generation_trace.patch
generation7_receipt_patch_file=$repo/scripts/mednafen_1.32.1_theron_generation7_fifo_ram_receipt.patch
main_ram_loader_patch_file=$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_trace.patch
main_ram_e009_dispatch_patch_file=$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_dispatch_trace.patch
main_ram_e009_fifo_patch_file=$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_fifo_trace.patch
build_script=$repo/scripts/build_mednafen_theron_irq2_trace.sh

if ! grep -Fq 'system_card_controller_state_write pc=%04x physical_pc=%08x address=2241 accumulator=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_state_store pc=%04x physical_pc=%08x opcode=%02x accumulator=%02x state_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_cd_register_store pc=%04x physical_pc=%08x opcode=%02x address=%04x accumulator=%02x value_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_result_store pc=%04x physical_pc=%08x opcode=%02x accumulator=%02x value_before=%02x' "$patch_file" ||
   ! grep -Fq 'system_card_controller_result_memory_write logical=222d physical=%08x value=%02x' "$patch_file" ||
   ! grep -Fq 'cd_interface_raw_sector_read lba=%d bytes=2352 sector_fnv1a=%08x span_offset=0 span_bytes=32 span_fnv1a=%08x' "$patch_file" ||
   ! grep -Fq 'for(unsigned i = 0; i < 2352; i++)' "$patch_file" ||
   ! grep -Fq 'system_card_controller_wait_sample callback=%llu state_2241=%02x state_write_count=%u cd_1800=%02x' "$patch_file" ||
   ! grep -Fq 'TheronIrq2TraceWatchedPC' "$patch_file" ||
   ! grep -Fq 'stage2_4090=%02x%02x%02x stage2_fc=%02x' "$patch_file" ||
   ! grep -Fq 'stage2_system_card_call pc=%04x return_pc=%04x target=%04x a=%02x x=%02x y=%02x p=%02x' "$patch_file" ||
   ! grep -Fq 'mpr0=%02x table=%02x%02x%02x%02x' "$patch_file" ||
   ! grep -Fq 'stage2_system_card_return pc=%04x call_pc=%04x a=%02x x=%02x y=%02x p=%02x' "$patch_file" ||
   ! grep -Fq 'logical_pc == 0x40cd || logical_pc == 0x40a4' "$patch_file" ||
   ! grep -Fq 'logical_pc == 0x40d0 || logical_pc == 0x40a7' "$patch_file" ||
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
   ! grep -Fq 'host_sdl_event type=%u' "$host_input_patch_file" ||
   ! grep -Fq 'host_window_event event=%u' "$host_input_patch_file" ||
   ! grep -Fq 'host_focus_state have=%u' "$host_input_patch_file" ||
   ! grep -Fq 'host_key_event type=%s scancode=%u repeat=%u' "$host_input_patch_file"; then
    printf 'FAIL: Mednafen host-input patch no longer retains raw SDL key-event evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'scsi_read_command generation=%u start_lba=%u sector_count=%u' "$transfer_patch_file" ||
   ! grep -Fq 'scsi_read_sector_binding generation=%u start_lba=%u sector_count=%u lba=%d sector_index=%u' "$transfer_patch_file" ||
   ! grep -Fq 'pce_cd_data_read cpu_pc=%04x data=%02x' "$transfer_patch_file" ||
   ! grep -Fq 'pce_cd_data_destination_candidate reader_pc=%04x writer_pc=%04x logical_destination=%04x physical=%08x read_value=%02x stored_value=%02x' "$transfer_patch_file" ||
   ! grep -Fq 'if(read_value != stored_value)' "$transfer_patch_file" ||
   ! grep -Fq 'TheronPCECDTraceDiscardDataReadOnNonMainRAMWrite' "$transfer_patch_file" ||
   ! grep -Fq 'TheronPCECDTraceTakeDataRead' "$transfer_patch_file"; then
    printf 'FAIL: Mednafen transfer patch no longer retains bounded SCSI/FIFO/RAM receipts\n' >&2
    exit 1
fi
if ! grep -Fq 'scsi_read_command generation=%u opcode=%02x cdb=%02x%02x%02x%02x%02x%02x start_lba=%u sector_count=%u' "$caller_patch_file" ||
   ! grep -Fq 'pce_cd_register_write cpu_pc=%04x physical=%08x data=%02x' "$caller_patch_file" ||
   ! grep -Fq '(physAddr & 0xf) == 1 && theron_cd_register_trace_count < 4096' "$caller_patch_file"; then
    printf 'FAIL: Mednafen caller patch no longer retains CDB and CPU-PC evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'post_stage2_execution_page logical_pc=%04x physical_pc=%08x physical_page=%04x' "$execution_patch_file" ||
   ! grep -Fq 'TheronIrq2TracePostStage2PhysicalPageSeen[8192]' "$execution_patch_file"; then
    printf 'FAIL: Mednafen execution patch no longer retains bounded post-stage2 physical-page evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'post_stage2_non_system_card_e009_call caller_pc=%04x physical_pc=%08x record=%06x destination=%04x destination_mode=%02x' "$e009_patch_file" ||
   ! grep -Fq 'TheronIrq2TracePostStage2NonSystemCardE009Count < 16' "$e009_patch_file"; then
    printf 'FAIL: Mednafen e009 patch no longer retains bounded non-System-Card caller evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'post_stage2_non_system_card_e00f_call caller_pc=%04x physical_pc=%08x a=%02x x=%02x y=%02x record=%06x destination=%04x destination_mode=%02x' "$e00f_patch_file" ||
   ! grep -Fq 'TheronIrq2TracePostStage2NonSystemCardE00FCount < 16' "$e00f_patch_file"; then
    printf 'FAIL: Mednafen e00f patch no longer retains bounded non-System-Card caller evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'if(ok && trace_count < 4096)' "$later_raw_patch_file"; then
    printf 'FAIL: later raw-sector patch no longer retains the bounded extended SCSI witness window\n' >&2
    exit 1
fi
if ! grep -Fq 'pce_cd_data_read cpu_pc=%04x data=%02x scsi_generation=%u' "$later_fifo_patch_file" ||
   ! grep -Fq 'TheronSCSITraceCurrentReadGeneration' "$later_fifo_patch_file"; then
    printf 'FAIL: later FIFO patch no longer binds data reads to SCSI generations\n' >&2
    exit 1
fi
if ! grep -Fq 'pce_cd_fifo_read generation=%u fifo_sequence=%llu reader_pc=%04x value=%02x' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'pce_cd_fifo_destination_receipt generation=%u fifo_sequence=%llu reader_pc=%04x logical_destination=%04x physical_destination=%06x value=%02x' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronPCECDTraceDataWrite(address, physical_address, V);' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronPCECDTraceDiscardDataReadOnNonMainRAMWrite(physical_address);' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'pce_cd_fifo_store_mismatch generation=%u fifo_sequence=%llu' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronPCECDTracePostGeneration7RAMWrite(address, physical_address, V, writer_pc,' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'generation7_fifo_window_complete fifo_sequence=%llu' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'post_generation7_main_ram_write writer_pc=%04x writer_physical_pc=%06x' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronPCECDFifoReceiptCapacity = 65536' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronSCSITraceCurrentReadGeneration() >= 7' "$generation7_receipt_patch_file" ||
   ! grep -Fq 'TheronSCSITraceCurrentReadGeneration() <= 10' "$generation7_receipt_patch_file"; then
    printf 'FAIL: later FIFO receipt patch no longer preserves an untruncated CPU-write binding\n' >&2
    exit 1
fi
if ! grep -Fq 'main_ram_loader_jsr logical_pc=%04x physical_pc=%06x target=%04x a=%02x x=%02x y=%02x' "$main_ram_loader_patch_file" ||
   ! grep -Fq 'main_ram_loader_block_transfer logical_pc=%04x physical_pc=%06x operation=%s source=%04x destination=%04x length=%04x' "$main_ram_loader_patch_file" ||
   ! grep -Fq 'FIRESTAFF_THERON_MAIN_RAM_LOADER_TRACE' "$main_ram_loader_patch_file" ||
   ! grep -Fq 'physical_pc >= 0x1f0000 && physical_pc < 0x1f8000' "$main_ram_loader_patch_file"; then
    printf 'FAIL: main-RAM loader patch no longer retains bounded executed control evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'main_ram_loader_e009_dispatch sequence=%u logical_pc=%04x physical_pc=%06x a=%02x x=%02x y=%02x' "$main_ram_e009_dispatch_patch_file" ||
   ! grep -Fq 'if(lastop == 0x20 && first == 0xe009)' "$main_ram_e009_dispatch_patch_file" ||
   ! grep -Fq 'TheronPCECDTraceMainRAMLoaderE009Call' "$main_ram_e009_dispatch_patch_file"; then
    printf 'FAIL: main-RAM e009 dispatch patch no longer retains direct PCECD receipt\n' >&2
    exit 1
fi
if ! grep -Fq 'main_ram_e009_fifo_read dispatch_sequence=%u generation=%u fifo_sequence=%llu reader_pc=%04x value=%02x' "$main_ram_e009_fifo_patch_file" ||
   ! grep -Fq 'if(TheronPCECDMainRAMLoaderE009Active)' "$main_ram_e009_fifo_patch_file"; then
    printf 'FAIL: main-RAM e009 FIFO patch no longer retains dispatch-bounded FIFO evidence\n' >&2
    exit 1
fi
if ! grep -Fq 'FIRESTAFF_MEDNAFEN_SDL2_PREFIX' "$build_script" ||
   ! grep -Fq 'verify_theron_mednafen_sdl2_runtime.sh' "$build_script"; then
    printf 'FAIL: trace build no longer gates capture on a real SDL2 runtime\n' >&2
    exit 1
fi

if [[ -z ${MEDNAFEN_SOURCE:-} ]]; then
    printf 'SKIP: MEDNAFEN_SOURCE is required for patch dry-run\n'
    exit 0
fi

scratch=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-theron-controller-patch.XXXXXX")
trap 'rm -rf "$scratch"' EXIT
cp -R "$MEDNAFEN_SOURCE/." "$scratch/source"
git -C "$scratch/source" apply --whitespace=nowarn "$patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$input_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$cd_register_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$state_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$input_state_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$host_input_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$transfer_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$caller_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$execution_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$e009_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$e00f_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$later_raw_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$later_fifo_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$generation7_receipt_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$main_ram_loader_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$main_ram_e009_dispatch_patch_file"
patch -d "$scratch/source" -p1 --batch --forward <"$main_ram_e009_fifo_patch_file"
printf 'PASS: Mednafen patches dry-run with controller, host/raw input, PCECD, and bounded transfer evidence\n'
