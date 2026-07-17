#include "nexus_v1_font256_s2d_first_section_capture.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

int nexus_v1_font256_s2d_first_section_capture_prepare(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DSectionWitnessReceipt *witness,
    Nexus_V1_Font256S2DFirstSectionCaptureReceipt *out_receipt)
{
    Nexus_V1_Font256S2DFirstSectionCaptureReceipt receipt;
    uint64_t source_fnv;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    if (!source_bytes || !witness || !witness->valid ||
        !witness->source_admission_bound || !witness->selected_section_bound ||
        !witness->preamble_capture_required || witness->glyph_layout_proven ||
        witness->palette_proven || witness->pixel_decode_permitted ||
        witness->draw_permitted ||
        witness->section_table_index != NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX ||
        witness->section_offset != 0x0120U || witness->section_length != 0x2010U ||
        witness->preamble_offset != witness->section_offset ||
        witness->preamble_length != NEXUS_V1_FONT256_S2D_FIRST_PREAMBLE_BYTES ||
        witness->section_offset > source_size ||
        witness->section_length > source_size - witness->section_offset) {
        *out_receipt = receipt;
        return 0;
    }
    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != witness->source_fnv1a64 ||
        fnv1a64(source_bytes + witness->section_offset, witness->section_length) !=
            witness->section_fnv1a64 ||
        fnv1a64(source_bytes + witness->preamble_offset, witness->preamble_length) !=
            witness->preamble_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.source_fnv1a64 = source_fnv;
    receipt.section_table_index = witness->section_table_index;
    receipt.span.source_offset = witness->section_offset;
    receipt.span.source_length = witness->section_length;
    receipt.span.source_fnv1a64 = witness->section_fnv1a64;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_font256_s2d_first_section_span_iterator_init(
    Nexus_V1_Font256S2DFirstSectionSpanIterator *iterator,
    const Nexus_V1_Font256S2DFirstSectionCaptureReceipt *receipt)
{
    if (!iterator || !receipt || !receipt->valid || !receipt->capture_required ||
        receipt->glyph_layout_proven || receipt->palette_proven ||
        receipt->pixel_decode_permitted || receipt->draw_permitted ||
        !receipt->span.source_length || !receipt->span.source_fnv1a64) return -1;
    memset(iterator, 0, sizeof(*iterator));
    iterator->receipt = *receipt;
    return 0;
}

int nexus_v1_font256_s2d_first_section_span_iterator_next(
    Nexus_V1_Font256S2DFirstSectionSpanIterator *iterator,
    Nexus_V1_Font256S2DRawSectionSpan *out_span)
{
    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted) return 0;
    *out_span = iterator->receipt.span;
    iterator->emitted = 1;
    return 1;
}
