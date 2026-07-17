#ifndef NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_H
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_H

#include "nexus_v1_owner_material_capture_campaign.h"

#include <stddef.h>
#include <stdint.h>

/* Fixed external index for an already admitted NXS1OMC1 campaign. Rows carry
 * identities only, never capture payload or raw-trace bytes. */
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_MAGIC "NXS1OMC2"
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_VERSION 1U
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES 32U
#define NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES 56U

typedef struct {
    int valid;
    uint64_t artifact_fnv1a64;
    uint64_t artifact_byte_count;
    uint32_t witness_count;
    uint64_t rows_fnv1a64;
    int campaign_bound;
    int rows_bounds_bound;
    int rows_hash_bound;
    int opaque_evidence_only;
    int owner_mapping_proven;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt;

int nexus_v1_owner_material_capture_campaign_artifact_admit(
    const Nexus_V1_OwnerMaterialCaptureCampaignReceipt *campaign,
    const uint8_t *artifact_bytes, size_t artifact_byte_count,
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt *out_receipt);

#endif
