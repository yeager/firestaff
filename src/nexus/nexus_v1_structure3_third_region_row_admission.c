#include "nexus_v1_structure3_third_region_row_admission.h"

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

int nexus_v1_structure3_third_region_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    uint32_t row_ordinal,
    Nexus_V1_Structure3ThirdRegionRowAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure3ThirdRegionRowAdmissionReceipt receipt;
    const uint8_t *entry_bytes;
    const uint8_t *row;
    uint64_t package_fnv1a64;
    uint64_t row_offset;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !entry || !entry->valid ||
        !entry->target_bound || !entry->fixed_header_bound ||
        !entry->counted_regions_bound || !entry->no_draw_only || !identity->valid ||
        identity->level_index != entry->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        row_ordinal >= entry->second_region_count ||
        entry->third_region_offset != entry->second_region_offset +
            entry->second_region_length ||
        entry->third_region_length != (uint32_t)entry->second_region_count * 12U ||
        entry->target_offset > (uint32_t)dgn_size ||
        entry->target_length > (uint32_t)dgn_size - entry->target_offset ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    entry_bytes = dgn_data + entry->target_offset;
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != entry->package_fnv1a64 ||
        fnv1a64(entry_bytes, entry->target_length) != entry->target_fnv1a64 ||
        fnv1a64(entry_bytes + entry->third_region_offset, entry->third_region_length) !=
            entry->third_region_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    row_offset = (uint64_t)entry->target_offset + entry->third_region_offset +
        (uint64_t)row_ordinal * 12U;
    if (row_offset > (uint64_t)dgn_size || 12U > (uint64_t)dgn_size - row_offset) {
        *out_receipt = receipt;
        return 0;
    }
    row = dgn_data + row_offset;
    receipt.valid = receipt.entry_bound = receipt.third_region_bound = 1;
    receipt.ordinal_row_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.entry_offset = entry->target_offset;
    receipt.entry_fnv1a64 = entry->target_fnv1a64;
    receipt.row_ordinal = row_ordinal;
    receipt.row_offset = (uint32_t)row_offset;
    receipt.row_fnv1a64 = fnv1a64(row, 12U);
    memcpy(receipt.raw_bytes, row, sizeof(receipt.raw_bytes));
    *out_receipt = receipt;
    return 1;
}
