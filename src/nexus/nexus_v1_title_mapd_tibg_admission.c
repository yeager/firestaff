#include "nexus_v1_title_mapd_tibg_admission.h"

#include <string.h>

/* Canonical TITLE.BIN MAPD TIBG header fields, derived from the SHA-256-
 * attested retail asset. These are provenance bindings of the same class
 * as the existing TITLE.BIN TITL/DGT2 constants: the thirteen BE32
 * header words, the marker-cell pattern and positions, the observed
 * filler-cell population, and the tail span facts. They assign no tile,
 * map, palette, colour, image, or presentation semantics. */
static const uint32_t
    k_header_fields[NEXUS_V1_TITLE_MAPD_HEADER_FIELD_COUNT] = {
    0x00008c6cU, 0x00000020U, 0x00008c2cU, 0x00008c4cU,
    0x00000020U, 0x00008c6cU, 0x00029020U, 0x00000018U,
    0x00001c1cU, 0x00003820U, 0x00005424U, 0x00007028U,
    0x00000000U
};

static const uint8_t k_marker_cell[4] = { 0x00U, 0x40U, 0x00U, 0x1cU };
static const uint8_t k_filler_cell[4] = { 0x20U, 0x20U, 0x12U, 0x00U };

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | (uint16_t)p[1]);
}

static uint32_t be32(const uint8_t *p)
{
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 |
           (uint32_t)p[2] << 8 | (uint32_t)p[3];
}

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

