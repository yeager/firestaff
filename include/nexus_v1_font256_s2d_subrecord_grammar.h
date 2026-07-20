#ifndef NEXUS_V1_FONT256_S2D_SUBRECORD_GRAMMAR_H
#define NEXUS_V1_FONT256_S2D_SUBRECORD_GRAMMAR_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_font256_s2d_section_corpus_receipt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FONT256.S2D populated-section subrecord grammar admission. On top of the
 * existing section corpus receipt this module binds the observed internal
 * subrecord arithmetic of three of the four populated SCR sections, still
 * as opaque measurements with no text, glyph, palette, record, encoding,
 * or pixel meaning:
 *  - ordinal 0 (table index 0, 8208 bytes): a 16-byte preamble of the
 *    eight canonical BE16 words {0x0010, 0x0000, 0x4000, 0xffff x5}
 *    followed by a 4096-word BE16 ramp with word[i] == 2*(i & 2047)
 *    (two identical 2048-word step-2 half ramps 0x0000..0x0ffe); the
 *    arithmetic 16 + 4096*2 = 8208 closes the section exactly.
 *  - ordinal 1 (table index 2, 15504 bytes): NO subrecord grammar is
 *    bound; measured over canonical 16-byte blocks the section shows
 *    742 populated of 969 blocks, an opaque composition measurement
 *    only. This section stays capture-required with no proven
 *    subrecord structure.
 *  - ordinal 2 (table index 4, 528 bytes): exactly 33 sixteen-byte
 *    records; records 0..2 carry canonical BE16 word sequences and
 *    records 3..32 (30 records) each carry eight words of 0x8000. The
 *    arithmetic 33*16 = 528 closes the section exactly.
 *  - ordinal 3 (table index 6, 484 bytes): the whole section is zero.
 * A bounded iterator exposes exactly the 38 raw subrecord spans
 * (3 + 1 + 33 + 1) whose lengths sum to the populated chain length.
 * No byte or word is assigned text, glyph, palette, record, encoding,
 * or pixel meaning, and no draw route is permitted. */
#define NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS 8U
#define NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS 4096U
#define NEXUS_V1_FONT256_S2D_SECTION0_RAMP_HALF_WORDS 2048U
#define NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES 16U
#define NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT 969U
#define NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT 742U
#define NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES 16U
#define NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS 8U
#define NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT 33U
#define NEXUS_V1_FONT256_S2D_SECTION4_BASE_RECORD_COUNT 30U
#define NEXUS_V1_FONT256_S2D_SECTION4_BASE_WORD 0x8000U
#define NEXUS_V1_FONT256_S2D_SUBRECORD_SPAN_COUNT 38U

typedef struct {
    int valid;
    int source_admission_bound;
    int section_bound;
    int subrecord_grammar_bound;
    int section_all_zero;
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
    /* Opaque composition measurements only; see module comment. */
    uint32_t preamble_word_count;
    uint32_t ramp_word_count;
    uint32_t ramp_half_word_count;
    uint32_t block_count;
    uint32_t populated_block_count;
    uint32_t record_count;
    uint32_t base_record_count;
} Nexus_V1_Font256S2DSubrecordReceipt;

typedef struct {
    int valid;
    int source_admission_bound;
    int all_sections_bound;
    int section0_grammar_bound;
    int section2_grammar_negative;
    int section4_grammar_bound;
    int section6_zero_bound;
    int capture_required;
    int glyph_layout_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t populated_section_count;
    Nexus_V1_Font256S2DSubrecordReceipt
        sections[NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT];
    /* Live digests of the 38 raw subrecord spans in file order; they
     * bind no span semantics. */
    uint64_t subrecord_span_fnv1a64[NEXUS_V1_FONT256_S2D_SUBRECORD_SPAN_COUNT];
} Nexus_V1_Font256S2DSubrecordCorpusReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_Font256S2DSubrecordSpan;

typedef struct {
    Nexus_V1_Font256S2DSubrecordCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_Font256S2DSubrecordSpanIterator;

/* Rechecks the canonical admission and the section corpus receipt against
 * the live source and publishes one bounded subrecord receipt for the
 * populated section at admission ordinal 0..3. Ordinals 0, 2, and 3 bind
 * the observed subrecord arithmetic above; ordinal 1 binds only the opaque
 * block-population measurement (subrecord_grammar_bound stays 0). Returns
 * 1 only for a fully matching receipt, otherwise 0. */
int nexus_v1_font256_s2d_subrecord_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    uint32_t ordinal,
    Nexus_V1_Font256S2DSubrecordReceipt *out_receipt);

/* Admits all four populated-section subrecord receipts and records the
 * live digests of the 38 raw subrecord spans. Returns 1 only when every
 * per-section receipt matches the live source. */
int nexus_v1_font256_s2d_subrecord_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSubrecordCorpusReceipt *out_receipt);

int nexus_v1_font256_s2d_subrecord_span_iterator_init(
    Nexus_V1_Font256S2DSubrecordSpanIterator *iterator,
    const Nexus_V1_Font256S2DSubrecordCorpusReceipt *receipt);

/* Returns 1 for each of the 38 raw subrecord spans in file order
 * (section 0 preamble, two half ramps; the whole section 2 as one
 * opaque span; the 33 section 4 records; the whole zero section 6),
 * 0 at end, -1 on invalid arguments/receipt. The spans are bounded
 * raw ranges only; the iterator never decodes glyphs or pixels. */
int nexus_v1_font256_s2d_subrecord_span_iterator_next(
    Nexus_V1_Font256S2DSubrecordSpanIterator *iterator,
    Nexus_V1_Font256S2DSubrecordSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
