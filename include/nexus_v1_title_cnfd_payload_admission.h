#ifndef NEXUS_V1_TITLE_CNFD_PAYLOAD_ADMISSION_H
#define NEXUS_V1_TITLE_CNFD_PAYLOAD_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_title_res_corpus_receipt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TITLE.BIN CNFD payload admission. The 33 CNFD directory records
 * (entries 27..59) each carry a payload of the same observed shape as
 * the DGT2 records: a 16-byte head ("CNFD" magic, class-local id, "pp"
 * tag, BE16 width, BE16 height, BE16 flag word), a 32-byte post-head
 * prefix, and a packed width*height/2 byte plane, with exact length
 * arithmetic 16 + 32 + width*height/2 per record and no trailing bytes.
 * The canonical flag word is 0x8000 for records {0, 6, 12, 18, 24, 30}
 * and 0x8b00 otherwise. The 33 records form one contiguous sub-chain
 * [0x16eec, 0x1b658) that covers the source tail exactly, and the 33
 * prefixes reduce to an observed 8 distinct values. This module binds
 * those provenance facts only: no byte or word is assigned colour,
 * palette, image, pixel, or presentation meaning, and no decode or
 * draw route is permitted. */
#define NEXUS_V1_TITLE_CNFD_COUNT 33U
#define NEXUS_V1_TITLE_CNFD_HEAD_BYTES 16U
#define NEXUS_V1_TITLE_CNFD_PREFIX_BYTES 32U
#define NEXUS_V1_TITLE_CNFD_FIRST_ENTRY_INDEX 27U
#define NEXUS_V1_TITLE_CNFD_CHAIN_OFFSET 0x16eecU
#define NEXUS_V1_TITLE_CNFD_CHAIN_END 0x1b658U
#define NEXUS_V1_TITLE_CNFD_FLAG_A 0x8000U
#define NEXUS_V1_TITLE_CNFD_FLAG_B 0x8b00U
#define NEXUS_V1_TITLE_CNFD_DISTINCT_PREFIX_COUNT 8U

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int cnfd_head_bound;
    int pp_header_bound;
    int prefix_span_bound;
    int plane_span_bound;
    int length_arithmetic_bound;
    int colour_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t cnfd_index;
    uint32_t entry_index;
    uint32_t entry_id;
    uint32_t record_offset;
    uint32_t record_length;
    uint64_t record_fnv1a64;
    /* Header values only; they establish no image, palette, stride, colour,
     * or pixel meaning. */
    uint16_t width;
    uint16_t height;
    uint16_t flag_word;
    uint32_t prefix_offset;
    uint64_t prefix_fnv1a64;
    uint32_t plane_offset;
    uint32_t plane_length;
    uint64_t plane_fnv1a64;
} Nexus_V1_TitleCnfdRecordReceipt;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int all_cnfd_bound;
    int contiguous_chain_observed;
    int chain_covers_source_tail;
    int colour_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t cnfd_count;
    uint32_t distinct_prefix_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t chain_fnv1a64;
    Nexus_V1_TitleCnfdRecordReceipt records[NEXUS_V1_TITLE_CNFD_COUNT];
} Nexus_V1_TitleCnfdCorpusReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleCnfdPlaneSpan;

typedef struct {
    Nexus_V1_TitleCnfdCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_TitleCnfdPlaneSpanIterator;

/* Revalidates the TITLE.BIN RES* directory receipt for CNFD entry
 * 27 + cnfd_index (0..32) against the live source and publishes one
 * bounded payload receipt (head words, prefix/plane spans, and the
 * exact 16 + 32 + width*height/2 length arithmetic). Returns 1 only for
 * a fully matching receipt, otherwise 0. */
int nexus_v1_title_cnfd_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t cnfd_index,
    Nexus_V1_TitleCnfdRecordReceipt *out_receipt);

/* Admits all 33 CNFD payloads and binds their observed contiguous
 * sub-chain, tail coverage, and distinct-prefix count. Returns 1 only
 * when every per-record receipt and the chain arithmetic match the live
 * source. */
int nexus_v1_title_cnfd_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleCnfdCorpusReceipt *out_receipt);

int nexus_v1_title_cnfd_plane_span_iterator_init(
    Nexus_V1_TitleCnfdPlaneSpanIterator *iterator,
    const Nexus_V1_TitleCnfdCorpusReceipt *receipt);

/* Returns 1 for each complete raw width*height/2 plane span in CNFD
 * order, 0 at end, -1 on invalid arguments/receipt. The spans are
 * bounded raw ranges only; the iterator never decodes pixels. */
int nexus_v1_title_cnfd_plane_span_iterator_next(
    Nexus_V1_TitleCnfdPlaneSpanIterator *iterator,
    Nexus_V1_TitleCnfdPlaneSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
