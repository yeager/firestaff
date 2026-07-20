#include "nexus_v1_font256_s2d_subrecord_grammar.h"

#include <string.h>

/* Canonical FONT256.S2D subrecord words, derived from the SHA-256-attested
 * retail asset. These are provenance bindings of the same class as the
 * existing section corpus composition measurements: preamble words, ramp
 * arithmetic, and record word values. They assign no text, glyph, palette,
 * record, encoding, or pixel semantics. */
static const uint16_t k_section0_preamble_word[
    NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS] = {
    0x0010U, 0x0000U, 0x4000U, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU
};
static const uint16_t k_section4_record0_word[
    NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x0000U, 0x0100U, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU, 0xffffU
};
static const uint16_t k_section4_record1_word[
    NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x8000U, 0x8000U, 0x8000U, 0xfffeU, 0x8000U, 0x8000U, 0x8000U, 0x8000U
};
static const uint16_t k_section4_record2_word[
    NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS] = {
    0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0x8000U, 0xfc20U
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

static int record_words_match(const uint8_t *record, const uint16_t *words)
{
    uint32_t index;
    for (index = 0U; index < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS;
            ++index) {
        if (be16(record + index * 2U) != words[index]) return 0;
    }
    return 1;
}

int nexus_v1_font256_s2d_subrecord_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    uint32_t ordinal,
    Nexus_V1_Font256S2DSubrecordReceipt *out_receipt)
{
    Nexus_V1_Font256S2DSubrecordReceipt receipt;
    Nexus_V1_Font256S2DPopulatedSectionReceipt base;
    const uint8_t *section;
    uint32_t index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    if (!nexus_v1_font256_s2d_populated_section_receipt_admit(
            source_bytes, source_size, admission, ordinal, &base)) {
        *out_receipt = receipt;
        return 0;
    }
    section = source_bytes + base.section_offset;
    receipt.valid = 1;
    receipt.source_admission_bound = 1;
    receipt.section_bound = 1;
    receipt.source_fnv1a64 = base.source_fnv1a64;
    receipt.admission_ordinal = ordinal;
    receipt.section_table_index = base.section_table_index;
    receipt.section_offset = base.section_offset;
    receipt.section_length = base.section_length;
    receipt.section_fnv1a64 = base.section_fnv1a64;

    if (ordinal == 0U) {
        /* Preamble of 8 canonical words, then the dual step-2 ramp. */
        if (base.section_length != 16U +
                NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS * 2U) {
            memset(&receipt, 0, sizeof(receipt));
            receipt.capture_required = 1;
            *out_receipt = receipt;
            return 0;
        }
        for (index = 0U;
                index < NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS;
                ++index) {
            if (be16(section + index * 2U) != k_section0_preamble_word[index]) {
                *out_receipt = receipt;
                return 0;
            }
        }
        for (index = 0U; index < NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS;
                ++index) {
            uint16_t expected = (uint16_t)(2U *
                (index & (NEXUS_V1_FONT256_S2D_SECTION0_RAMP_HALF_WORDS - 1U)));
            if (be16(section + 16U + (size_t)index * 2U) != expected) {
                *out_receipt = receipt;
                return 0;
            }
        }
        receipt.preamble_word_count = NEXUS_V1_FONT256_S2D_SECTION0_PREAMBLE_WORDS;
        receipt.ramp_word_count = NEXUS_V1_FONT256_S2D_SECTION0_RAMP_WORDS;
        receipt.ramp_half_word_count =
            NEXUS_V1_FONT256_S2D_SECTION0_RAMP_HALF_WORDS;
        receipt.subrecord_grammar_bound = 1;
    } else if (ordinal == 1U) {
        /* Negative grammar: opaque 16-byte block population only. */
        uint32_t populated = 0U;
        uint32_t block;
        if (base.section_length !=
                NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT *
                    NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES) {
            *out_receipt = receipt;
            return 0;
        }
        for (block = 0U; block < NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT;
                ++block) {
            const uint8_t *bytes =
                section + block * NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES;
            uint32_t byte_index;
            for (byte_index = 0U;
                    byte_index < NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES;
                    ++byte_index) {
                if (bytes[byte_index] != 0U) {
                    ++populated;
                    break;
                }
            }
        }
        if (populated != NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT) {
            *out_receipt = receipt;
            return 0;
        }
        receipt.block_count = NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT;
        receipt.populated_block_count = populated;
        receipt.subrecord_grammar_bound = 0;
    } else if (ordinal == 2U) {
        /* 33 sixteen-byte records: 3 canonical heads, 30 base records. */
        uint32_t base_records = 0U;
        if (base.section_length != NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT *
                NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES) {
            *out_receipt = receipt;
            return 0;
        }
        if (!record_words_match(section, k_section4_record0_word) ||
            !record_words_match(
                section + NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES,
                k_section4_record1_word) ||
            !record_words_match(
                section + 2U * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES,
                k_section4_record2_word)) {
            *out_receipt = receipt;
            return 0;
        }
        for (index = NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT -
                NEXUS_V1_FONT256_S2D_SECTION4_BASE_RECORD_COUNT;
                index < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT; ++index) {
            const uint8_t *record =
                section + index * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES;
            uint32_t word;
            for (word = 0U;
                    word < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_WORDS;
                    ++word) {
                if (be16(record + word * 2U) !=
                        NEXUS_V1_FONT256_S2D_SECTION4_BASE_WORD) {
                    *out_receipt = receipt;
                    return 0;
                }
            }
            ++base_records;
        }
        receipt.record_count = NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT;
        receipt.base_record_count = base_records;
        receipt.subrecord_grammar_bound = 1;
    } else if (ordinal == 3U) {
        /* The whole section is zero. */
        for (index = 0U; index < base.section_length; ++index) {
            if (section[index] != 0U) {
                *out_receipt = receipt;
                return 0;
            }
        }
        receipt.section_all_zero = 1;
        receipt.subrecord_grammar_bound = 1;
    } else {
        memset(&receipt, 0, sizeof(receipt));
        receipt.capture_required = 1;
        *out_receipt = receipt;
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_font256_s2d_subrecord_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSubrecordCorpusReceipt *out_receipt)
{
    Nexus_V1_Font256S2DSubrecordCorpusReceipt receipt;
    uint32_t ordinal;
    uint32_t span;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.capture_required = 1;
    for (ordinal = 0U; ordinal < NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;
            ++ordinal) {
        if (!nexus_v1_font256_s2d_subrecord_admit(
                source_bytes, source_size, admission, ordinal,
                &receipt.sections[ordinal])) {
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.valid = 1;
    receipt.source_admission_bound = 1;
    receipt.all_sections_bound = 1;
    receipt.section0_grammar_bound = receipt.sections[0].subrecord_grammar_bound;
    receipt.section2_grammar_negative =
        !receipt.sections[1].subrecord_grammar_bound &&
        receipt.sections[1].populated_block_count ==
            NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT;
    receipt.section4_grammar_bound = receipt.sections[2].subrecord_grammar_bound;
    receipt.section6_zero_bound = receipt.sections[3].section_all_zero;
    receipt.source_fnv1a64 = receipt.sections[0].source_fnv1a64;
    receipt.populated_section_count = NEXUS_V1_FONT256_S2D_POPULATED_SECTION_COUNT;

    /* Live digests of the 38 raw subrecord spans in file order. */
    span = 0U;
    receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
        source_bytes + receipt.sections[0].section_offset, 16U);
    receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
        source_bytes + receipt.sections[0].section_offset + 16U, 4096U);
    receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
        source_bytes + receipt.sections[0].section_offset + 4112U, 4096U);
    receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
        source_bytes + receipt.sections[1].section_offset,
        receipt.sections[1].section_length);
    for (ordinal = 0U; ordinal < NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT;
            ++ordinal) {
        receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
            source_bytes + receipt.sections[2].section_offset +
                ordinal * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES,
            NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES);
    }
    receipt.subrecord_span_fnv1a64[span++] = fnv1a64(
        source_bytes + receipt.sections[3].section_offset,
        receipt.sections[3].section_length);
    if (span != NEXUS_V1_FONT256_S2D_SUBRECORD_SPAN_COUNT) {
        memset(&receipt, 0, sizeof(receipt));
        receipt.capture_required = 1;
        *out_receipt = receipt;
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_font256_s2d_subrecord_span_iterator_init(
    Nexus_V1_Font256S2DSubrecordSpanIterator *iterator,
    const Nexus_V1_Font256S2DSubrecordCorpusReceipt *receipt)
{
    if (!iterator || !receipt || !receipt->valid) return 0;
    iterator->receipt = *receipt;
    iterator->emitted = 0U;
    return 1;
}

int nexus_v1_font256_s2d_subrecord_span_iterator_next(
    Nexus_V1_Font256S2DSubrecordSpanIterator *iterator,
    Nexus_V1_Font256S2DSubrecordSpan *out_span)
{
    Nexus_V1_Font256S2DSubrecordSpan span;
    uint32_t emitted;
    uint32_t record;

    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted >= NEXUS_V1_FONT256_S2D_SUBRECORD_SPAN_COUNT) return 0;
    emitted = iterator->emitted;
    if (emitted == 0U) {
        span.source_offset = iterator->receipt.sections[0].section_offset;
        span.source_length = 16U;
    } else if (emitted == 1U) {
        span.source_offset = iterator->receipt.sections[0].section_offset + 16U;
        span.source_length = 4096U;
    } else if (emitted == 2U) {
        span.source_offset =
            iterator->receipt.sections[0].section_offset + 4112U;
        span.source_length = 4096U;
    } else if (emitted == 3U) {
        span.source_offset = iterator->receipt.sections[1].section_offset;
        span.source_length = iterator->receipt.sections[1].section_length;
    } else if (emitted < 4U + NEXUS_V1_FONT256_S2D_SECTION4_RECORD_COUNT) {
        record = emitted - 4U;
        span.source_offset = iterator->receipt.sections[2].section_offset +
            record * NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES;
        span.source_length = NEXUS_V1_FONT256_S2D_SECTION4_RECORD_BYTES;
    } else {
        span.source_offset = iterator->receipt.sections[3].section_offset;
        span.source_length = iterator->receipt.sections[3].section_length;
    }
    span.source_fnv1a64 = iterator->receipt.subrecord_span_fnv1a64[emitted];
    ++iterator->emitted;
    *out_span = span;
    return 1;
}
