#include "nexus_v1_owner_material_capture_campaign_artifact.h"

#include <string.h>

static uint32_t artifact_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24U) | ((uint32_t)bytes[1] << 16U) |
        ((uint32_t)bytes[2] << 8U) | bytes[3];
}

static uint64_t artifact_be64(const uint8_t *bytes)
{
    uint64_t value = 0U;
    size_t index;

    for (index = 0U; index < 8U; ++index) value = (value << 8U) | bytes[index];
    return value;
}

static uint64_t artifact_fnv1a64(const uint8_t *bytes, size_t byte_count)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < byte_count; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_owner_material_capture_campaign_artifact_admit(
    const Nexus_V1_OwnerMaterialCaptureCampaignReceipt *campaign,
    const uint8_t *artifact_bytes, size_t artifact_byte_count,
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt *out_receipt)
{
    Nexus_V1_OwnerMaterialCaptureCampaignArtifactReceipt receipt;
    uint32_t row_count;
    size_t rows_byte_count;
    size_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.no_draw_only = 1;
    receipt.blocks_real_dgn_mesh_render = 1;
    if (!campaign || !campaign->valid ||
        !campaign->imported_original_saturn_captures_bound ||
        !campaign->opaque_evidence_only || !campaign->no_draw_only ||
        campaign->fallback_visuals_permitted ||
        !campaign->blocks_real_dgn_mesh_render || !artifact_bytes ||
        artifact_byte_count <
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES ||
        memcmp(artifact_bytes,
               NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_MAGIC, 8U) != 0 ||
        artifact_be32(artifact_bytes + 8U) !=
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_VERSION ||
        artifact_be32(artifact_bytes + 12U) !=
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES ||
        artifact_be32(artifact_bytes + 20U) !=
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    row_count = artifact_be32(artifact_bytes + 16U);
    if (row_count != campaign->witness_count ||
        row_count < NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MIN_WITNESSES ||
        row_count > NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_MAX_WITNESSES ||
        (size_t)row_count > (artifact_byte_count -
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES) /
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    rows_byte_count = (size_t)row_count *
        NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES;
    receipt.rows_bounds_bound = artifact_byte_count ==
        NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES +
        rows_byte_count;
    receipt.rows_hash_bound = receipt.rows_bounds_bound &&
        artifact_be64(artifact_bytes + 24U) == artifact_fnv1a64(
            artifact_bytes + NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES,
            rows_byte_count);
    if (!receipt.rows_hash_bound) {
        *out_receipt = receipt;
        return 0;
    }
    for (index = 0U; index < row_count; ++index) {
        const uint8_t *row = artifact_bytes +
            NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_HEADER_BYTES +
            index * NEXUS_V1_OWNER_MATERIAL_CAPTURE_CAMPAIGN_ARTIFACT_ROW_BYTES;
        const Nexus_V1_OwnerMaterialCaptureCampaignCoverage *coverage =
            &campaign->witnesses[index];

        if (!coverage->valid || !coverage->opaque_original_capture_covered ||
            artifact_be64(row) != coverage->level_index ||
            artifact_be64(row + 8U) != coverage->source_fnv1a64 ||
            artifact_be64(row + 16U) != coverage->descriptor_fnv1a64 ||
            artifact_be64(row + 24U) != coverage->face_row_fnv1a64 ||
            artifact_be64(row + 32U) != coverage->capture_fnv1a64 ||
            artifact_be64(row + 40U) != coverage->raw_trace_fnv1a64 ||
            artifact_be64(row + 48U) != coverage->raw_trace_byte_count) {
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.valid = 1;
    receipt.artifact_fnv1a64 = artifact_fnv1a64(artifact_bytes, artifact_byte_count);
    receipt.artifact_byte_count = artifact_byte_count;
    receipt.witness_count = row_count;
    receipt.rows_fnv1a64 = artifact_be64(artifact_bytes + 24U);
    receipt.campaign_bound = 1;
    receipt.opaque_evidence_only = 1;
    *out_receipt = receipt;
    return 1;
}
