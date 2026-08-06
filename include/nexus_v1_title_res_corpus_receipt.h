#ifndef NEXUS_V1_TITLE_RES_CORPUS_RECEIPT_H
#define NEXUS_V1_TITLE_RES_CORPUS_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TITLE.BIN RES* directory corpus receipt. The SHA-256-attested retail
 * TITLE.BIN carries a RES* container with exactly 60 directory entries in
 * four classes: 22 DGT2 records (ids 0..21), 4 TITL records (ids 0..3),
 * 1 MAPD record (id 0), and 33 CNFD records (ids 0..32). Every record head
 * repeats its directory magic and id; DGT2 and CNFD records carry a "pp"
 * tag byte pair, TITL records a "PP" pair, and the MAPD record a "TIBG"
 * tag. The records form one contiguous chain [0x2e8, 0x1b658) that covers
 * the source tail exactly. This module records those provenance facts only:
 * no byte or word is assigned image, palette, subrecord, or presentation
 * semantics, and no decode or draw route is permitted. */
#define NEXUS_V1_TITLE_BIN_BYTES 112216U
#define NEXUS_V1_TITLE_BIN_SHA256 \
    "51f1f18b68acf5993b00ffcb458ef2a7372b21595656f3ed5b95520c9a305fc3"
/* Documented English Saturn ISO revision from docs/VERIFIED_HASHES.md.  Its
 * RES* directory has the same bounded retail layout, but it is not the
 * canonical capture revision above.  Accepting this identity admits the
 * user's real European corpus without promoting title presentation. */
#define NEXUS_V1_TITLE_BIN_ENGLISH_SHA256 \
    "a634e8daf2a581df154b454919ee2ed44e937371668219d7cdf6d0983a613e44"

#define NEXUS_V1_TITLE_RES_ENTRY_COUNT 60U
#define NEXUS_V1_TITLE_RES_ENTRY_BYTES 12U
#define NEXUS_V1_TITLE_RES_HEAD_BYTES 16U
#define NEXUS_V1_TITLE_RES_FIRST_OFFSET 0x2e8U

#define NEXUS_V1_TITLE_RES_DGT2_COUNT 22U
#define NEXUS_V1_TITLE_RES_TITL_COUNT 4U
#define NEXUS_V1_TITLE_RES_MAPD_COUNT 1U
#define NEXUS_V1_TITLE_RES_CNFD_COUNT 33U

typedef enum {
    NEXUS_V1_TITLE_RES_CLASS_DGT2 = 0,
    NEXUS_V1_TITLE_RES_CLASS_TITL = 1,
    NEXUS_V1_TITLE_RES_CLASS_MAPD = 2,
    NEXUS_V1_TITLE_RES_CLASS_CNFD = 3
} Nexus_V1_TitleResClass;

/* Live source identity. sha256_verified/sha256_hex attest the pinned
 * canonical asset class or the documented English retail revision above;
 * source_fnv1a64 binds the exact live bytes (real retail asset or a synthetic
 * mirror in tests). No canonical FNV is pinned so that synthetic dual-mode
 * tests can bind their own live bytes. */
typedef struct {
    int sha256_verified;
    const char *sha256_hex;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleResSourceIdentity;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int entry_bound;
    int record_head_bound;
    int record_span_bound;
    int pp_tag_observed;
    int PP_tag_observed;
    int tibg_tag_observed;
    uint64_t source_fnv1a64;
    uint32_t entry_index;
    uint8_t entry_class;
    uint32_t entry_id;
    uint32_t record_offset;
    uint32_t record_length;
    uint64_t record_fnv1a64;
    uint64_t entry_fnv1a64;
    uint64_t record_head_fnv1a64;
    uint32_t record_inner_id;
    /* Raw head words at record +8/+10/+12/+14 retained as opaque
     * measurements. They establish no record grammar, palette, image, or
     * pixel meaning. */
    uint16_t head_tag_word;
    uint16_t head_word0;
    uint16_t head_word1;
    uint16_t head_word2;
} Nexus_V1_TitleResRecordReceipt;

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int all_records_bound;
    int contiguous_chain_observed;
    int chain_covers_source_tail;
    int record_grammar_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t entry_count;
    uint32_t dgt2_count;
    uint32_t titl_count;
    uint32_t mapd_count;
    uint32_t cnfd_count;
    uint32_t chain_offset;
    uint32_t chain_length;
    uint64_t table_fnv1a64;
    uint64_t chain_fnv1a64;
    Nexus_V1_TitleResRecordReceipt records[NEXUS_V1_TITLE_RES_ENTRY_COUNT];
} Nexus_V1_TitleResCorpusReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleResSpan;

typedef struct {
    Nexus_V1_TitleResCorpusReceipt receipt;
    uint32_t emitted;
} Nexus_V1_TitleResSpanIterator;

/* Rechecks the canonical RES* directory against the live source and
 * publishes one bounded per-record receipt (directory entry, record head,
 * and whole-record span bindings plus raw head words). Returns 1 only for
 * a fully matching receipt, otherwise 0. */
int nexus_v1_title_res_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t entry_index,
    Nexus_V1_TitleResRecordReceipt *out_receipt);

/* Admits all 60 directory records and binds their observed contiguous
 * chain and tail coverage. The chain span is an external-capture target
 * only: no subspan has record, image, palette, or presentation meaning.
 * Returns 1 only when every per-record receipt, the class counts, and the
 * chain arithmetic match the live source. */
int nexus_v1_title_res_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleResCorpusReceipt *out_receipt);

int nexus_v1_title_res_span_iterator_init(
    Nexus_V1_TitleResSpanIterator *iterator,
    const Nexus_V1_TitleResCorpusReceipt *receipt);

/* Returns 1 for each complete raw record span in directory order, 0 at
 * end, -1 on invalid arguments/receipt. The iterator never creates
 * inferred subspans. */
int nexus_v1_title_res_span_iterator_next(
    Nexus_V1_TitleResSpanIterator *iterator,
    Nexus_V1_TitleResSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
