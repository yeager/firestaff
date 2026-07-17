#ifndef NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_H
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_H

#include "nexus_v1_owner_material_capture_admission.h"

#include <stddef.h>
#include <stdint.h>

/* A bounded aggregation of already imported NXS1OMC1 evidence. It records
 * capture coverage across distinct dungeon routes only; it retains neither
 * payload bytes nor any mesh, texture, palette, VDP1, decoder, or draw claim. */
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES 16U
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MIN_WITNESSES 2U

typedef struct {
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target;
    const Nexus_V1_OwnerMaterialCaptureAdmissionReceipt *capture;
    const Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *trace;
} Nexus_V1_OwnerMaterialCaptureCampaignWitness;

typedef struct {
    const Nexus_V1_OwnerMaterialCaptureCampaignWitness *witnesses;
    size_t witness_count;
} Nexus_V1_OwnerMaterialCaptureCampaignInput;

typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t source_fnv1a64;
    uint64_t descriptor_fnv1a64;
    uint64_t face_row_fnv1a64;
    uint64_t capture_fnv1a64;
    uint64_t raw_trace_fnv1a64;
    uint64_t raw_trace_byte_count;
    int opaque_original_capture_covered;
} Nexus_V1_OwnerMaterialCaptureCampaignCoverage;

typedef struct {
    int valid;
    uint32_t witness_count;
    Nexus_V1_OwnerMaterialCaptureCampaignCoverage
        witnesses[NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES];
    int imported_original_saturn_captures_bound;
    int opaque_evidence_only;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_OwnerMaterialCaptureCampaignReceipt;

int nexus_v1_owner_material_capture_campaign_admit(
    const Nexus_V1_OwnerMaterialCaptureCampaignInput *input,
    Nexus_V1_OwnerMaterialCaptureCampaignReceipt *out_receipt);

#endif
