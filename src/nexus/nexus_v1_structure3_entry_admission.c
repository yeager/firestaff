#include "nexus_v1_structure3_entry_admission.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;

    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static uint16_t read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
        ((uint32_t)bytes[2] << 8) | bytes[3];
}

int nexus_v1_structure3_entry_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3TargetAdmissionReceipt *target,
    Nexus_V1_Structure3EntryAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure3EntryAdmissionReceipt receipt;
    const uint8_t *entry;
    uint64_t package_fnv1a64;
    uint64_t first_length;
    uint64_t second_length;
    uint64_t third_length;
    uint64_t expected_second;
    uint64_t expected_third;
    uint64_t expected_end;
    uint32_t first_offset;
    uint32_t second_offset;
    uint32_t third_offset;
    uint16_t first_count;
    uint16_t second_count;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !target || !target->valid ||
        !target->field_bound || !target->directory_bound ||
        !target->target_span_bound || !target->no_draw_only || !identity->valid ||
        identity->level_index != target->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        target->target_length < NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES ||
        target->target_offset > (uint32_t)dgn_size ||
        target->target_length > (uint32_t)dgn_size - target->target_offset ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    entry = dgn_data + target->target_offset;
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != target->package_fnv1a64 ||
        fnv1a64(entry, target->target_length) != target->target_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    first_count = read_be16(entry + 4U);
    second_count = read_be16(entry + 6U);
    first_offset = read_be32(entry + 8U);
    second_offset = read_be32(entry + 16U);
    third_offset = read_be32(entry + 20U);
    first_length = (uint64_t)first_count * 12U;
    second_length = (uint64_t)second_count * 12U;
    third_length = (uint64_t)second_count * 12U;
    expected_second = (uint64_t)first_offset + first_length;
    expected_third = expected_second + second_length;
    expected_end = expected_third + third_length;
    if (first_offset != NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES ||
        expected_second > UINT32_MAX || expected_third > UINT32_MAX ||
        expected_end > target->target_length || second_offset != expected_second ||
        third_offset != expected_third) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.target_bound = receipt.fixed_header_bound = 1;
    receipt.counted_regions_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.target_offset = target->target_offset;
    receipt.target_length = target->target_length;
    receipt.target_fnv1a64 = target->target_fnv1a64;
    receipt.raw_tag = read_be32(entry);
    receipt.first_region_count = first_count;
    receipt.second_region_count = second_count;
    receipt.first_region_offset = first_offset;
    receipt.second_region_offset = second_offset;
    receipt.third_region_offset = third_offset;
    receipt.first_region_length = (uint32_t)first_length;
    receipt.second_region_length = (uint32_t)second_length;
    receipt.third_region_length = (uint32_t)third_length;
    receipt.header_fnv1a64 = fnv1a64(entry, NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES);
    receipt.first_region_fnv1a64 = first_length ?
        fnv1a64(entry + first_offset, (size_t)first_length) : 0U;
    receipt.second_region_fnv1a64 = second_length ?
        fnv1a64(entry + second_offset, (size_t)second_length) : 0U;
    receipt.third_region_fnv1a64 = third_length ?
        fnv1a64(entry + third_offset, (size_t)third_length) : 0U;
    *out_receipt = receipt;
    return 1;
}
