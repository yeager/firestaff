#include "nexus_v1_sal_payload_capture_admission.h"

#include <string.h>

static uint64_t nexus_v1_sal_payload_fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    if (!bytes || size == 0U) return 0U;
    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int nexus_v1_sal_payload_target_matches_trace(
    const Nexus_V1_LevelSoundCaptureTargetReceipt *target,
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace)
{
    return target->valid && trace->status == NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE &&
        trace->capture_target_bound && trace->mednafen_debugger_provenance &&
        trace->original_saturn_execution_claimed && trace->trace_sha256_present &&
        trace->raw_trace_bytes_bound && trace->raw_trace_fnv1a64 &&
        trace->raw_trace_byte_count != 0U && trace->trace_chain_complete &&
        target->level_index == trace->level_index &&
        target->raw_map_selector == trace->raw_map_selector &&
        target->map_attribute == trace->map_attribute &&
        target->sal_offset == trace->sal_offset && target->sal_size == trace->sal_size &&
        target->canonical_sal_fnv1a64 == trace->canonical_sal_fnv1a64 &&
        target->canonical_map_fnv1a64 == trace->canonical_map_fnv1a64 &&
        !target->sal_decode_proven && !target->playback_permitted &&
        target->no_playback_only && !target->fallback_visuals_permitted &&
        !trace->driver_dispatch_proven && !trace->sal_decode_proven &&
        !trace->playback_permitted && trace->blocks_real_sfx_playback &&
        !trace->fallback_visuals_permitted;
}

int nexus_v1_sal_payload_capture_admit(
    const Nexus_V1_SalPayloadCaptureAdmissionInput *input,
    Nexus_V1_SalPayloadCaptureAdmissionReceipt *out_receipt)
{
    Nexus_V1_SalPayloadCaptureAdmissionReceipt receipt;
    const Nexus_V1_LevelSoundCaptureTargetReceipt *target;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    const Nexus_V1_SalDispatchEvidenceReceipt *evidence;
    const Nexus_V1_SalContainerProvenanceReceipt *container;
    const Nexus_V1_SndlevMapProvenanceReceipt *map;
    const Nexus_V1_SndlevMapRowProvenanceReceipt *row;
    uint64_t source_fnv;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = NEXUS_V1_SAL_PAYLOAD_CAPTURE_MISSING;
    receipt.level_index = -1;
    receipt.blocks_real_sfx_playback = 1;
    if (!input || !(target = input->target) || !(trace = input->trace) ||
        !(evidence = input->evidence) || !(container = input->container) ||
        !(map = input->map) || !(row = input->map_row) ||
        !input->active_sal_bytes || input->active_sal_byte_count == 0U) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.level_index = target->level_index;
    if (!container->valid || !map->valid || !row->valid ||
        container->codec_proven || container->playback_permitted ||
        map->playback_permitted || row->playback_permitted ||
        container->source_byte_count != input->active_sal_byte_count ||
        target->sal_offset < 0 || target->sal_size <= 0 ||
        (uint64_t)target->sal_offset < container->descriptor_offset ||
        (uint64_t)target->sal_offset > container->source_byte_count ||
        (uint64_t)target->sal_size >
            container->source_byte_count - (uint64_t)target->sal_offset ||
        row->row_index >= map->record_count ||
        row->table_fnv1a64 != map->table_fnv1a64 ||
        map->source_fnv1a64 != target->canonical_map_fnv1a64) {
        receipt.status = NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE;
        *out_receipt = receipt;
        return 0;
    }

    source_fnv = nexus_v1_sal_payload_fnv1a64(input->active_sal_bytes,
                                                input->active_sal_byte_count);
    if (!source_fnv || source_fnv != target->canonical_sal_fnv1a64 ||
        container->source_fnv1a64 != source_fnv) {
        receipt.status = NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE;
        *out_receipt = receipt;
        return 0;
    }
    receipt.active_source_bound = 1;

    if (!nexus_v1_sal_payload_target_matches_trace(target, trace) ||
        evidence->status != NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED ||
        evidence->level_index != target->level_index ||
        !evidence->raw_trace_bound ||
        evidence->raw_trace_fnv1a64 != trace->raw_trace_fnv1a64 ||
        evidence->raw_trace_byte_count != trace->raw_trace_byte_count ||
        !evidence->selector_dispatch_observed || !evidence->sal_read_observed ||
        !evidence->driver_output_observed || !evidence->observation_order_proven ||
        evidence->driver_dispatch_proven || evidence->sal_decode_proven ||
        evidence->playback_permitted || !evidence->blocks_real_sfx_playback ||
        evidence->fallback_visuals_permitted) {
        receipt.status = NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_TRACE;
        *out_receipt = receipt;
        return 0;
    }

    receipt.status = NEXUS_V1_SAL_PAYLOAD_CAPTURE_ADMITTED_OPAQUE;
    receipt.raw_map_selector = target->raw_map_selector;
    receipt.map_attribute = target->map_attribute;
    receipt.canonical_sal_fnv1a64 = target->canonical_sal_fnv1a64;
    receipt.canonical_map_fnv1a64 = target->canonical_map_fnv1a64;
    receipt.sal_window_offset = (uint64_t)target->sal_offset;
    receipt.sal_window_size = (uint64_t)target->sal_size;
    receipt.sal_window_fnv1a64 = nexus_v1_sal_payload_fnv1a64(
        input->active_sal_bytes + receipt.sal_window_offset,
        (size_t)receipt.sal_window_size);
    receipt.container_descriptor_offset = container->descriptor_offset;
    receipt.container_descriptor_length = container->descriptor_length;
    receipt.container_descriptor_fnv1a64 = container->descriptor_fnv1a64;
    receipt.map_table_fnv1a64 = map->table_fnv1a64;
    receipt.map_row_fnv1a64 = row->row_fnv1a64;
    receipt.map_row_index = row->row_index;
    receipt.raw_trace_fnv1a64 = trace->raw_trace_fnv1a64;
    receipt.raw_trace_byte_count = trace->raw_trace_byte_count;
    receipt.selector_dispatch_pc = trace->selector_dispatch_pc;
    receipt.sal_read_pc = trace->sal_read_pc;
    receipt.driver_output_pc = trace->driver_output_pc;
    receipt.original_saturn_observations_bound = 1;
    *out_receipt = receipt;
    return 1;
}
