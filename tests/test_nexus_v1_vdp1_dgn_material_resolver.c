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

static uint8_t *read_file(const char *path, long *size)
{
    FILE *file = fopen(path, "rb");
    uint8_t *data;
    if (!file || fseek(file, 0, SEEK_END) || (*size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET)) { if (file) fclose(file); return NULL; }
    data = malloc((size_t)*size);
    if (!data || fread(data, 1, (size_t)*size, file) != (size_t)*size) {
        free(data); fclose(file); return NULL;
    }
    fclose(file);
    return data;
}

static int test_retail_item_capture(const char *capture_path, const char *item_path)
{
    long capture_size, item_size;
    uint8_t *capture = read_file(capture_path, &capture_size);
    uint8_t *item = read_file(item_path, &item_size);
    Nexus_V1_ItemIbsBank *bank = calloc(1, sizeof(*bank));
    Nexus_V1_SaturnRuntimeCaptureFrameReceipt frame;
    Nexus_V1_Vdp1TextureCommand command;
    Nexus_V1_Vdp1CaptureCompositeInput resolved;
    Nexus_V1_Vdp1ItemMaterialResolverInput context = {0};
    int ok = capture && item && bank &&
        nexus_v1_saturn_runtime_capture_frame(capture, (size_t)capture_size, 0, &frame) &&
        nexus_v1_item_ibs_parse_verified(item, (int)item_size, 1, bank) == 0 &&
        nexus_v1_vdp1_texture_command_parse(frame.vdp1_vram + 0xf00, 32, &command) == 0;
    if (ok) {
        context.item_ibs = item; context.item_ibs_size = (int)item_size;
        context.item_ibs_hash_verified = 1; context.bank = bank;
        ok = nexus_v1_vdp1_item_material_resolver(frame.vdp1_vram,
            NEXUS_V1_VDP1_VRAM_BYTES, frame.vdp1_vram + 0xf00, 32,
            &command, 0xf00, &resolved, &context) &&
            resolved.item_image == item + 0x123dc &&
            resolved.item_palette == item + 0xb098 &&
            resolved.item_source_hash_verified && !resolved.dgn_source_hash_verified;
        context.item_ibs_hash_verified = 0;
        ok = ok && !nexus_v1_vdp1_item_material_resolver(frame.vdp1_vram,
            NEXUS_V1_VDP1_VRAM_BYTES, frame.vdp1_vram + 0xf00, 32,
            &command, 0xf00, &resolved, &context);
    }
    free(capture); free(item); free(bank);
    return ok;
}

int main(void)
{
    uint8_t *vram = (uint8_t *)calloc(1, NEXUS_V1_VDP1_VRAM_BYTES);
    uint8_t *dgn = (uint8_t *)calloc(1, 0x1800U);
    uint8_t *dgn_direct = (uint8_t *)calloc(1, 0x900U);
    uint8_t command[NEXUS_V1_VDP1_COMMAND_BYTES] = {0};
    Nexus_V1_Vdp1TextureCommand parsed;
    Nexus_V1_Vdp1DgnMaterialResolverInput context;
    Nexus_V1_Vdp1CaptureCompositeInput resolved;
    Nexus_V1_Vdp1DgnMaterialResolverReceipt envelope;
    int i;

    if (getenv("FIRESTAFF_NEXUS_ITEM_CAPTURE") && getenv("FIRESTAFF_NEXUS_ITEM_IBS") &&
        !test_retail_item_capture(getenv("FIRESTAFF_NEXUS_ITEM_CAPTURE"),
                                  getenv("FIRESTAFF_NEXUS_ITEM_IBS"))) {
        fprintf(stderr, "FAIL: retail ITEM.IBS capture material resolution\n");
        return 1;
    }

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
    be16(dgn + 0x1c, 2U);
    be16(dgn + 0x1e, 1U);
    be32(dgn + 0x1000, 1U);
    be32(dgn + 0x1004, 8U);
    be16(dgn + 0x1000 + 8U + 6U, 1U);
    be32(dgn + 0x1000 + 8U + 16U, 32U);
    be16(dgn + 0x1000 + 32U + 10U, 0U);
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
    le16(command + 10, 0x0204U);
    if (nexus_v1_vdp1_texture_command_parse(
            command, sizeof(command), &parsed) != 0 ||
        parsed.colour_mode != 1U || parsed.texture_byte_count != 32U) {
        fprintf(stderr, "FAIL: fixture command parse\n");
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    memset(&context, 0, sizeof(context));
    context.dgn_bytes = dgn;
    context.dgn_byte_count = 0x1800;
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
        !resolved.dgn_source_hash_verified || resolved.palette_slot_base != 16 ||
        resolved.dgn_structure3_face_owner_count != 1 ||
        !resolved.dgn_structure3_face_owner_verified) {
        fprintf(stderr, "FAIL: unique source/CLUT resolver join src=%u bytes=%u colr=%04x range=%d\n",
                parsed.texture_source_byte_offset, parsed.texture_byte_count,
                parsed.colour_control, parsed.texture_source_range_valid);
        free(vram); free(dgn); free(dgn_direct); return 1;
    }
    /* Identical all-zero bytes in a DGN image are not source ownership. */
    memset(vram + 0x100, 0, 32);
    memset(dgn + 0x800 + 22, 0, 32);
    if (!nexus_v1_vdp1_dgn_material_resolver(
            vram, NEXUS_V1_VDP1_VRAM_BYTES, command, sizeof(command), &parsed,
            0x300U, &resolved, &context) ||
        !resolved.transparent_capture_noop_verified ||
        resolved.dgn_source_hash_verified || resolved.dgn_image != NULL) {
        fprintf(stderr, "FAIL: zero texture falsely attributed to DGN\n");
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
    le16(command + 10, 0x0102U);
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
