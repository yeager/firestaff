#ifndef NEXUS_V1_VDP1_DGN_MATERIAL_RESOLVER_H
#define NEXUS_V1_VDP1_DGN_MATERIAL_RESOLVER_H

#include "nexus_v1_vdp1_capture_compositor.h"

/* A single canonical retail LEV*.DGN byte image. The caller owns the
 * canonical hash attestation; this resolver never treats an arbitrary DGN
 * buffer as retail merely because its envelope parses. */
typedef struct {
    const uint8_t *dgn_bytes;
    int dgn_byte_count;
    int source_hash_verified;
    int palette_slot_base;
} Nexus_V1_Vdp1DgnMaterialResolverInput;

typedef struct {
    int valid;
    int source_hash_verified;
    int structure2_envelope_verified;
    int candidate_count;
    int image_matches;
    int palette_matches;
    int unique_join;
    uint16_t structure2_image_id;
    uint16_t structure2_encoding;
    uint16_t structure2_width;
    uint16_t structure2_height;
} Nexus_V1_Vdp1DgnMaterialResolverReceipt;

/* Resolve one captured mode-1 draw to one and only one Structure2 material.
 * DGN payload bytes remain in canonical byte order; the capture compositor
 * performs the explicit Saturn word-order comparison. No face, transform,
 * culling, or camera meaning is inferred here. */
int nexus_v1_vdp1_dgn_material_resolver(
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *command, int command_size,
    const Nexus_V1_Vdp1TextureCommand *parsed,
    uint32_t command_byte_offset,
    Nexus_V1_Vdp1CaptureCompositeInput *out_input,
    void *context);

void nexus_v1_vdp1_dgn_material_resolver_receipt(
    const Nexus_V1_Vdp1DgnMaterialResolverInput *input,
    Nexus_V1_Vdp1DgnMaterialResolverReceipt *out_receipt);

#endif
