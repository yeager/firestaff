#ifndef NEXUS_V1_SATURN_TILE_TRANSFORM_H
#define NEXUS_V1_SATURN_TILE_TRANSFORM_H

#include <stddef.h>
#include <stdint.h>

/*
 * Saturn runtime evidence: SH-2 0x060132e0 calls the 0x060133f8 worker
 * with 18 rows, input stride 0x80 and output stride 0x1c0.  The observed
 * 0x060135f8 inner expander calls 0x060136c4 for eight rows of 48 pixels.
 * This helper mirrors that observed operation only; it does not infer a
 * CLUT owner or a VDP1/VDP2 consumer.
 */
int nexus_v1_saturn_expand_tile_8x48(
    const uint8_t *input,
    size_t input_size,
    uint16_t *output,
    size_t output_words,
    size_t output_stride_words,
    const uint16_t table_words[32],
    int16_t coefficient_r9,
    int16_t coefficient_r10);

#endif
