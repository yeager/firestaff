#include "nexus_v1_font256_vdp2_capture_join.h"

#include <string.h>

int nexus_v1_font256_vdp2_capture_join(
    const Nexus_V1_Font256Vdp2CaptureJoinInput *input,
    Nexus_V1_Font256Vdp2CaptureJoinReceipt *out_receipt)
{
    Nexus_V1_Font256Vdp2CaptureJoinReceipt receipt;
    uint32_t cg_offset;
    uint32_t cg_size;
    uint32_t palette_offset;
    uint32_t palette_size;

    memset(&receipt, 0, sizeof(receipt));
    receipt.semantic_admission_blocked = 1;
    if (!input || !input->capture_character_generator ||
        !input->capture_palette || !input->font256_s2d ||
        !input->decoded || !input->decoded->valid ||
        !input->source_hash_verified || input->font256_s2d_size <= 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    cg_offset = input->decoded->character_generator_offset;
    cg_size = input->decoded->character_generator_size;
    palette_offset = input->decoded->palette_offset;
    palette_size = input->decoded->palette_size;
    if (cg_size < 16U || palette_size < 16U ||
        cg_offset > (uint32_t)input->font256_s2d_size ||
        cg_size > (uint32_t)input->font256_s2d_size - cg_offset ||
        palette_offset > (uint32_t)input->font256_s2d_size ||
        palette_size > (uint32_t)input->font256_s2d_size - palette_offset ||
        input->capture_character_generator_size != (int)(cg_size - 16U) ||
        input->capture_palette_size != (int)(palette_size - 16U)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.source_hash_verified = 1;
    receipt.character_generator_tile_count =
        (int)((cg_size - 16U) / 64U);
    receipt.palette_color_count = (int)((palette_size - 16U) / 2U);
    if (receipt.character_generator_tile_count <= 0 ||
        receipt.character_generator_tile_count > 256 ||
        receipt.palette_color_count != 256 ||
        memcmp(input->capture_character_generator,
               input->font256_s2d + cg_offset + 16U, cg_size - 16U) != 0 ||
        memcmp(input->capture_palette,
               input->font256_s2d + palette_offset + 16U,
               palette_size - 16U) != 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    receipt.character_generator_span_join_verified = 1;
    receipt.palette_span_join_verified = 1;
    /* A source join does not establish page PND values, text encoding,
     * character-code-to-tile mapping, placement, or layer ownership. */
    receipt.text_code_mapping_proven = 0;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}
