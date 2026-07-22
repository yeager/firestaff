#include "nexus_v1_sal_payload_capture_admission.h"

#include <stdio.h>
#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void set_valid(Nexus_V1_SalPayloadCaptureAdmissionInput *input,
                      Nexus_V1_LevelSoundCaptureTargetReceipt *target,
                      Nexus_V1_LevelSoundTraceAdmissionReceipt *trace,
                      Nexus_V1_SalDispatchEvidenceReceipt *evidence,
                      Nexus_V1_SalContainerProvenanceReceipt *container,
                      Nexus_V1_SndlevMapProvenanceReceipt *map,
                      Nexus_V1_SndlevMapRowProvenanceReceipt *row,
                      uint8_t *sal, size_t sal_size)
{
    uint64_t sal_fnv = fnv1a64(sal, sal_size);
    memset(target, 0, sizeof(*target));
    memset(trace, 0, sizeof(*trace));
    memset(evidence, 0, sizeof(*evidence));
    memset(container, 0, sizeof(*container));
    memset(map, 0, sizeof(*map));
    memset(row, 0, sizeof(*row));
    memset(input, 0, sizeof(*input));

    target->valid = target->original_saturn_driver_capture_required = 1;
    target->level_index = 3; target->raw_map_selector = 0x43;
    target->map_attribute = 7; target->sal_offset = 16; target->sal_size = 12;
    target->canonical_sal_fnv1a64 = sal_fnv;
    target->canonical_map_fnv1a64 = UINT64_C(0x55aa);
    target->no_playback_only = 1;

    trace->status = NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE;
    trace->level_index = target->level_index;
    trace->raw_map_selector = target->raw_map_selector;
    trace->map_attribute = target->map_attribute;
    trace->sal_offset = target->sal_offset; trace->sal_size = target->sal_size;
    trace->canonical_sal_fnv1a64 = target->canonical_sal_fnv1a64;
    trace->canonical_map_fnv1a64 = target->canonical_map_fnv1a64;
    trace->capture_target_bound = trace->mednafen_debugger_provenance = 1;
    trace->original_saturn_execution_claimed = trace->trace_sha256_present = 1;
    trace->raw_trace_bytes_bound = trace->trace_chain_complete = 1;
    trace->raw_trace_fnv1a64 = UINT64_C(0x1234); trace->raw_trace_byte_count = 99;
    trace->selector_dispatch_pc = 0x06001000U; trace->sal_read_pc = 0x06001020U;
    trace->driver_output_pc = 0x06001040U; trace->blocks_real_sfx_playback = 1;

    evidence->status = NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED;
    evidence->level_index = target->level_index; evidence->raw_trace_bound = 1;
    evidence->raw_trace_fnv1a64 = trace->raw_trace_fnv1a64;
    evidence->raw_trace_byte_count = trace->raw_trace_byte_count;
    evidence->selector_dispatch_observed = evidence->sal_read_observed = 1;
    evidence->driver_output_observed = evidence->observation_order_proven = 1;
    evidence->blocks_real_sfx_playback = 1;

    container->valid = 1; container->source_fnv1a64 = sal_fnv;
    container->source_byte_count = sal_size; container->descriptor_offset = 8;
    container->descriptor_length = sal_size - 8;
    container->descriptor_fnv1a64 = fnv1a64(sal + 8, sal_size - 8);
    map->valid = 1; map->source_fnv1a64 = target->canonical_map_fnv1a64;
    map->record_count = 1; map->table_fnv1a64 = UINT64_C(0x7777);
    row->valid = 1; row->row_index = 0; row->table_fnv1a64 = map->table_fnv1a64;
    row->row_fnv1a64 = UINT64_C(0x8888);

    input->target = target; input->trace = trace; input->evidence = evidence;
    input->container = container; input->map = map; input->map_row = row;
    input->active_sal_bytes = sal; input->active_sal_byte_count = sal_size;
}

int main(void)
{
    uint8_t sal[64] = { 'd','s','p','0','1','.','E','X' };
    Nexus_V1_SalPayloadCaptureAdmissionInput input;
    Nexus_V1_LevelSoundCaptureTargetReceipt target;
    Nexus_V1_LevelSoundTraceAdmissionReceipt trace;
    Nexus_V1_SalDispatchEvidenceReceipt evidence;
    Nexus_V1_SalContainerProvenanceReceipt container;
    Nexus_V1_SndlevMapProvenanceReceipt map;
    Nexus_V1_SndlevMapRowProvenanceReceipt row;
    Nexus_V1_SalPayloadCaptureAdmissionReceipt receipt;
    size_t index;

    for (index = 8U; index < sizeof(sal); ++index) sal[index] = (uint8_t)index;
    set_valid(&input, &target, &trace, &evidence, &container, &map, &row,
              sal, sizeof(sal));
    if (!nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_ADMITTED_OPAQUE ||
        !receipt.active_source_bound || !receipt.original_saturn_observations_bound ||
        receipt.sal_window_offset != 16U || receipt.sal_window_size != 12U ||
        receipt.sal_window_fnv1a64 != fnv1a64(sal + 16, 12U) ||
        receipt.payload_decode_proven || receipt.playback_permitted ||
        !receipt.blocks_real_sfx_playback || receipt.fallback_audio_permitted) return 1;

    sal[16] ^= 1U;
    if (nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE) return 1;
    sal[16] ^= 1U;
    set_valid(&input, &target, &trace, &evidence, &container, &map, &row,
              sal, sizeof(sal));
    target.sal_offset = 7;
    if (nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE) return 1;
    set_valid(&input, &target, &trace, &evidence, &container, &map, &row,
              sal, sizeof(sal));
    evidence.raw_trace_fnv1a64++;
    if (nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_TRACE) return 1;
    set_valid(&input, &target, &trace, &evidence, &container, &map, &row,
              sal, sizeof(sal));
    row.table_fnv1a64++;
    if (nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE) return 1;
    set_valid(&input, &target, &trace, &evidence, &container, &map, &row,
              sal, sizeof(sal));
    trace.sal_decode_proven = 1;
    if (nexus_v1_sal_payload_capture_admit(&input, &receipt) ||
        receipt.status != NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_TRACE) return 1;
    puts("test_nexus_v1_sal_payload_capture_admission: PASS");
    return 0;
}
