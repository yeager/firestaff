#ifndef NEXUS_V1_TITLE_MAPD_TIBG_ADMISSION_H
#define NEXUS_V1_TITLE_MAPD_TIBG_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_title_res_corpus_receipt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* TITLE.BIN MAPD TIBG payload admission. The single MAPD directory
 * record (entry 26, id 0) carries a "TIBG" payload of one observed
 * shape: a 64-byte header ("MAPD" magic, id, "TIBG" tag, and thirteen
 * canonical BE32 fields), a 4-byte-cell span [0x40, 0x8c54) holding 8965
 * cells with exactly five marker cells 00 40 00 1c at 0x40 + k*0x1c04
 * (k = 0..4), an observed filler-cell population, and a 32-byte tail of
 * sixteen BE16 words whose last word is 0xffff. Header fields, marker
 * positions, and span boundaries close arithmetically against the
 * canonical record length 0x8c74. This module binds those provenance
 * facts only: no byte, cell, or word is assigned tile, map, palette,
 * colour, image, or presentation meaning, and no decode or draw route
 * is permitted. */
#define NEXUS_V1_TITLE_MAPD_ENTRY_INDEX 26U
#define NEXUS_V1_TITLE_MAPD_RECORD_BYTES 0x8c74U
#define NEXUS_V1_TITLE_MAPD_HEADER_BYTES 64U
#define NEXUS_V1_TITLE_MAPD_HEADER_FIELD_COUNT 13U
#define NEXUS_V1_TITLE_MAPD_MARKER_BASE 0x40U
#define NEXUS_V1_TITLE_MAPD_MARKER_COUNT 5U
#define NEXUS_V1_TITLE_MAPD_MARKER_STRIDE 0x1c04U
#define NEXUS_V1_TITLE_MAPD_CELL_COUNT 8965U
#define NEXUS_V1_TITLE_MAPD_FILLER_CELL_COUNT 3360U
#define NEXUS_V1_TITLE_MAPD_TAIL_OFFSET 0x8c54U
#define NEXUS_V1_TITLE_MAPD_TAIL_BYTES 32U
#define NEXUS_V1_TITLE_MAPD_TAIL_WORDS 16U
#define NEXUS_V1_TITLE_MAPD_TAIL_LAST_WORD 0xffffU

typedef struct {
    int valid;
    int source_identity_bound;
    int res_directory_bound;
    int mapd_head_bound;
    int header_fields_bound;
    int marker_chain_bound;
    int cell_span_bound;
    int tail_span_bound;
    int tile_proven;
    int map_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    int presentation_permitted;
    uint64_t source_fnv1a64;
    uint32_t entry_index;
    uint32_t entry_id;
    uint32_t record_offset;
    uint32_t record_length;
    uint64_t record_fnv1a64;
    uint32_t header_offset;
    uint64_t header_fnv1a64;
    /* Opaque header measurements only; they establish no tile, map,
     * palette, or address meaning. */
    uint32_t header_payload_size_field;
    uint32_t header_field_0x24;
    uint32_t cell_span_offset;
    uint32_t cell_span_length;
    uint64_t cell_span_fnv1a64;
    uint32_t filler_cell_count;
    uint32_t marker_count;
    uint32_t tail_offset;
    uint64_t tail_fnv1a64;
    uint16_t tail_last_word;
} Nexus_V1_TitleMapdTibgReceipt;

typedef struct {
    uint32_t source_offset;
    uint32_t source_length;
    uint64_t source_fnv1a64;
} Nexus_V1_TitleMapdTibgSpan;

typedef struct {
    Nexus_V1_TitleMapdTibgReceipt receipt;
    uint32_t emitted;
} Nexus_V1_TitleMapdTibgSpanIterator;

/* Revalidates the TITLE.BIN RES* directory receipt for the MAPD entry
 * against the live source and publishes one bounded TIBG payload receipt
 * (header fields, marker chain, cell span with filler population, and
 * tail span). Returns 1 only for a fully matching receipt, otherwise 0. */
int nexus_v1_title_mapd_tibg_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleMapdTibgReceipt *out_receipt);

int nexus_v1_title_mapd_tibg_span_iterator_init(
    Nexus_V1_TitleMapdTibgSpanIterator *iterator,
    const Nexus_V1_TitleMapdTibgReceipt *receipt);

/* Returns 1 for the raw cell span and then the raw tail span, 0 at end,
 * -1 on invalid arguments/receipt. The spans are bounded raw ranges
 * only; the iterator never decodes cells or words. */
int nexus_v1_title_mapd_tibg_span_iterator_next(
    Nexus_V1_TitleMapdTibgSpanIterator *iterator,
    Nexus_V1_TitleMapdTibgSpan *out_span);

#ifdef __cplusplus
}
#endif

#endif
