#include "nexus_v1_vdp1_dgn_material_resolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void be16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)(value >> 8U);
    p[1] = (uint8_t)value;
}

static void be32(uint8_t *p, unsigned long value)
{
    p[0] = (uint8_t)(value >> 24U);
    p[1] = (uint8_t)(value >> 16U);
    p[2] = (uint8_t)(value >> 8U);
    p[3] = (uint8_t)value;
}

static void le16(uint8_t *p, unsigned int value)
{
    p[0] = (uint8_t)value;
    p[1] = (uint8_t)(value >> 8U);
}

int main(void)
{
    uint8_t *vram = (uint8_t *)calloc(1, NEXUS_V1_VDP1_VRAM_BYTES);
    uint8_t *dgn = (uint8_t *)calloc(1, 0x900U);
    uint8_t *dgn_direct = (uint8_t *)calloc(1, 0x900U);
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
    Nexus_V1_Vdp1TextureCommand parsed;
    Nexus_V1_Vdp1DgnMaterialResolverInput context;
    Nexus_V1_Vdp1CaptureCompositeInput resolved;
    Nexus_V1_Vdp1DgnMaterialResolverReceipt envelope;
    int i;

    if (!vram || !dgn || !dgn_direct) return 1;
    /* One canonical Structure2 descriptor at DGN block 1, then FFFF and
     * opaque payload. The payload is intentionally only a fixture; the
     * source-hash flag is the caller's retail attestation. */
    be16(dgn + 0x14, 1U);
    be32(dgn + 0x18, 86U);
    be16(dgn + 0x800, 0U);
    be16(dgn + 0x802, 0x0008U);
    be16(dgn + 0x806, 16U);
    be16(dgn + 0x808, 4U);
    be32(dgn + 0x80c, 22U);
    be32(dgn + 0x810, 54U);
    be16(dgn + 0x814, 0xffffU);
    for (i = 0; i < 32; ++i) {
        dgn[0x800 + 22 + i] = 0x12U;
        dgn[0x800 + 54 + i] = 0x80U;
        vram[0x100 + i] = 0x12U;
        vram[0x193c0 + i] = 0x80U;
    }
    le16(command + 0, 0x1002U);
    le16(command + 4, 1U << 3U);
    le16(command + 6, 0x3278U);
    le16(command + 8, 0x0020U);
    le16(command + 10, 0x0402U);
    if (nexus_v1_vdp1_texture_command_parse(
            command, sizeof(command), &parsed) != 0 ||
        parsed.colour_mode != 1U || parsed.texture_byte_count != 32U) {
        fprintf(stderr, "FAIL: fixture command parse\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    memset(&context, 0, sizeof(context));
    context.dgn_bytes = dgn;
    context.dgn_byte_count = 0x900;
    context.source_hash_verified = 1;
    context.palette_slot_base = 16;
    memset(&envelope, 0, sizeof(envelope));
    nexus_v1_vdp1_dgn_material_resolver_receipt(&context, &envelope);
    if (!envelope.valid || !envelope.structure2_envelope_verified ||
        envelope.candidate_count != 1) {
        fprintf(stderr, "FAIL: Structure2 envelope receipt\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    if (!nexus_v1_vdp1_dgn_material_resolver(
            vram, NEXUS_V1_VDP1_VRAM_BYTES, command, sizeof(command), &parsed,
            0x300U, &resolved, &context) ||
        resolved.dgn_image_size != 32 || resolved.dgn_palette_size != 32 ||
        !resolved.dgn_source_hash_verified || resolved.palette_slot_base != 16) {
        fprintf(stderr, "FAIL: unique source/CLUT resolver join src=%u bytes=%u colr=%04x range=%d\n",
                parsed.texture_source_byte_offset, parsed.texture_byte_count,
                parsed.colour_control, parsed.texture_source_range_valid);
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    /* Structure2 encoding 28h is the canonical direct-colour DGN owner.
     * The capture source is Saturn little-endian; the DGN image remains
     * big-endian and the resolver returns the exact owner span. */
    be16(dgn_direct + 0x14, 1U);
    be32(dgn_direct + 0x18, 54U);
    be16(dgn_direct + 0x800, 0U);
    be16(dgn_direct + 0x802, 0x0028U);
    be16(dgn_direct + 0x806, 8U);
    be16(dgn_direct + 0x808, 2U);
    be32(dgn_direct + 0x80c, 22U);
    be32(dgn_direct + 0x810, 0U);
    be16(dgn_direct + 0x814, 0xffffU);
    for (i = 0; i < 32; i += 2) {
        vram[0x200 + i] = (uint8_t)(0x20U + i);
        vram[0x201 + i] = (uint8_t)(0x80U + i);
        dgn_direct[0x800 + 22 + i] = vram[0x201 + i];
        dgn_direct[0x800 + 22 + i + 1] = vram[0x200 + i];
    }
    le16(command + 4, 5U << 3U);
    le16(command + 8, 0x0040U);
    le16(command + 10, 0x0201U);
    if (nexus_v1_vdp1_texture_command_parse(
            command, sizeof(command), &parsed) != 0 ||
        parsed.colour_mode != 5U || parsed.texture_byte_count != 32U) {
        fprintf(stderr, "FAIL: direct-colour fixture command parse\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    memset(&context, 0, sizeof(context));
    context.dgn_bytes = dgn_direct;
    context.dgn_byte_count = 0x900;
    context.source_hash_verified = 1;
    memset(&resolved, 0, sizeof(resolved));
    if (!nexus_v1_vdp1_dgn_material_resolver(
            vram, NEXUS_V1_VDP1_VRAM_BYTES, command, sizeof(command), &parsed,
            0x300U, &resolved, &context) ||
        resolved.dgn_direct_image != dgn_direct + 0x800 + 22 ||
        resolved.dgn_direct_image_size != 32 ||
        !resolved.dgn_direct_source_hash_verified) {
        fprintf(stderr, "FAIL: direct-colour Structure2 28h source join\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    context.source_hash_verified = 0;
    if (nexus_v1_vdp1_dgn_material_resolver(
            vram, NEXUS_V1_VDP1_VRAM_BYTES, command, sizeof(command), &parsed,
            0x300U, &resolved, &context)) {
        fprintf(stderr, "FAIL: unverified DGN source was admitted\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    free(vram);
    free(dgn);
    free(dgn_direct);
    puts("test_nexus_v1_vdp1_dgn_material_resolver: PASS");
    return 0;
}
