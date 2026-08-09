#include "nexus_v1_font256_vdp2_capture_join.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    uint8_t *s2d = (uint8_t *)calloc(1U, 0x2000U);
    uint8_t cg[4 * 64];
    uint8_t palette[256 * 2];
    Nexus_V1_FontS2dDecodeResult decoded;
    Nexus_V1_Font256Vdp2CaptureJoinInput input;
    Nexus_V1_Font256Vdp2CaptureJoinReceipt receipt;
    int i;

    if (!s2d) return 1;
    memset(&decoded, 0, sizeof(decoded));
    decoded.valid = 1;
    decoded.character_generator_offset = 0x100U;
    decoded.character_generator_size = 16U + sizeof(cg);
    decoded.palette_offset = 0x500U;
    decoded.palette_size = 16U + sizeof(palette);
    for (i = 0; i < (int)sizeof(cg); ++i) {
        cg[i] = (uint8_t)(i + 1);
        s2d[0x100U + 16U + (size_t)i] = cg[i];
    }
    for (i = 0; i < (int)sizeof(palette); ++i) {
        palette[i] = (uint8_t)(0x80U + i);
        s2d[0x500U + 16U + (size_t)i] = palette[i];
    }
    memset(&input, 0, sizeof(input));
    input.capture_character_generator = cg;
    input.capture_character_generator_size = sizeof(cg);
    input.capture_palette = palette;
    input.capture_palette_size = sizeof(palette);
    input.font256_s2d = s2d;
    input.font256_s2d_size = 0x2000;
    input.decoded = &decoded;
    input.source_hash_verified = 1;
    if (!nexus_v1_font256_vdp2_capture_join(&input, &receipt) ||
        !receipt.valid || !receipt.character_generator_span_join_verified ||
        !receipt.palette_span_join_verified || receipt.text_code_mapping_proven ||
        !receipt.semantic_admission_blocked ||
        receipt.character_generator_tile_count != 4 ||
        receipt.palette_color_count != 256) {
        fprintf(stderr, "FAIL: FONT256 VDP2 source join\n");
        free(s2d);
        return 1;
    }
    palette[0] ^= 1U;
    if (nexus_v1_font256_vdp2_capture_join(&input, &receipt)) {
        fprintf(stderr, "FAIL: altered FONT256 palette was admitted\n");
        free(s2d);
        return 1;
    }
    free(s2d);
    puts("test_nexus_v1_font256_vdp2_capture_join: PASS");
    return 0;
}
