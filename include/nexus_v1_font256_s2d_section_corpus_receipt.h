#ifndef NEXUS_V1_FONT256_S2D_SECTION_CORPUS_RECEIPT_H
#define NEXUS_V1_FONT256_S2D_SECTION_CORPUS_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_font256_s2d_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The canonical FONT256.S2D admission binds exactly four populated SCR
 * sections (table indices 0, 2, 4, 6). Only the first section previously had
 * a witness/span receipt. This module extends the same capture-required,
 * no-semantics treatment to every populated section and to their observed
 * contiguous chain. It records raw composition measurements only; no byte or
 * word is assigned text, glyph, palette, record, or pixel meaning, and no
 * draw route is permitted. */
#define NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT 4U
#define NEXUS_V1_FONT256_S2D_SECTION_PREAMBLE_BYTES 16U

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_Font256S2DSectionCorpusSpan;

typedef struct {
    int valid;
    int source_admission_bound;
    int section_bound;
    int preamble_bound;
    int capture_required;
    int glyph_layout_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t admission_ordinal;
    uint32_t section_table_index;
    uint32_t section_offset;
    uint32_t section_length;
    uint64_t section_fnv1a64;
    uint32_t preamble_offset;
    uint32_t preamble_length;
    uint64_t preamble_fnv1a64;
    /* Opaque composition measurements over the admitted section bytes only.
     * They establish no directory role, record boundary, glyph layout,
     * palette record, bit order, character encoding, or pixel data. */
    uint32_t zero_byte_count;
    uint32_t nonzero_byte_count;
    uint32_t post_preamble_word_count;
    uint32_t be16_ramp_prefix_word_count;
    int be16_ramp_full;
} Nexus_V1_Font256S2DPopulatedSectionReceipt;

typedef struct {
    int valid;
    int source_admission_bound;
    int all_sections_bound;
    int contiguous_chain_observed;
    int chain_covers_source_tail;
    int capture_required;
    int glyph_layout_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t populated_section_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t chain_fnv1a64;
    Nexus_V1_Font256S2DPopulatedSectionReceipt
        sections[NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT];
} Nexus_V1_Font256S2DSectionCorpusReceipt;

typedef struct {
    Nexus_V1_Font256S2DSectionCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_Font256S2DSectionCorpusSpanIterator;

/* Rechecks the canonical admission against the live source and publishes one
 * bounded populated-section receipt (preamble witness plus opaque raw
 * composition measurements). `ordinal` is the admission parse order 0..3,
 * not the original SCR table index. Returns 1 only for a fully matching
 * receipt, otherwise 0. */
int nexus_v1_font256_s2d_populated_section_receipt_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    uint32_t ordinal,
    Nexus_V1_Font256S2DPopulatedSectionReceipt *out_receipt);

/* Admits all four populated sections and binds their observed contiguous
 * chain. The chain span is an external-capture target only: no subspan has
 * record, glyph, palette, or pixel meaning. Returns 1 only when every
 * per-section receipt and the chain arithmetic match the live source. */
int nexus_v1_font256_s2d_section_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSectionCorpusReceipt *out_receipt);

int nexus_v1_font256_s2d_section_corpus_span_iterator_init(
    Nexus_V1_Font256S2DSectionCorpusSpanIterator *iterator,
    const Nexus_V1_Font256S2DSectionCorpusReceipt *receipt);

/* Returns 1 for each complete raw populated-section span in admission order,
 * 0 at end, -1 on invalid arguments/receipt. The iterator never creates
 * inferred subspans. */
int nexus_v1_font256_s2d_section_corpus_span_iterator_next(
    Nexus_V1_Font256S2DSectionCorpusSpanIterator *iterator,
    Nexus_V1_Font256S2DSectionCorpusSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
