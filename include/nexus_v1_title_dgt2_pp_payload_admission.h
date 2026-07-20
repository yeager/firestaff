#ifndef NEXUS_V1_TITLE_DGT2_PP_PAYLOAD_ADMISSION_H
#define NEXUS_V1_TITLE_DGT2_PP_PAYLOAD_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_title_res_corpus_receipt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TITLE.BIN DGT2 payload admission. The 22 DGT2 directory records
 * (entries 0..21) each carry a payload of one observed shape: a 16-byte
 * head ("DGT2" magic, class-local id, "pp" tag, BE16 width, BE16 height,
 * BE16 flag word), a 32-byte post-head prefix, and a packed byte plane of
 * width*height/2 bytes, with exact length arithmetic
 * 16 + 32 + width*height/2 per record and no trailing bytes. The
 * canonical dimensions are 64x8 (records 0..3), 104x8 (4..5), 24x24
 * (6..20), and 168x12 (21); the canonical flag word is 0x8220 except
 * 0x81e0 for records 6..20. The 22 records form one contiguous sub-chain
 * [0x2e8, 0x2318) inside the whole-file chain, and exactly two prefix
 * pairs are byte-identical in the attested source ((2,4) and (3,5); 20
 * distinct prefixes of 22). This module binds those provenance facts
 * only: no byte or word is assigned colour, palette, image, pixel, or
 * presentation meaning, and no decode or draw route is permitted. */
#define NEXUS_V1_TITLE_DGT2_COUNT 22U
#define NEXUS_V1_TITLE_DGT2_HEAD_BYTES 16U
#define NEXUS_V1_TITLE_DGT2_PREFIX_BYTES 32U
#define NEXUS_V1_TITLE_DGT2_FIRST_ENTRY_INDEX 0U
#define NEXUS_V1_TITLE_DGT2_CHAIN_OFFSET 0x2e8U
#define NEXUS_V1_TITLE_DGT2_CHAIN_END 0x2318U
#define NEXUS_V1_TITLE_DGT2_FLAG_A 0x8220U
#define NEXUS_V1_TITLE_DGT2_FLAG_B 0x81e0U
#define NEXUS_V1_TITLE_DGT2_DISTINCT_PREFIX_COUNT 20U

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int dgt2_head_bound;
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
    uint32_t dgt2_index;
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
} Nexus_V1_TitleDgt2PpRecordReceipt;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int all_dgt2_bound;
    int contiguous_chain_observed;
    int shared_prefix_pair_2_4_observed;
    int shared_prefix_pair_3_5_observed;
    int colour_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t dgt2_count;
    uint32_t distinct_prefix_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t chain_fnv1a64;
    Nexus_V1_TitleDgt2PpRecordReceipt records[NEXUS_V1_TITLE_DGT2_COUNT];
} Nexus_V1_TitleDgt2PpCorpusReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleDgt2PpPlaneSpan;

typedef struct {
    Nexus_V1_TitleDgt2PpCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_TitleDgt2PpPlaneSpanIterator;

/* Revalidates the TITLE.BIN RES* directory receipt for DGT2 entry
 * dgt2_index (0..21) against the live source and publishes one bounded
 * payload receipt (head words, prefix/plane spans, and the exact
 * 16 + 32 + width*height/2 length arithmetic). Returns 1 only for a
 * fully matching receipt, otherwise 0. */
int nexus_v1_title_dgt2_pp_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t dgt2_index,
    Nexus_V1_TitleDgt2PpRecordReceipt *out_receipt);

/* Admits all 22 DGT2 payloads and binds their observed contiguous
 * sub-chain, the distinct-prefix count, and the two byte-identical
 * prefix-pair observations. Returns 1 only when every per-record receipt
 * and the chain arithmetic match the live source. */
int nexus_v1_title_dgt2_pp_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleDgt2PpCorpusReceipt *out_receipt);

int nexus_v1_title_dgt2_pp_plane_span_iterator_init(
    Nexus_V1_TitleDgt2PpPlaneSpanIterator *iterator,
    const Nexus_V1_TitleDgt2PpCorpusReceipt *receipt);

/* Returns 1 for each complete raw width*height/2 plane span in DGT2
 * order, 0 at end, -1 on invalid arguments/receipt. The spans are
 * bounded raw ranges only; the iterator never decodes pixels. */
int nexus_v1_title_dgt2_pp_plane_span_iterator_next(
    Nexus_V1_TitleDgt2PpPlaneSpanIterator *iterator,
    Nexus_V1_TitleDgt2PpPlaneSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
