#include "nexus_v1_title_res_corpus_receipt.h"

#include <string.h>

/* Canonical TITLE.BIN table layout, derived from the SHA-256-attested retail
 * asset. These are provenance bindings of the same class as the existing
 * WARNING.BIN descriptor constants: entry classes, class-local ids, and
 * record offsets. Record lengths are derived from consecutive offsets; the
 * chain ends exactly at the end of the source. No offset, id, or length is
 * assigned image, palette, subrecord, or presentation semantics. */
static const uint8_t k_entry_class[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,
    2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3
};

static const uint32_t k_entry_id[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,
    0,1,2,3,
    0,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
    24,25,26,27,28,29,30,31,32
};

static const uint32_t k_entry_offset[NEXUS_V1_TITLE_RES_ENTRY_COUNT] = {
    0x002e8U, 0x00418U, 0x00548U, 0x00678U, 0x007a8U, 0x00978U,
    0x00b48U, 0x00c98U, 0x00de8U, 0x00f38U, 0x01088U, 0x011d8U,
    0x01328U, 0x01478U, 0x015c8U, 0x01718U, 0x01868U, 0x019b8U,
    0x01b08U, 0x01c58U, 0x01da8U, 0x01ef8U, 0x02318U, 0x0a0a8U,
    0x0b438U, 0x0d068U, 0x0e278U, 0x16eecU, 0x1709cU, 0x172c4U,
    0x17390U, 0x1745cU, 0x1756cU, 0x1767cU, 0x17850U, 0x17a5cU,
    0x17c0cU, 0x17dbcU, 0x17f3cU, 0x180bcU, 0x183c4U, 0x18734U,
    0x188d0U, 0x18a6cU, 0x18ba0U, 0x18cd4U, 0x19104U, 0x195b4U,
    0x19680U, 0x1974cU, 0x1985cU, 0x1996cU, 0x19bf4U, 0x19eccU,
    0x19f98U, 0x1a064U, 0x1a174U, 0x1a284U, 0x1a3e0U, 0x1a564U
};

