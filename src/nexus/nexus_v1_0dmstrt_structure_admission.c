#include "nexus_v1_0dmstrt_structure_admission.h"

#include <string.h>

/* Canonical 0DMSTRT.BIN fixup-table values, derived from the
 * SHA-256-attested retail asset. Each entry is a BE16 tag 0x0601
 * followed by one of these BE16 values. These are provenance bindings
 * of the same class as the TITLE.BIN CNFD constants: they assign no
 * instruction, code, data, relocation, address, or execution
 * semantics. */
static const uint16_t k_head_table_value[NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT] = {
    0x363cU, 0x0074U, 0x0894U, 0x0898U, 0x0888U, 0x088cU, 0x0890U
};
static const uint16_t k_table_a_value[NEXUS_V1_0DMSTRT_TABLE_A_COUNT] = {
    0x6e30U, 0x7098U, 0x715cU, 0x71bcU
};
static const uint16_t k_table_b_value[NEXUS_V1_0DMSTRT_TABLE_B_COUNT] = {
    0x5ff8U, 0x5ffcU, 0x6092U, 0x61f2U, 0x61f6U, 0x621cU,
    0x6220U, 0x6224U, 0x624aU, 0x624eU, 0x6252U, 0x6290U
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

static uint32_t nonzero_count(const uint8_t *bytes, size_t size)
{
    uint32_t count = 0U;
    size_t index;
    for (index = 0; index < size; ++index) {
        if (bytes[index] != 0U) ++count;
    }
    return count;
}

static int fixup_table_match(const uint8_t *source, uint32_t offset,
    const uint16_t *values, uint32_t count)
{
    uint32_t index;
    for (index = 0U; index < count; ++index) {
        if (be16(source + offset + index * 4U) != NEXUS_V1_0DMSTRT_FIXUP_TAG ||
            be16(source + offset + index * 4U + 2U) != values[index]) {
            return 0;
        }
    }
    return 1;
}

/* The 8 raw partition spans in file order: region A, gap 1, region B,
 * gap 2, tail descriptor, fixup A, gap 3, fixup B. Their lengths sum
 * to the source size with zero gap. */
static const uint32_t k_span_offset[NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT] = {
    NEXUS_V1_0DMSTRT_REGION_A_OFFSET,
    NEXUS_V1_0DMSTRT_GAP1_OFFSET,
    NEXUS_V1_0DMSTRT_REGION_B_OFFSET,
    NEXUS_V1_0DMSTRT_GAP2_OFFSET,
    NEXUS_V1_0DMSTRT_TAIL_OFFSET,
    NEXUS_V1_0DMSTRT_TABLE_A_OFFSET,
    NEXUS_V1_0DMSTRT_GAP3_OFFSET,
    NEXUS_V1_0DMSTRT_TABLE_B_OFFSET
};
static const uint32_t k_span_length[NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT] = {
    NEXUS_V1_0DMSTRT_REGION_A_END - NEXUS_V1_0DMSTRT_REGION_A_OFFSET,
    NEXUS_V1_0DMSTRT_GAP1_LENGTH,
    NEXUS_V1_0DMSTRT_REGION_B_END - NEXUS_V1_0DMSTRT_REGION_B_OFFSET,
    NEXUS_V1_0DMSTRT_GAP2_LENGTH,
    NEXUS_V1_0DMSTRT_TABLE_A_OFFSET - NEXUS_V1_0DMSTRT_TAIL_OFFSET,
    NEXUS_V1_0DMSTRT_TABLE_A_COUNT * 4U,
    NEXUS_V1_0DMSTRT_GAP3_LENGTH,
    NEXUS_V1_0DMSTRT_TABLE_B_COUNT * 4U
};

int nexus_v1_0dmstrt_structure_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_0DmstrtSourceIdentity *identity,
    Nexus_V1_0DmstrtStructureReceipt *out_receipt)
{
    Nexus_V1_0DmstrtStructureReceipt receipt;
    uint32_t stamp_index;
    uint32_t ff_count;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !identity ||
        source_size != NEXUS_V1_0DMSTRT_BYTES ||
        !identity->sha256_verified || !identity->sha256_hex ||
        strcmp(identity->sha256_hex, NEXUS_V1_0DMSTRT_SHA256) != 0 ||
        !identity->source_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_fnv1a64 = fnv1a64(source_bytes, source_size);
    if (receipt.source_fnv1a64 != identity->source_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.source_identity_bound = 1;

    /* Exact partition arithmetic: the eight region spans cover the
     * source with zero gap, and fixup table B ends at the source end. */
    if (NEXUS_V1_0DMSTRT_REGION_A_END != NEXUS_V1_0DMSTRT_GAP1_OFFSET ||
        NEXUS_V1_0DMSTRT_GAP1_OFFSET + NEXUS_V1_0DMSTRT_GAP1_LENGTH !=
            NEXUS_V1_0DMSTRT_REGION_B_OFFSET ||
        NEXUS_V1_0DMSTRT_REGION_B_END != NEXUS_V1_0DMSTRT_GAP2_OFFSET ||
        NEXUS_V1_0DMSTRT_GAP2_OFFSET + NEXUS_V1_0DMSTRT_GAP2_LENGTH !=
            NEXUS_V1_0DMSTRT_TAIL_OFFSET ||
        NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET + NEXUS_V1_0DMSTRT_ISO_STUB_LENGTH !=
            NEXUS_V1_0DMSTRT_TABLE_A_OFFSET ||
        NEXUS_V1_0DMSTRT_TABLE_A_OFFSET + NEXUS_V1_0DMSTRT_TABLE_A_COUNT * 4U !=
            NEXUS_V1_0DMSTRT_GAP3_OFFSET ||
        NEXUS_V1_0DMSTRT_GAP3_OFFSET + NEXUS_V1_0DMSTRT_GAP3_LENGTH !=
            NEXUS_V1_0DMSTRT_TABLE_B_OFFSET ||
        NEXUS_V1_0DMSTRT_TABLE_B_OFFSET + NEXUS_V1_0DMSTRT_TABLE_B_COUNT * 4U !=
            NEXUS_V1_0DMSTRT_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.size_arithmetic_bound = 1;
    receipt.regions_cover_source = 1;

    receipt.region_a_nonzero = nonzero_count(
        source_bytes + NEXUS_V1_0DMSTRT_REGION_A_OFFSET,
        NEXUS_V1_0DMSTRT_REGION_A_END - NEXUS_V1_0DMSTRT_REGION_A_OFFSET);
    if (receipt.region_a_nonzero != NEXUS_V1_0DMSTRT_REGION_A_NONZERO) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.region_a_fnv1a64 = fnv1a64(
        source_bytes + NEXUS_V1_0DMSTRT_REGION_A_OFFSET,
        NEXUS_V1_0DMSTRT_REGION_A_END - NEXUS_V1_0DMSTRT_REGION_A_OFFSET);
    receipt.region_a_bound = 1;

    if (nonzero_count(source_bytes + NEXUS_V1_0DMSTRT_GAP1_OFFSET,
            NEXUS_V1_0DMSTRT_GAP1_LENGTH) != 0U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.gap1_zero_bound = 1;

    receipt.region_b_nonzero = nonzero_count(
        source_bytes + NEXUS_V1_0DMSTRT_REGION_B_OFFSET,
        NEXUS_V1_0DMSTRT_REGION_B_END - NEXUS_V1_0DMSTRT_REGION_B_OFFSET);
    if (receipt.region_b_nonzero != NEXUS_V1_0DMSTRT_REGION_B_NONZERO) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.region_b_fnv1a64 = fnv1a64(
        source_bytes + NEXUS_V1_0DMSTRT_REGION_B_OFFSET,
        NEXUS_V1_0DMSTRT_REGION_B_END - NEXUS_V1_0DMSTRT_REGION_B_OFFSET);
    receipt.region_b_bound = 1;

    if (nonzero_count(source_bytes + NEXUS_V1_0DMSTRT_GAP2_OFFSET,
            NEXUS_V1_0DMSTRT_GAP2_LENGTH) != 0U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.gap2_zero_bound = 1;

    /* Tail descriptor: 0xff separator, 31-byte printable stamp leading
     * with the "GFS_SBL" class tag, NUL terminator, byte 0x01. */
    if (source_bytes[NEXUS_V1_0DMSTRT_TAIL_OFFSET] !=
            NEXUS_V1_0DMSTRT_TAIL_SEPARATOR ||
        memcmp(source_bytes + NEXUS_V1_0DMSTRT_STAMP_OFFSET, "GFS_SBL",
            NEXUS_V1_0DMSTRT_STAMP_TAG_BYTES) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    for (stamp_index = 0U; stamp_index < NEXUS_V1_0DMSTRT_STAMP_LENGTH;
            ++stamp_index) {
        uint8_t byte = source_bytes[NEXUS_V1_0DMSTRT_STAMP_OFFSET + stamp_index];
        if (byte < 0x20U || byte > 0x7eU) {
            *out_receipt = receipt;
            return 0;
        }
    }
    if (source_bytes[NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET] != 0x00U ||
        source_bytes[NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET + 1U] != 0x01U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.tail_descriptor_bound = 1;

    /* ISO-style stub: "CD001" standard identifier plus the "." and
     * ".." directory-id bytes and the observed 0xff population. */
    if (memcmp(source_bytes + NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET, "CD001",
            5U) != 0 ||
        source_bytes[NEXUS_V1_0DMSTRT_ISO_DOT_OFFSET] != 0x2eU ||
        source_bytes[NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET] != 0x2eU ||
        source_bytes[NEXUS_V1_0DMSTRT_ISO_DOTDOT_OFFSET + 1U] != 0x2eU) {
        *out_receipt = receipt;
        return 0;
    }
    ff_count = 0U;
    for (stamp_index = 0U;
            stamp_index < NEXUS_V1_0DMSTRT_ISO_STUB_OFFSET +
                NEXUS_V1_0DMSTRT_ISO_STUB_LENGTH -
                (NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 5U);
            ++stamp_index) {
        if (source_bytes[NEXUS_V1_0DMSTRT_ISO_MAGIC_OFFSET + 5U +
                stamp_index] == 0xffU) {
            ++ff_count;
        }
    }
    if (ff_count != NEXUS_V1_0DMSTRT_ISO_STUB_FF_COUNT) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.iso_stub_bound = 1;

    if (be16(source_bytes + NEXUS_V1_0DMSTRT_HEAD_SENTINEL_OFFSET) !=
            NEXUS_V1_0DMSTRT_HEAD_SENTINEL ||
        !fixup_table_match(source_bytes, NEXUS_V1_0DMSTRT_HEAD_TABLE_OFFSET,
            k_head_table_value, NEXUS_V1_0DMSTRT_HEAD_TABLE_COUNT)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.head_table_bound = 1;

    if (!fixup_table_match(source_bytes, NEXUS_V1_0DMSTRT_TABLE_A_OFFSET,
            k_table_a_value, NEXUS_V1_0DMSTRT_TABLE_A_COUNT)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.fixup_table_a_bound = 1;

    if (nonzero_count(source_bytes + NEXUS_V1_0DMSTRT_GAP3_OFFSET,
            NEXUS_V1_0DMSTRT_GAP3_LENGTH) != 0U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.gap3_zero_bound = 1;

    if (!fixup_table_match(source_bytes, NEXUS_V1_0DMSTRT_TABLE_B_OFFSET,
            k_table_b_value, NEXUS_V1_0DMSTRT_TABLE_B_COUNT)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.fixup_table_b_bound = 1;

    {
        uint32_t span_index;
        for (span_index = 0U;
                span_index < NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT;
                ++span_index) {
            receipt.region_span_fnv1a64[span_index] = fnv1a64(
                source_bytes + k_span_offset[span_index],
                k_span_length[span_index]);
        }
    }

    receipt.valid = 1;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_0dmstrt_region_span_iterator_init(
    Nexus_V1_0DmstrtRegionSpanIterator *iterator,
    const Nexus_V1_0DmstrtStructureReceipt *receipt)
{
    if (!iterator || !receipt || !receipt->valid) return 0;
    iterator->receipt = *receipt;
    iterator->emitted = 0U;
    return 1;
}

int nexus_v1_0dmstrt_region_span_iterator_next(
    Nexus_V1_0DmstrtRegionSpanIterator *iterator,
    Nexus_V1_0DmstrtRegionSpan *out_span)
{
    Nexus_V1_0DmstrtRegionSpan span;

    if (!iterator || !out_span || !iterator->receipt.valid) return -1;
    if (iterator->emitted >= NEXUS_V1_0DMSTRT_REGION_SPAN_COUNT) return 0;
    span.source_offset = k_span_offset[iterator->emitted];
    span.source_length = k_span_length[iterator->emitted];
    span.source_fnv1a64 =
        iterator->receipt.region_span_fnv1a64[iterator->emitted];
    ++iterator->emitted;
    *out_span = span;
    return 1;
}
