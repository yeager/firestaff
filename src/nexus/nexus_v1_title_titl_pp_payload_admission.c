#include "nexus_v1_title_titl_pp_payload_admission.h"

#include <string.h>

/* Canonical TITLE.BIN TITL PP payload facts, derived from the SHA-256-
 * attested retail asset. These are provenance bindings of the same class
 * as the existing WARNING.BIN DGT2/PP constants: PP header dimensions,
 * the 512-byte post-header prefix, the width*height plane, and the two
 * trailing bytes per record, plus the observed contiguous sub-chain. They
 * assign no colour, palette, image, pixel, or presentation semantics
 * beyond the already admitted ST-124 section-6 PP contract. */
static const uint16_t k_titl_width[NEXUS_V1_TITLE_TITL_COUNT] = {
    304U, 160U, 304U, 256U
};
static const uint16_t k_titl_height[NEXUS_V1_TITLE_TITL_COUNT] = {
    104U, 28U, 22U, 16U
};

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | (uint16_t)p[1]);
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

int nexus_v1_title_titl_pp_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t titl_index,
    Nexus_V1_TitleTitlPpRecordReceipt *out_receipt)
{
    Nexus_V1_TitleTitlPpRecordReceipt receipt;
    Nexus_V1_TitleResRecordReceipt base;
    const uint8_t *record;
    uint32_t entry_index;
    uint32_t plane_length;
    uint32_t expected_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (titl_index >= NEXUS_V1_TITLE_TITL_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    entry_index = NEXUS_V1_TITLE_TITL_FIRST_ENTRY_INDEX + titl_index;
    if (!nexus_v1_title_res_record_admit(source_bytes, source_size, identity,
            entry_index, &base) ||
        base.entry_class != NEXUS_V1_TITLE_RES_CLASS_TITL ||
        base.entry_id != titl_index ||
        base.record_length < NEXUS_V1_TITLE_TITL_HEAD_BYTES +
            NEXUS_V1_TITLE_TITL_PREFIX_BYTES +
            NEXUS_V1_TITLE_TITL_TRAILING_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    record = source_bytes + base.record_offset;
    if (be16(record + 10U) != k_titl_width[titl_index] ||
        be16(record + 12U) != k_titl_height[titl_index] ||
        be16(record + 14U) != NEXUS_V1_TITLE_TITL_PREFIX_LEADING_WORD) {
        *out_receipt = receipt;
        return 0;
    }
    plane_length = (uint32_t)k_titl_width[titl_index] *
                   (uint32_t)k_titl_height[titl_index];
    expected_length = NEXUS_V1_TITLE_TITL_HEAD_BYTES +
        NEXUS_V1_TITLE_TITL_PREFIX_BYTES + plane_length +
        NEXUS_V1_TITLE_TITL_TRAILING_BYTES;
    if (base.record_length != expected_length ||
        base.record_offset + base.record_length > source_size) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.source_identity_bound =
        receipt.res_directory_bound = receipt.titl_head_bound =
        receipt.pp_header_bound = receipt.prefix_span_bound =
        receipt.plane_span_bound = receipt.trailing_span_bound =
        receipt.length_arithmetic_bound = 1;
    receipt.source_fnv1a64 = base.source_fnv1a64;
    receipt.titl_index = titl_index;
    receipt.entry_index = entry_index;
    receipt.entry_id = base.entry_id;
    receipt.record_offset = base.record_offset;
    receipt.record_length = base.record_length;
    receipt.record_fnv1a64 = base.record_fnv1a64;
    receipt.width = k_titl_width[titl_index];
    receipt.height = k_titl_height[titl_index];
    receipt.prefix_leading_word = be16(record + 14U);
    receipt.prefix_offset = base.record_offset + NEXUS_V1_TITLE_TITL_HEAD_BYTES;
    receipt.prefix_fnv1a64 = fnv1a64(
        source_bytes + receipt.prefix_offset,
        NEXUS_V1_TITLE_TITL_PREFIX_BYTES);
    receipt.plane_offset = receipt.prefix_offset +
        NEXUS_V1_TITLE_TITL_PREFIX_BYTES;
    receipt.plane_length = plane_length;
    receipt.plane_fnv1a64 =
        fnv1a64(source_bytes + receipt.plane_offset, plane_length);
    receipt.trailing_offset = receipt.plane_offset + plane_length;
    receipt.trailing_fnv1a64 = fnv1a64(
        source_bytes + receipt.trailing_offset,
        NEXUS_V1_TITLE_TITL_TRAILING_BYTES);
    receipt.trailing_be16 = be16(source_bytes + receipt.trailing_offset);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_title_titl_pp_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleTitlPpCorpusReceipt *out_receipt)
{
    Nexus_V1_TitleTitlPpCorpusReceipt receipt;
    uint32_t index;
    uint64_t chain_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (index = 0U; index < NEXUS_V1_TITLE_TITL_COUNT; ++index) {
        if (!nexus_v1_title_titl_pp_record_admit(source_bytes, source_size,
                identity, index, &receipt.records[index])) {
            memset(&receipt, 0, sizeof(receipt));
            *out_receipt = receipt;
            return 0;
        }
        if (index > 0U &&
            receipt.records[index].record_offset !=
                receipt.records[index - 1U].record_offset +
                    receipt.records[index - 1U].record_length) {
            memset(&receipt, 0, sizeof(receipt));
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.chain_offset = receipt.records[0].record_offset;
    for (index = 0U; index < NEXUS_V1_TITLE_TITL_COUNT; ++index) {
        receipt.chain_length += receipt.records[index].record_length;
    }
    chain_end = (uint64_t)receipt.chain_offset + (uint64_t)receipt.chain_length;
    if (receipt.chain_offset != NEXUS_V1_TITLE_TITL_CHAIN_OFFSET ||
        chain_end != (uint64_t)NEXUS_V1_TITLE_TITL_CHAIN_END) {
        memset(&receipt, 0, sizeof(receipt));
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.source_identity_bound =
        receipt.res_directory_bound = receipt.all_titl_bound = 1;
    receipt.contiguous_chain_observed = 1;
    receipt.shared_prefix_observed =
        receipt.records[0].prefix_fnv1a64 == receipt.records[1].prefix_fnv1a64 &&
        receipt.records[0].prefix_fnv1a64 == receipt.records[2].prefix_fnv1a64 &&
        receipt.records[0].prefix_fnv1a64 == receipt.records[3].prefix_fnv1a64;
    receipt.shared_prefix_fnv1a64 = receipt.records[0].prefix_fnv1a64;
    receipt.source_fnv1a64 = receipt.records[0].source_fnv1a64;
    receipt.titl_count = NEXUS_V1_TITLE_TITL_COUNT;
    receipt.chain_fnv1a64 =
        fnv1a64(source_bytes + receipt.chain_offset, receipt.chain_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_title_titl_pp_plane_span_iterator_init(
    Nexus_V1_TitleTitlPpPlaneSpanIterator *iterator,
    const Nexus_V1_TitleTitlPpCorpusReceipt *receipt)
{
    uint32_t index;
    if (!iterator || !receipt || !receipt->valid || !receipt->all_titl_bound ||
        receipt->colour_proven || receipt->palette_proven ||
        receipt->pixel_decode_permitted || receipt->draw_permitted ||
        receipt->presentation_permitted ||
        receipt->titl_count != NEXUS_V1_TITLE_TITL_COUNT) return -1;
    for (index = 0U; index < NEXUS_V1_TITLE_TITL_COUNT; ++index) {
        if (!receipt->records[index].valid ||
            !receipt->records[index].plane_length ||
            !receipt->records[index].plane_fnv1a64) return -1;
    }
    memset(iterator, 0, sizeof(*iterator));
    iterator->receipt = *receipt;
    return 0;
}

int nexus_v1_title_titl_pp_plane_span_iterator_next(
    Nexus_V1_TitleTitlPpPlaneSpanIterator *iterator,
    Nexus_V1_TitleTitlPpPlaneSpan *out_span)
{
    const Nexus_V1_TitleTitlPpRecordReceipt *record;
    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted >= NEXUS_V1_TITLE_TITL_COUNT) return 0;
    record = &iterator->receipt.records[iterator->emitted];
    out_span->source_offset = record->plane_offset;
    out_span->source_length = record->plane_length;
    out_span->source_fnv1a64 = record->plane_fnv1a64;
    ++iterator->emitted;
    return 1;
}
