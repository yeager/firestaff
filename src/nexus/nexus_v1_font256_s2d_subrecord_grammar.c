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

static int section2_is_english_revision(uint64_t source_fnv1a64)
{
    /* SHA-256 admission is performed by the caller. This FNV is the live
     * source binding for the admitted English revision, not a guessed name
     * or a host-generated replacement. */
    return source_fnv1a64 == UINT64_C(0x90c4ce611bd5f5fe);
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
        /* Negative grammar: exhaustive opaque composition inventory.
         * 742 populated of 969 canonical 16-byte blocks in exactly 52
         * populated runs from block 0 through block 968; byte alphabet
         * exactly {0x00, 0x03, 0x0f, 0xff} with canonical counts; the
         * lead block alone carries all sixteen 0xff bytes; every other
         * nonzero byte is below 0x10. */
        uint32_t populated = 0U;
        uint32_t runs = 0U;
        uint32_t first_populated = NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT;
        uint32_t last_populated = 0U;
        uint32_t zero_count = 0U;
        uint32_t b03_count = 0U;
        uint32_t b0f_count = 0U;
        uint32_t bff_count = 0U;
        const int english = section2_is_english_revision(base.source_fnv1a64);
        const uint32_t expected_populated = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_BLOCK_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT;
        const uint32_t expected_runs = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_RUN_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_RUN_COUNT;
        const uint32_t expected_zero = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_ZERO_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_BYTE_ZERO_COUNT;
        const uint32_t expected_03 = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_03_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_BYTE_03_COUNT;
        const uint32_t expected_0f = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_0F_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_BYTE_0F_COUNT;
        const uint32_t expected_ff = english ?
            NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_FF_COUNT :
            NEXUS_V1_FONT256_S2D_SECTION2_BYTE_FF_COUNT;
        int previous_populated = 0;
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
            int block_populated = 0;
            uint32_t byte_index;
            for (byte_index = 0U;
                    byte_index < NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES;
                    ++byte_index) {
                switch (bytes[byte_index]) {
                case 0x00U: ++zero_count; break;
                case 0x03U: ++b03_count; block_populated = 1; break;
                case 0x0fU: ++b0f_count; block_populated = 1; break;
                case 0xffU: ++bff_count; block_populated = 1; break;
                default:
                    /* Outside the canonical alphabet: no admission. */
                    *out_receipt = receipt;
                    return 0;
                }
            }
            if (block_populated) {
                ++populated;
                if (!previous_populated) ++runs;
                if (first_populated ==
                        NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT) {
                    first_populated = block;
                }
                last_populated = block;
            }
            previous_populated = block_populated;
        }
        if (populated != expected_populated || runs != expected_runs ||
            first_populated !=
                NEXUS_V1_FONT256_S2D_SECTION2_FIRST_POPULATED_BLOCK ||
            last_populated !=
                NEXUS_V1_FONT256_S2D_SECTION2_LAST_POPULATED_BLOCK ||
            zero_count != expected_zero || b03_count != expected_03 ||
            b0f_count != expected_0f || bff_count != expected_ff) {
            *out_receipt = receipt;
            return 0;
        }
        /* The lead block alone must carry all sixteen 0xff bytes. */
        for (index = 0U; index < NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_BYTES;
                ++index) {
            if (section[index] != 0xffU) {
                *out_receipt = receipt;
                return 0;
            }
        }
        receipt.block_count = NEXUS_V1_FONT256_S2D_SECTION2_BLOCK_COUNT;
        receipt.populated_block_count = populated;
        receipt.populated_run_count = runs;
        receipt.first_populated_block = first_populated;
        receipt.last_populated_block = last_populated;
        receipt.byte_zero_count = zero_count;
        receipt.byte_03_count = b03_count;
        receipt.byte_0f_count = b0f_count;
        receipt.byte_ff_count = bff_count;
        receipt.lead_block_all_ones = 1;
        receipt.nonlead_high_nibble_clear = 1;
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
    receipt.source_fnv1a64 = receipt.sections[0].source_fnv1a64;
    receipt.section0_grammar_bound = receipt.sections[0].subrecord_grammar_bound;
    receipt.section2_grammar_negative =
        !receipt.sections[1].subrecord_grammar_bound &&
        receipt.sections[1].populated_block_count ==
            (section2_is_english_revision(receipt.source_fnv1a64) ?
                NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_BLOCK_COUNT :
                NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_BLOCK_COUNT);
    receipt.section2_composition_bound =
        receipt.sections[1].valid &&
        !receipt.sections[1].subrecord_grammar_bound &&
        receipt.sections[1].lead_block_all_ones &&
        receipt.sections[1].nonlead_high_nibble_clear &&
        receipt.sections[1].populated_run_count ==
            (section2_is_english_revision(receipt.source_fnv1a64) ?
                NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_POPULATED_RUN_COUNT :
                NEXUS_V1_FONT256_S2D_SECTION2_POPULATED_RUN_COUNT) &&
        receipt.sections[1].byte_03_count ==
            (section2_is_english_revision(receipt.source_fnv1a64) ?
                NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_03_COUNT :
                NEXUS_V1_FONT256_S2D_SECTION2_BYTE_03_COUNT) &&
        receipt.sections[1].byte_0f_count ==
            (section2_is_english_revision(receipt.source_fnv1a64) ?
                NEXUS_V1_FONT256_S2D_ENGLISH_SECTION2_BYTE_0F_COUNT :
                NEXUS_V1_FONT256_S2D_SECTION2_BYTE_0F_COUNT);
    receipt.section4_grammar_bound = receipt.sections[2].subrecord_grammar_bound;
    receipt.section6_zero_bound = receipt.sections[3].section_all_zero;
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
