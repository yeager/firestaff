#ifndef NEXUS_V1_FONT256_S2D_FIRST_SECTION_CAPTURE_H
#define NEXUS_V1_FONT256_S2D_FIRST_SECTION_CAPTURE_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_font256_s2d_section_witness.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The iterator deliberately emits one span only: the complete bounded first
 * section. No smaller subspan has record, glyph, palette, or pixel meaning. */
typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_Font256S2DRawSectionSpan;

typedef struct {
    int valid;
    int capture_required;
    int glyph_layout_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t section_table_index;
    Nexus_V1_Font256S2DRawSectionSpan span;
} Nexus_V1_Font256S2DFirstSectionCaptureReceipt;

typedef struct {
    Nexus_V1_Font256S2DFirstSectionCaptureReceipt receipt;
    int emitted;
} Nexus_V1_Font256S2DFirstSectionSpanIterator;

int nexus_v1_font256_s2d_first_section_capture_prepare(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DSectionWitnessReceipt *witness,
    Nexus_V1_Font256S2DFirstSectionCaptureReceipt *out_receipt);

int nexus_v1_font256_s2d_first_section_span_iterator_init(
    Nexus_V1_Font256S2DFirstSectionSpanIterator *iterator,
    const Nexus_V1_Font256S2DFirstSectionCaptureReceipt *receipt);

/* Returns 1 for the one complete raw section span, 0 at end, -1 on invalid
 * arguments/receipt. The iterator never creates inferred subspans. */
int nexus_v1_font256_s2d_first_section_span_iterator_next(
    Nexus_V1_Font256S2DFirstSectionSpanIterator *iterator,
    Nexus_V1_Font256S2DRawSectionSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
