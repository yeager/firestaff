#ifndef NEXUS_V1_TITLE_TITL_PP_PAYLOAD_ADMISSION_H
#define NEXUS_V1_TITLE_TITL_PP_PAYLOAD_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_title_res_corpus_receipt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TITLE.BIN TITL PP payload admission. The four TITL directory records
 * (entries 22..25) each carry a PP payload of the already admitted ST-124
 * section-6 shape: a six-byte PP header ("PP" tag, BE16 width, BE16
 * height), a 512-byte post-header prefix, a width*height byte plane, and
 * two trailing bytes before the next record. The canonical dimensions are
 * 304x104, 160x28, 304x22, and 256x16 with a shared 0x8220 leading word
 * at the head of the 512-byte prefix, and all four 512-byte prefixes are
 * byte-identical in the canonical attested source. The documented English
 * retail revision keeps the same bounded record spans but has a distinct
 * second prefix; the receipt records that observation instead of treating
 * it as canonical shared data.
 * The four records form one contiguous sub-chain [0x2318, 0xe278) inside
 * the whole-file chain. This module binds those provenance facts only: no
 * byte or word is assigned colour, palette, image, pixel, or presentation
 * meaning, and no decode or draw route is permitted. */
#define NEXUS_V1_TITLE_TITL_COUNT 4U
#define NEXUS_V1_TITLE_TITL_HEAD_BYTES 14U
#define NEXUS_V1_TITLE_TITL_PREFIX_BYTES 512U
#define NEXUS_V1_TITLE_TITL_TRAILING_BYTES 2U
#define NEXUS_V1_TITLE_TITL_FIRST_ENTRY_INDEX 22U
#define NEXUS_V1_TITLE_TITL_CHAIN_OFFSET 0x2318U
#define NEXUS_V1_TITLE_TITL_CHAIN_END 0xe278U
#define NEXUS_V1_TITLE_TITL_PREFIX_LEADING_WORD 0x8220U

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int titl_head_bound;
    int pp_header_bound;
    int prefix_span_bound;
    int plane_span_bound;
    int trailing_span_bound;
    int length_arithmetic_bound;
    int colour_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t titl_index;
    uint32_t entry_index;
    uint32_t entry_id;
    uint32_t record_offset;
    uint32_t record_length;
    uint64_t record_fnv1a64;
    /* Header values only; they establish no image, palette, stride, colour,
     * or pixel meaning. */
    uint16_t width;
    uint16_t height;
    uint16_t prefix_leading_word;
    uint32_t prefix_offset;
    uint64_t prefix_fnv1a64;
    uint32_t plane_offset;
    uint32_t plane_length;
    uint64_t plane_fnv1a64;
    uint32_t trailing_offset;
    uint64_t trailing_fnv1a64;
    uint16_t trailing_be16;
} Nexus_V1_TitleTitlPpRecordReceipt;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int all_titl_bound;
    int contiguous_chain_observed;
    int shared_prefix_observed;
    int colour_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t titl_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t chain_fnv1a64;
    uint64_t shared_prefix_fnv1a64;
    Nexus_V1_TitleTitlPpRecordReceipt records[NEXUS_V1_TITLE_TITL_COUNT];
} Nexus_V1_TitleTitlPpCorpusReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleTitlPpPlaneSpan;

typedef struct {
    Nexus_V1_TitleTitlPpCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_TitleTitlPpPlaneSpanIterator;

/* Revalidates the TITLE.BIN RES* directory receipt for TITL entry
 * 22 + titl_index against the live source and publishes one bounded PP
 * payload receipt (head words, prefix/plane/trailing spans, and the exact
 * 14 + 512 + width*height + 2 length arithmetic). Returns 1 only for a
 * fully matching receipt, otherwise 0. */
int nexus_v1_title_titl_pp_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t titl_index,
    Nexus_V1_TitleTitlPpRecordReceipt *out_receipt);

/* Admits all four TITL PP payloads and binds their observed contiguous
 * sub-chain and any shared 512-byte prefix observation.
 * Returns 1 only when every per-record receipt and the chain arithmetic
 * match the live source. */
int nexus_v1_title_titl_pp_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleTitlPpCorpusReceipt *out_receipt);

int nexus_v1_title_titl_pp_plane_span_iterator_init(
    Nexus_V1_TitleTitlPpPlaneSpanIterator *iterator,
    const Nexus_V1_TitleTitlPpCorpusReceipt *receipt);

/* Returns 1 for each complete raw width*height plane span in TITL order,
 * 0 at end, -1 on invalid arguments/receipt. The spans are bounded raw
 * ranges only; the iterator never decodes pixels. */
int nexus_v1_title_titl_pp_plane_span_iterator_next(
    Nexus_V1_TitleTitlPpPlaneSpanIterator *iterator,
    Nexus_V1_TitleTitlPpPlaneSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