int nexus_v1_title_mapd_tibg_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleMapdTibgReceipt *out_receipt)
{
    Nexus_V1_TitleMapdTibgReceipt receipt;
    Nexus_V1_TitleResRecordReceipt base;
    const uint8_t *record;
    uint32_t field;
    uint32_t marker;
    uint32_t cell;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!nexus_v1_title_res_record_admit(source_bytes, source_size, identity,
            NEXUS_V1_TITLE_MAPD_ENTRY_INDEX, &base) ||
        base.entry_class != NEXUS_V1_TITLE_RES_CLASS_MAPD ||
        base.entry_id != 0U ||
        base.record_length != NEXUS_V1_TITLE_MAPD_RECORD_BYTES ||
        base.record_offset + base.record_length > source_size) {
        *out_receipt = receipt;
        return 0;
    }
    record = source_bytes + base.record_offset;
    for (field = 0U; field < NEXUS_V1_TITLE_MAPD_HEADER_FIELD_COUNT; ++field) {
        if (be32(record + 0x0cU + field * 4U) != k_header_fields[field]) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (k_header_fields[0] != NEXUS_V1_TITLE_MAPD_RECORD_BYTES - 8U) {
        *out_receipt = receipt;
        return 0;
    }
    for (marker = 0U; marker < NEXUS_V1_TITLE_MAPD_MARKER_COUNT; ++marker) {
        uint32_t at = NEXUS_V1_TITLE_MAPD_MARKER_BASE +
            marker * NEXUS_V1_TITLE_MAPD_MARKER_STRIDE;
        if (at + 4U > NEXUS_V1_TITLE_MAPD_TAIL_OFFSET ||
            memcmp(record + at, k_marker_cell, 4U) != 0) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (NEXUS_V1_TITLE_MAPD_MARKER_BASE +
            (uint32_t)NEXUS_V1_TITLE_MAPD_MARKER_COUNT *
                NEXUS_V1_TITLE_MAPD_MARKER_STRIDE !=
        NEXUS_V1_TITLE_MAPD_TAIL_OFFSET ||
        NEXUS_V1_TITLE_MAPD_TAIL_OFFSET + NEXUS_V1_TITLE_MAPD_TAIL_BYTES !=
        NEXUS_V1_TITLE_MAPD_RECORD_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.filler_cell_count = 0U;
    for (cell = 0U; cell < NEXUS_V1_TITLE_MAPD_CELL_COUNT; ++cell) {
        const uint8_t *at = record + NEXUS_V1_TITLE_MAPD_MARKER_BASE + cell * 4U;
        if (memcmp(at, k_filler_cell, 4U) == 0) {
            ++receipt.filler_cell_count;
        }
    }
    if (receipt.filler_cell_count != NEXUS_V1_TITLE_MAPD_FILLER_CELL_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    if (be16(record + NEXUS_V1_TITLE_MAPD_TAIL_OFFSET +
            NEXUS_V1_TITLE_MAPD_TAIL_BYTES - 2U) !=
        NEXUS_V1_TITLE_MAPD_TAIL_LAST_WORD) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.source_identity_bound =
        receipt.res_directory_bound = receipt.mapd_head_bound =
        receipt.header_fields_bound = receipt.marker_chain_bound =
        receipt.cell_span_bound = receipt.tail_span_bound = 1;
    receipt.source_fnv1a64 = base.source_fnv1a64;
    receipt.entry_index = base.entry_index;
    receipt.entry_id = base.entry_id;
    receipt.record_offset = base.record_offset;
    receipt.record_length = base.record_length;
    receipt.record_fnv1a64 = base.record_fnv1a64;
    receipt.header_offset = base.record_offset;
    receipt.header_fnv1a64 = fnv1a64(record, NEXUS_V1_TITLE_MAPD_HEADER_BYTES);
    receipt.header_payload_size_field = k_header_fields[0];
    receipt.header_field_0x24 = k_header_fields[6];
    receipt.cell_span_offset = base.record_offset + NEXUS_V1_TITLE_MAPD_MARKER_BASE;
    receipt.cell_span_length = NEXUS_V1_TITLE_MAPD_CELL_COUNT * 4U;
    receipt.cell_span_fnv1a64 = fnv1a64(
        source_bytes + receipt.cell_span_offset, receipt.cell_span_length);
    receipt.marker_count = NEXUS_V1_TITLE_MAPD_MARKER_COUNT;
    receipt.tail_offset = base.record_offset + NEXUS_V1_TITLE_MAPD_TAIL_OFFSET;
    receipt.tail_fnv1a64 = fnv1a64(
        source_bytes + receipt.tail_offset, NEXUS_V1_TITLE_MAPD_TAIL_BYTES);
    receipt.tail_last_word = be16(
        source_bytes + receipt.tail_offset + NEXUS_V1_TITLE_MAPD_TAIL_BYTES - 2U);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_title_mapd_tibg_span_iterator_init(
    Nexus_V1_TitleMapdTibgSpanIterator *iterator,
    const Nexus_V1_TitleMapdTibgReceipt *receipt)
{
    if (!iterator || !receipt || !receipt->valid || !receipt->cell_span_bound ||
        !receipt->tail_span_bound || receipt->tile_proven ||
        receipt->map_proven || receipt->palette_proven ||
        receipt->pixel_decode_permitted || receipt->draw_permitted ||
        receipt->presentation_permitted) return -1;
    memset(iterator, 0, sizeof(*iterator));
    iterator->receipt = *receipt;
    return 0;
}

int nexus_v1_title_mapd_tibg_span_iterator_next(
    Nexus_V1_TitleMapdTibgSpanIterator *iterator,
    Nexus_V1_TitleMapdTibgSpan *out_span)
{
    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted == 0U) {
        out_span->source_offset = iterator->receipt.cell_span_offset;
        out_span->source_length = iterator->receipt.cell_span_length;
        out_span->source_fnv1a64 = iterator->receipt.cell_span_fnv1a64;
    } else if (iterator->emitted == 1U) {
        out_span->source_offset = iterator->receipt.tail_offset;
        out_span->source_length = NEXUS_V1_TITLE_MAPD_TAIL_BYTES;
        out_span->source_fnv1a64 = iterator->receipt.tail_fnv1a64;
    } else {
        return 0;
    }
    ++iterator->emitted;
    return 1;
}
