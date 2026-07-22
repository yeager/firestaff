#ifndef NEXUS_V1_SAL_PAYLOAD_CAPTURE_ADMISSION_H
#define NEXUS_V1_SAL_PAYLOAD_CAPTURE_ADMISSION_H

#include "nexus_v1_engine.h"
#include "nexus_v1_sal_container_provenance.h"
#include "nexus_v1_sndlev_map_provenance.h"

#include <stddef.h>
#include <stdint.h>

/* This is an ownership boundary for one already requested SAL window. It is
 * intentionally not a sample decoder, a driver ABI, or a playback route. */
typedef enum {
    NEXUS_V1_SAL_PAYLOAD_CAPTURE_MISSING = 0,
    NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_SOURCE = 1,
    NEXUS_V1_SAL_PAYLOAD_CAPTURE_BLOCKED_TRACE = 2,
    NEXUS_V1_SAL_PAYLOAD_CAPTURE_ADMITTED_OPAQUE = 3
} Nexus_V1_SalPayloadCaptureStatus;

typedef struct {
    const Nexus_V1_LevelSoundCaptureTargetReceipt *target;
    const Nexus_V1_LevelSoundTraceAdmissionReceipt *trace;
    const Nexus_V1_SalDispatchEvidenceReceipt *evidence;
    const Nexus_V1_SalContainerProvenanceReceipt *container;
    const Nexus_V1_SndlevMapProvenanceReceipt *map;
    const Nexus_V1_SndlevMapRowProvenanceReceipt *map_row;
    const uint8_t *active_sal_bytes;
    size_t active_sal_byte_count;
} Nexus_V1_SalPayloadCaptureAdmissionInput;

typedef struct {
    Nexus_V1_SalPayloadCaptureStatus status;
    int level_index;
    int raw_map_selector;
    int map_attribute;
    uint64_t canonical_sal_fnv1a64;
    uint64_t canonical_map_fnv1a64;
    uint64_t sal_window_offset;
    uint64_t sal_window_size;
    uint64_t sal_window_fnv1a64;
    uint64_t container_descriptor_offset;
    uint64_t container_descriptor_length;
    uint64_t container_descriptor_fnv1a64;
    uint64_t map_table_fnv1a64;
    uint64_t map_row_fnv1a64;
    uint32_t map_row_index;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    uint32_t selector_dispatch_pc;
    uint32_t sal_read_pc;
    uint32_t driver_output_pc;
    int active_source_bound;
    int original_saturn_observations_bound;
    int payload_decode_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
    int fallback_audio_permitted;
} Nexus_V1_SalPayloadCaptureAdmissionReceipt;

/* Revalidates every source and capture boundary and copies only identities,
 * offsets, and hashes into the output receipt. Source bytes are never kept. */
int nexus_v1_sal_payload_capture_admit(
    const Nexus_V1_SalPayloadCaptureAdmissionInput *input,
    Nexus_V1_SalPayloadCaptureAdmissionReceipt *out_receipt);

#endif
