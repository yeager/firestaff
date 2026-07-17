#ifndef NEXUS_V1_OWNER_MATERIAL_CAPTURE_ARTIFACT_PREFLIGHT_H
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_ARTIFACT_PREFLIGHT_H

#include "nexus_v1_owner_material_capture_campaign.h"

#include <stddef.h>
#include <stdint.h>

/* Rechecks one external NXS1OMC1 artifact against one admitted NXS1OMC2
 * campaign row. The capture and trace remain opaque and are never retained. */
typedef struct {
    const Nexus_V1_OwnerMaterialCaptureCampaignReceipt *campaign;
    uint32_t witness_index;
    const Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *target;
    const uint8_t *capture_bytes;
    size_t capture_byte_count;
    const uint8_t *raw_trace;
    size_t raw_trace_byte_count;
    const Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *trace;
} Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput;

typedef struct {
    int valid;
    uint32_t witness_index;
    uint64_t capture_fnv1a64;
    uint64_t raw_trace_fnv1a64;
    uint64_t raw_trace_byte_count;
    int campaign_row_bound;
    int artifact_bound;
    int raw_trace_bound;
    int engine_trace_bound;
    int opaque_evidence_only;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt;

int nexus_v1_owner_material_capture_artifact_preflight(
    const Nexus_V1_OwnerMaterialCaptureArtifactPreflightInput *input,
    Nexus_V1_OwnerMaterialCaptureArtifactPreflightReceipt *out_receipt);

#endif
