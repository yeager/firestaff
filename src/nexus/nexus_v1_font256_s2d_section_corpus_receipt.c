#include "nexus_v1_font256_s2d_section_corpus_receipt.h"

#include <string.h>

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

static int admission_usable(const Nexus_V1_Font256S2DAdmissionReceipt *admission)
{
    return admission && admission->valid && admission->source_identity_bound &&
        admission->scr_header_bound && admission->section_table_bound &&
        !admission->glyph_layout_proven && !admission->pixel_decode_permitted &&
        !admission->draw_permitted &&
        admission->section_count == NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
}

static int source_table_recheck(const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission, uint64_t *out_source_fnv)
{
    uint64_t source_fnv;
    if (!source_bytes || source_size != (size_t)admission->source_bytes) return 0;
    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != admission->source_fnv1a64) return 0;
    if (fnv1a64(source_bytes + NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET,
                NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX *
                    NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE) !=
        admission->section_table_fnv1a64) return 0;
    if (out_source_fnv) *out_source_fnv = source_fnv;
    return 1;
}

int nexus_v1_font256_s2d_populated_section_receipt_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    uint32_t ordinal,
    Nexus_V1_Font256S2DPopulatedSectionReceipt *out_receipt)
{
    Nexus_V1_Font256S2DPopulatedSectionReceipt receipt;
    const Nexus_V1_FontSection *section;
    const uint8_t *section_bytes;
    uint64_t source_fnv;
    uint32_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    if (!admission_usable(admission) ||
        ordinal >= NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT ||
        !source_table_recheck(source_bytes, source_size, admission, &source_fnv)) {
        *out_receipt = receipt;
        return 0;
    }
    section = &admission->sections[ordinal];
    if (section->file_offset > source_size ||
        section->size > source_size - section->file_offset ||
        section->size < NEXUS_V1_FONT256_S2D_SECTION_PREAMBLE_BYTES ||
        fnv1a64(source_bytes + section->file_offset, section->size) !=
            admission->section_fnv1a64[ordinal]) {
        *out_receipt = receipt;
        return 0;
    }

    section_bytes = source_bytes + section->file_offset;
    receipt.valid = 1;
    receipt.source_admission_bound = 1;
    receipt.section_bound = 1;
    receipt.preamble_bound = 1;
    receipt.source_fnv1a64 = source_fnv;
    receipt.admission_ordinal = ordinal;
    receipt.section_table_index = (uint32_t)section->index;
    receipt.section_offset = section->file_offset;
    receipt.section_length = section->size;
    receipt.section_fnv1a64 = admission->section_fnv1a64[ordinal];
    receipt.preamble_offset = section->file_offset;
    receipt.preamble_length = NEXUS_V1_FONT256_S2D_SECTION_PREAMBLE_BYTES;
    receipt.preamble_fnv1a64 = fnv1a64(section_bytes, receipt.preamble_length);
    for (index = 0; index < section->size; ++index) {
        if (section_bytes[index] == 0U) {
            ++receipt.zero_byte_count;
        } else {
            ++receipt.nonzero_byte_count;
        }
    }
    receipt.post_preamble_word_count =
        (section->size - NEXUS_V1_FONT256_S2D_SECTION_PREAMBLE_BYTES) / 2U;
    for (index = 0; index < receipt.post_preamble_word_count; ++index) {
        if (be16(section_bytes + NEXUS_V1_FONT256_S2D_SECTION_PREAMBLE_BYTES +
                 (size_t)index * 2U) != (uint16_t)index) {
            break;
        }
        ++receipt.be16_ramp_prefix_word_count;
    }
    receipt.be16_ramp_full =
        receipt.be16_ramp_prefix_word_count == receipt.post_preamble_word_count &&
        receipt.post_preamble_word_count > 0U;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_font256_s2d_section_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSectionCorpusReceipt *out_receipt)
{
    Nexus_V1_Font256S2DSectionCorpusReceipt receipt;
    uint32_t ordinal;
    uint32_t expected_offset;
    uint64_t chain_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    if (!admission_usable(admission)) {
        *out_receipt = receipt;
        return 0;
    }
    for (ordinal = 0; ordinal < NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
         ++ordinal) {
        if (!nexus_v1_font256_s2d_populated_section_receipt_admit(
                source_bytes, source_size, admission, ordinal,
                &receipt.sections[ordinal])) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.capture_required = 1;
            *out_receipt = receipt;
            return 0;
        }
    }

    expected_offset = receipt.sections[0].section_offset;
    receipt.chain_offset = expected_offset;
    receipt.chain_length = 0U;
    for (ordinal = 0; ordinal < NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
         ++ordinal) {
        if (receipt.sections[ordinal].section_offset != expected_offset) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.capture_required = 1;
            *out_receipt = receipt;
            return 0;
        }
        expected_offset += receipt.sections[ordinal].section_length;
        receipt.chain_length += receipt.sections[ordinal].section_length;
    }
    chain_end = (uint64_t)receipt.chain_offset + (uint64_t)receipt.chain_length;
    if (chain_end > (uint64_t)source_size) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.capture_required = 1;
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = 1;
    receipt.source_admission_bound = 1;
    receipt.all_sections_bound = 1;
    receipt.contiguous_chain_observed = 1;
    receipt.chain_covers_source_tail = chain_end == (uint64_t)source_size;
    receipt.source_fnv1a64 = receipt.sections[0].source_fnv1a64;
    receipt.populated_section_count = NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
    receipt.chain_fnv1a64 =
        fnv1a64(source_bytes + receipt.chain_offset, receipt.chain_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_font256_s2d_section_corpus_span_iterator_init(
    Nexus_V1_Font256S2DSectionCorpusSpanIterator *iterator,
    const Nexus_V1_Font256S2DSectionCorpusReceipt *receipt)
{
    uint32_t ordinal;
    if (!iterator || !receipt || !receipt->valid || !receipt->capture_required ||
        !receipt->all_sections_bound || receipt->glyph_layout_proven ||
        receipt->palette_proven || receipt->pixel_decode_permitted ||
        receipt->draw_permitted ||
        receipt->populated_section_count !=
            NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT) return -1;
    for (ordinal = 0; ordinal < NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
         ++ordinal) {
        if (!receipt->sections[ordinal].valid ||
            !receipt->sections[ordinal].section_length ||
            !receipt->sections[ordinal].section_fnv1a64) return -1;
    }
    memset(iterator, 0, sizeof(*iterator));
    iterator->receipt = *receipt;
    return 0;
}

int nexus_v1_font256_s2d_section_corpus_span_iterator_next(
    Nexus_V1_Font256S2DSectionCorpusSpanIterator *iterator,
    Nexus_V1_Font256S2DSectionCorpusSpan *out_span)
{
    const Nexus_V1_Font256S2DPopulatedSectionReceipt *section;
    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted >= NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT) return 0;
    section = &iterator->receipt.sections[iterator->emitted];
    out_span->source_offset = section->section_offset;
    out_span->source_length = section->section_length;
    out_span->source_fnv1a64 = section->section_fnv1a64;
    ++iterator->emitted;
    return 1;
}
