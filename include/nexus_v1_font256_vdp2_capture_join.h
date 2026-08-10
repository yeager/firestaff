#ifndef NEXUS_V1_FONT256_VDP2_CAPTURE_JOIN_H
#define NEXUS_V1_FONT256_VDP2_CAPTURE_JOIN_H

#include "nexus_v1_font_s2d.h"

/* Exact source-domain join for the VDP2 Page, character-generator and palette
 * spans. This proves source ownership only; page-entry meaning and text-code
 * mapping remain separate Saturn-runtime facts. */
typedef struct {
    const uint8_t *capture_page;
    int capture_page_size;
    const uint8_t *capture_character_generator;
    int capture_character_generator_size;
    const uint8_t *capture_palette;
    int capture_palette_size;
    const uint8_t *font256_s2d;
    int font256_s2d_size;
    const Nexus_V1_FontS2dDecodeResult *decoded;
    int source_hash_verified;
} Nexus_V1_Font256Vdp2CaptureJoinInput;

typedef struct {
    int valid;
    int source_hash_verified;
    int page_span_join_verified;
    int character_generator_span_join_verified;
    int palette_span_join_verified;
    int character_generator_tile_count;
    int palette_color_count;
    int text_code_mapping_proven;
    int semantic_admission_blocked;
} Nexus_V1_Font256Vdp2CaptureJoinReceipt;

int nexus_v1_font256_vdp2_capture_join(
    const Nexus_V1_Font256Vdp2CaptureJoinInput *input,
    Nexus_V1_Font256Vdp2CaptureJoinReceipt *out_receipt);

#endif