static const char k_class_magic[NEXUS_V1_TITLE_RES_ENTRY_COUNT][4] = {
    {'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},
    {'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},
    {'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},
    {'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},
    {'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},{'D','G','T','2'},
    {'D','G','T','2'},{'D','G','T','2'},
    {'T','I','T','L'},{'T','I','T','L'},{'T','I','T','L'},{'T','I','T','L'},
    {'M','A','P','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},{'C','N','F','D'},
    {'C','N','F','D'}
};

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

static int title_bin_sha256_is_verified(const char *sha256_hex)
{
    return sha256_hex &&
        (strcmp(sha256_hex, NEXUS_V1_TITLE_BIN_SHA256) == 0 ||
         strcmp(sha256_hex, NEXUS_V1_TITLE_BIN_ENGLISH_SHA256) == 0);
}

static int directory_recheck(const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity, uint64_t *out_source_fnv)
{
    uint64_t source_fnv;
    uint32_t index;
    if (!source_bytes || !identity || source_size != NEXUS_V1_TITLE_BIN_BYTES ||
        !identity->sha256_verified ||
        !title_bin_sha256_is_verified(identity->sha256_hex) ||
        !identity->source_fnv1a64) {
        return 0;
    }
    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != identity->source_fnv1a64 || source_size < 12U ||
        memcmp(source_bytes, "RES*", 4) != 0 ||
        be32(source_bytes + 4U) != source_size ||
        be16(source_bytes + 8U) != NEXUS_V1_TITLE_RES_ENTRY_COUNT) {
        return 0;
    }
    for (index = 0U; index < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++index) {
        const uint8_t *entry = source_bytes + 12U +
            (size_t)index * NEXUS_V1_TITLE_RES_ENTRY_BYTES;
        if (memcmp(entry, k_class_magic[index], 4) != 0 ||
            be32(entry + 4U) != k_entry_id[index] ||
            be32(entry + 8U) != k_entry_offset[index]) {
            return 0;
        }
    }
    if (out_source_fnv) *out_source_fnv = source_fnv;
    return 1;
}

int nexus_v1_title_res_record_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    uint32_t entry_index,
    Nexus_V1_TitleResRecordReceipt *out_receipt)
{
    Nexus_V1_TitleResRecordReceipt receipt;
    const uint8_t *record;
    uint64_t source_fnv;
    uint32_t record_offset;
    uint32_t record_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!directory_recheck(source_bytes, source_size, identity, &source_fnv) ||
        entry_index >= NEXUS_V1_TITLE_RES_ENTRY_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    record_offset = k_entry_offset[entry_index];
    record_end = entry_index + 1U < NEXUS_V1_TITLE_RES_ENTRY_COUNT ?
        k_entry_offset[entry_index + 1U] : (uint32_t)source_size;
    if (record_offset < 12U +
            NEXUS_V1_TITLE_RES_ENTRY_COUNT * NEXUS_V1_TITLE_RES_ENTRY_BYTES ||
        record_end <= record_offset || record_end > source_size ||
        record_end - record_offset < NEXUS_V1_TITLE_RES_HEAD_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    record = source_bytes + record_offset;
    if (memcmp(record, k_class_magic[entry_index], 4) != 0 ||
        be32(record + 4U) != k_entry_id[entry_index]) {
        *out_receipt = receipt;
        return 0;
    }
    if ((k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_DGT2 ||
         k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_CNFD) &&
        record[8] != 0x70U) {
        *out_receipt = receipt;
        return 0;
    }
    if (k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_TITL &&
        record[8] != 0x50U) {
        *out_receipt = receipt;
        return 0;
    }
    if (k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_MAPD &&
        memcmp(record + 8U, "TIBG", 4) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.source_identity_bound = receipt.res_directory_bound =
        receipt.entry_bound = receipt.record_head_bound =
        receipt.record_span_bound = 1;
    receipt.pp_tag_observed =
        k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_DGT2 ||
        k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_CNFD;
    receipt.PP_tag_observed =
        k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_TITL;
    receipt.tibg_tag_observed =
        k_entry_class[entry_index] == NEXUS_V1_TITLE_RES_CLASS_MAPD;
    receipt.source_fnv1a64 = source_fnv;
    receipt.entry_index = entry_index;
    receipt.entry_class = k_entry_class[entry_index];
    receipt.entry_id = k_entry_id[entry_index];
    receipt.record_offset = record_offset;
    receipt.record_length = record_end - record_offset;
    receipt.record_fnv1a64 =
        fnv1a64(record, receipt.record_length);
    receipt.entry_fnv1a64 = fnv1a64(
        source_bytes + 12U + (size_t)entry_index * NEXUS_V1_TITLE_RES_ENTRY_BYTES,
        NEXUS_V1_TITLE_RES_ENTRY_BYTES);
    receipt.record_head_fnv1a64 = fnv1a64(record, NEXUS_V1_TITLE_RES_HEAD_BYTES);
    receipt.record_inner_id = be32(record + 4U);
    receipt.head_tag_word = be16(record + 8U);
    receipt.head_word0 = be16(record + 10U);
    receipt.head_word1 = be16(record + 12U);
    receipt.head_word2 = be16(record + 14U);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_title_res_corpus_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_TitleResSourceIdentity *identity,
    Nexus_V1_TitleResCorpusReceipt *out_receipt)
{
    Nexus_V1_TitleResCorpusReceipt receipt;
    uint32_t index;
    uint64_t chain_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (index = 0U; index < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++index) {
        if (!nexus_v1_title_res_record_admit(source_bytes, source_size,
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
        switch (receipt.records[index].entry_class) {
        case NEXUS_V1_TITLE_RES_CLASS_DGT2: ++receipt.dgt2_count; break;
        case NEXUS_V1_TITLE_RES_CLASS_TITL: ++receipt.titl_count; break;
        case NEXUS_V1_TITLE_RES_CLASS_MAPD: ++receipt.mapd_count; break;
        default: ++receipt.cnfd_count; break;
        }
    }
    if (receipt.dgt2_count != NEXUS_V1_TITLE_RES_DGT2_COUNT ||
        receipt.titl_count != NEXUS_V1_TITLE_RES_TITL_COUNT ||
        receipt.mapd_count != NEXUS_V1_TITLE_RES_MAPD_COUNT ||
        receipt.cnfd_count != NEXUS_V1_TITLE_RES_CNFD_COUNT) {
        memset(&receipt, 0, sizeof(receipt));
        *out_receipt = receipt;
        return 0;
    }
    receipt.chain_offset = receipt.records[0].record_offset;
    for (index = 0U; index < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++index) {
        receipt.chain_length += receipt.records[index].record_length;
    }
    chain_end = (uint64_t)receipt.chain_offset + (uint64_t)receipt.chain_length;
    if (receipt.chain_offset != NEXUS_V1_TITLE_RES_FIRST_OFFSET ||
        chain_end > (uint64_t)source_size) {
        memset(&receipt, 0, sizeof(receipt));
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.source_identity_bound = receipt.res_directory_bound =
        receipt.all_records_bound = 1;
    receipt.contiguous_chain_observed = 1;
    receipt.chain_covers_source_tail = chain_end == (uint64_t)source_size;
    receipt.source_fnv1a64 = receipt.records[0].source_fnv1a64;
    receipt.entry_count = NEXUS_V1_TITLE_RES_ENTRY_COUNT;
    receipt.table_fnv1a64 = fnv1a64(source_bytes + 12U,
        NEXUS_V1_TITLE_RES_ENTRY_COUNT * NEXUS_V1_TITLE_RES_ENTRY_BYTES);
    receipt.chain_fnv1a64 =
        fnv1a64(source_bytes + receipt.chain_offset, receipt.chain_length);
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_title_res_span_iterator_init(
    Nexus_V1_TitleResSpanIterator *iterator,
    const Nexus_V1_TitleResCorpusReceipt *receipt)
{
    uint32_t index;
    if (!iterator || !receipt || !receipt->valid || !receipt->all_records_bound ||
        receipt->record_grammar_proven || receipt->palette_proven ||
        receipt->pixel_decode_permitted || receipt->draw_permitted ||
        receipt->entry_count != NEXUS_V1_TITLE_RES_ENTRY_COUNT) return -1;
    for (index = 0U; index < NEXUS_V1_TITLE_RES_ENTRY_COUNT; ++index) {
        if (!receipt->records[index].valid ||
            !receipt->records[index].record_length ||
            !receipt->records[index].record_fnv1a64) return -1;
    }
    memset(iterator, 0, sizeof(*iterator));
    iterator->receipt = *receipt;
    return 0;
}

int nexus_v1_title_res_span_iterator_next(
    Nexus_V1_TitleResSpanIterator *iterator,
    Nexus_V1_TitleResSpan *out_span)
{
    const Nexus_V1_TitleResRecordReceipt *record;
    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted >= NEXUS_V1_TITLE_RES_ENTRY_COUNT) return 0;
    record = &iterator->receipt.records[iterator->emitted];
    out_span->source_offset = record->record_offset;
    out_span->source_length = record->record_length;
    out_span->source_fnv1a64 = record->record_fnv1a64;
    ++iterator->emitted;
    return 1;
}
