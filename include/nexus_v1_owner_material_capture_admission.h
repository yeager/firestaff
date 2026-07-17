#ifndef NEXUS_V1_OWNER_MATERIAL_CAPTURE_ADMISSION_H
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_ADMISSION_H

#include "nexus_v1_engine.h"

#include <stddef.h>
#include <stdint.h>

/* Fixed external envelope for one already source-bound Structure1F/1A owner,
 * Structure3 face, and Structure2 candidate target. Its payload and trace are
 * opaque evidence only; this header never establishes owner mapping, mesh,
 * texture, palette, VDP1, decoder, or draw semantics. */
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_MAGIC "NXS1OMC1"
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_VERSION 1U
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_HEADER_BYTES 160U

typedef struct {
    int valid;
    uint64_t capture_fnv1a64;
    uint64_t capture_byte_count;
    uint64_t target_source_fnv1a64;
    uint64_t target_descriptor_fnv1a64;
    uint64_t target_face_row_fnv1a64;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    uint64_t raw_trace_fnv1a64;
    uint64_t raw_trace_byte_count;
    int target_bound;
    int owner_bound;
    int face_bound;
    int descriptor_bound;
    int candidates_bound;
    int payload_bounds_bound;
    int payload_hash_bound;
    int trace_witness_bound;
    int payload_opaque;
    int original_saturn_capture_verified;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_OwnerMaterialCaptureAdmissionReceipt;

int nexus_v1_owner_material_capture_admit(
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target,
    const uint8_t *capture_bytes, size_t capture_byte_count,
    Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *out_receipt);

typedef struct {
    int valid;
    int independent_captures_bound;
    int target_bound;
    int opaque_evidence_only;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_OwnerMaterialCaptureAdjudicationReceipt;

int nexus_v1_owner_material_capture_adjudicate(
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *first,
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *second,
    Nexus_V1_OwnerMaterialCaptureAdjudicationReceipt *out_receipt);

#endif
