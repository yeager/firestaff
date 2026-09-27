#include "nexus_v1_vdp1_dgn_material_resolver.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8U) | p[1]);
}

static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24U) | ((uint32_t)p[1] << 16U) |
        ((uint32_t)p[2] << 8U) | p[3];
}

static int swapped_words_equal(const uint8_t *capture, int capture_size,
                               const uint8_t *dgn, int dgn_size)
{
    int i;

    if (!capture || !dgn || capture_size <= 0 || capture_size != dgn_size ||
        (capture_size & 1) != 0) return 0;
    for (i = 0; i < dgn_size; i += 2) {
        if (capture[i] != dgn[i + 1] || capture[i + 1] != dgn[i]) return 0;
    }
    return 1;
}

static int bytes_all_zero(const uint8_t *bytes, int size)
{
    int i;
    if (!bytes || size <= 0) return 0;
    for (i = 0; i < size; ++i)
        if (bytes[i] != 0U) return 0;
    return 1;
}

static void clear_receipt(Nexus_V1_Vdp1DgnMaterialResolverReceipt *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

static int structure3_face_owner_count(
    const uint8_t *data, int size, uint16_t image_id)
{
    uint32_t base;
    uint32_t bytes;
    uint32_t entry_count;
    uint32_t entry;
    int owners = 0;

    if (!data || size < 0x20) return 0;
    base = (uint32_t)read_be16(data + 0x1c) * 0x800U;
    bytes = (uint32_t)read_be16(data + 0x1e) * 0x800U;
    if (base == 0U || bytes < 4U || base > (uint32_t)size ||
        bytes > (uint32_t)size - base) return 0;
    entry_count = read_be32(data + base);
    if (entry_count == 0U || entry_count > 4096U ||
        entry_count > (bytes - 4U) / 4U) return 0;
    for (entry = 0U; entry < entry_count; ++entry) {
        uint32_t entry_offset = read_be32(
            data + base + 4U + entry * 4U);
        uint32_t entry_end = entry + 1U < entry_count
            ? read_be32(data + base + 4U + (entry + 1U) * 4U) : bytes;
        uint16_t face_count;
        uint32_t face_offset;
        uint32_t face;
        if (entry_offset > entry_end || entry_end > bytes ||
            entry_end - entry_offset < 24U) return 0;
        face_count = read_be16(data + base + entry_offset + 6U);
        face_offset = read_be32(data + base + entry_offset + 16U);
        if (face_offset < entry_offset || face_offset > entry_end ||
            (uint32_t)face_count > (entry_end - face_offset) / 12U) return 0;
        for (face = 0U; face < face_count; ++face) {
            const uint8_t *row = data + base + face_offset + face * 12U;
            if (read_be16(row + 10U) == image_id) ++owners;
        }
    }
    return owners;
}

int nexus_v1_vdp1_dgn_material_resolver(
    const uint8_t *vdp1_vram, int vdp1_vram_size,
    const uint8_t *command, int command_size,
    const Nexus_V1_Vdp1TextureCommand *parsed,
    uint32_t command_byte_offset,
    Nexus_V1_Vdp1CaptureCompositeInput *out_input,
    void *context)
{
    Nexus_V1_Vdp1DgnMaterialResolverInput *input =
        (Nexus_V1_Vdp1DgnMaterialResolverInput *)context;
    const uint8_t *data;
    uint32_t useful;
    uint32_t base;
    uint32_t cursor;
    uint32_t palette_offset;
    uint16_t expected_image_id = 0;
    int terminator_found = 0;
    int image_matches = 0;
    int palette_matches = 0;
    int image_palette_matches = 0;
    int matched_image_id_verified = 0;
    const uint8_t *matched_image = NULL;
    const uint8_t *matched_palette = NULL;
    uint16_t matched_image_id = 0;
    const uint8_t *matched_joined_image = NULL;
    const uint8_t *matched_joined_palette = NULL;
    uint16_t matched_joined_image_id = 0;

    (void)command;
    (void)command_size;
    (void)command_byte_offset;
    if (out_input) memset(out_input, 0, sizeof(*out_input));
    if (!input || !input->dgn_bytes || input->dgn_byte_count < 0x20 ||
        input->source_hash_verified <= 0 || !vdp1_vram ||
        vdp1_vram_size != (int)NEXUS_V1_VDP1_VRAM_BYTES || !parsed ||
        !parsed->texture_command ||
        (parsed->colour_mode != 1U && parsed->colour_mode != 5U) ||
        !parsed->texture_source_range_valid || parsed->texture_byte_count == 0U ||
        input->palette_slot_base < 0 || input->palette_slot_base > 240) return 0;
    data = input->dgn_bytes;
    base = (uint32_t)read_be16(data + 0x14) * 0x800U;
    useful = read_be32(data + 0x18);
    if (base > (uint32_t)input->dgn_byte_count ||
        useful > (uint32_t)input->dgn_byte_count - base ||
        useful < 22U) return 0;
    /* CMDCOLR's documented <<2 result is a VDP1 word address; this buffer
     * uses byte offsets, so the conversion is <<3. */
    palette_offset = (((uint32_t)parsed->colour_control & ~UINT32_C(3)) << 3U);
    if (parsed->colour_mode == 1U &&
        palette_offset > (uint32_t)vdp1_vram_size - 32U) return 0;

    for (cursor = 0U; cursor + 20U <= useful; cursor += 20U) {
        const uint8_t *descriptor = data + base + cursor;
        uint16_t image_id = read_be16(descriptor);
        uint16_t encoding;
        uint16_t width;
        uint16_t height;
        uint32_t image_offset;
        uint32_t dgn_palette_offset;
        uint32_t image_size;
        const uint8_t *image = NULL;
        const uint8_t *palette = NULL;
        int image_match = 0;
        int palette_match = 0;
        int image_range_valid;
        int palette_range_valid;

        if (image_id == 0xffffU) {
            terminator_found = 1;
            break;
        }
        if (image_id != expected_image_id) return 0;
        ++expected_image_id;
        encoding = read_be16(descriptor + 2);
        width = read_be16(descriptor + 6);
        height = read_be16(descriptor + 8);
        image_offset = read_be32(descriptor + 12);
        dgn_palette_offset = read_be32(descriptor + 16);
        if ((parsed->colour_mode == 1U && encoding != 0x0008U) ||
            (parsed->colour_mode == 5U && encoding != 0x0028U) ||
            width == 0U || height == 0U) continue;
        image_size = parsed->colour_mode == 5U
            ? (uint32_t)width * (uint32_t)height * 2U
            : ((uint32_t)width * (uint32_t)height + 1U) / 2U;
        image_range_valid = image_offset >= cursor + 22U &&
            image_offset <= useful && image_size <= useful - image_offset;
        if (image_range_valid && image_size == parsed->texture_byte_count) {
            image = data + base + image_offset;
            image_match = swapped_words_equal(
                vdp1_vram + parsed->texture_source_byte_offset,
                (int)parsed->texture_byte_count, image, (int)image_size);
        }
        /* CMDCOLR selects a frame-local CLUT independently from CMDSRCA.
         * A DGN palette remains a valid source candidate even when its image
         * descriptor has a different size from the captured pixel span. */
        palette_range_valid = parsed->colour_mode == 1U &&
            dgn_palette_offset >= cursor + 22U &&
            dgn_palette_offset <= useful &&
            32U <= useful - dgn_palette_offset;
        if (palette_range_valid) {
            palette = data + base + dgn_palette_offset;
            palette_match = swapped_words_equal(
                vdp1_vram + palette_offset, 32, palette, 32);
        }
        if (image_match) {
            ++image_matches;
            matched_image = image;
            matched_image_id = image_id;
        }
        /* Nexus may reuse a canonical DGN palette for a different image
         * descriptor. CMDCOLR is a frame-local CLUT selection, so image and
         * palette ownership must be joined independently within the same
         * hash-verified DGN rather than forced to one descriptor. */
        if (palette_match) {
            ++palette_matches;
            matched_palette = palette;
        }
        /* Identical pixel payloads can be stored under several Structure2
         * IDs while each descriptor carries a different CLUT.  The unique
         * pair of captured image bytes and captured palette bytes is then a
         * stronger source join than requiring the image bytes to be unique
         * in isolation.  Keep the independently unique fallback below for
         * Nexus commands that reuse a canonical palette across descriptors. */
        if (image_match && palette_match) {
            ++image_palette_matches;
            matched_joined_image = image;
            matched_joined_palette = palette;
            matched_joined_image_id = image_id;
        }
    }
    if (!terminator_found || !out_input) return 0;
    if (parsed->colour_mode == 5U) {
        if (image_matches != 1 || !matched_image) return 0;
        memset(out_input, 0, sizeof(*out_input));
        out_input->dgn_direct_image = matched_image;
        out_input->dgn_direct_image_size = (int)parsed->texture_byte_count;
        out_input->dgn_direct_source_hash_verified = 1;
        return 1;
    }
    if (image_matches == 0 &&
        bytes_all_zero(vdp1_vram + parsed->texture_source_byte_offset,
                       (int)parsed->texture_byte_count) &&
        (parsed->draw_mode & UINT16_C(0x0040)) == 0U) {
        /* Four frame-760 draws are identical all-zero mode-1 clear sprites.
         * They have no DGN image owner; retain them as capture evidence only. */
        memset(out_input, 0, sizeof(*out_input));
        out_input->transparent_capture_noop_verified = 1;
        return 1;
    }
    if (image_palette_matches == 1) {
        matched_image = matched_joined_image;
        matched_palette = matched_joined_palette;
        matched_image_id = matched_joined_image_id;
        matched_image_id_verified = 1;
    } else if (image_palette_matches == 0 && image_matches > 0 &&
               palette_matches == 1 && matched_image && matched_palette) {
        /* Several Structure2 records may carry byte-identical pixels under
         * different image IDs. If there is no unique image+palette record
         * pair but the captured palette itself has one unique owner, the
         * rendered pixels are still byte-exact and the palette is still
         * independently source-bound. Do not claim a face owner unless the
         * image ID is unique. */
        if (image_matches == 1) matched_image_id_verified = 1;
    } else {
        return 0;
    }
    memset(out_input, 0, sizeof(*out_input));
    out_input->dgn_image = matched_image;
    out_input->dgn_image_size = (int)parsed->texture_byte_count;
    out_input->dgn_palette = matched_palette;
    out_input->dgn_palette_size = 32;
    out_input->dgn_source_hash_verified = 1;
    out_input->dgn_structure3_face_owner_count = matched_image_id_verified
        ? structure3_face_owner_count(data, input->dgn_byte_count,
                                      matched_image_id)
        : 0;
    out_input->dgn_structure3_face_owner_verified =
        out_input->dgn_structure3_face_owner_count > 0;
    out_input->palette_slot_base = input->palette_slot_base;
    return 1;
}

void nexus_v1_vdp1_dgn_material_resolver_receipt(
    const Nexus_V1_Vdp1DgnMaterialResolverInput *input,
    Nexus_V1_Vdp1DgnMaterialResolverReceipt *out_receipt)
{
    Nexus_V1_Vdp1DgnMaterialResolverReceipt receipt;
    uint32_t base;
    uint32_t useful;
    uint32_t cursor;

    clear_receipt(&receipt);
    if (!input || !input->dgn_bytes || input->dgn_byte_count < 0x20 ||
        !input->source_hash_verified) {
        if (out_receipt) *out_receipt = receipt;
        return;
    }
    base = (uint32_t)read_be16(input->dgn_bytes + 0x14) * 0x800U;
    useful = read_be32(input->dgn_bytes + 0x18);
    if (base > (uint32_t)input->dgn_byte_count ||
        useful > (uint32_t)input->dgn_byte_count - base) {
        if (out_receipt) *out_receipt = receipt;
        return;
    }
    receipt.source_hash_verified = 1;
    for (cursor = 0U; cursor + 20U <= useful; cursor += 20U) {
        if (read_be16(input->dgn_bytes + base + cursor) == 0xffffU) {
            receipt.structure2_envelope_verified = 1;
            break;
        }
        ++receipt.candidate_count;
    }
    receipt.valid = receipt.structure2_envelope_verified;
    if (out_receipt) *out_receipt = receipt;
}
