#include "nexus_v1_structure1f_item_admission.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0U; i < size; ++i) { hash ^= bytes[i]; hash *= UINT64_C(1099511628211); }
    return hash;
}

int nexus_v1_structure1f_item_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t record_index, Nexus_V1_Structure1FItemAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1FItemAdmissionReceipt receipt;
    const Nexus_V1_Structure1FDirectoryFamilyReceipt *items;
    uint64_t package_fnv1a64;
    uint64_t offset;
    const uint8_t *record;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || !directory || !directory->valid ||
        !directory->direct_identity_bound || !directory->parser_layout_bound ||
        !directory->family_directory_bound || !directory->no_draw_only ||
        !identity->valid || identity->level_index != directory->level_index ||
        identity->byte_count != (uint64_t)dgn_size || dgn_size <= 0 ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt; return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    items = &directory->families[NEXUS_V1_DGN_STRUCTURE1F_ITEMS];
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != directory->package_fnv1a64 ||
        items->source_tag != 0x10U || items->record_size != 8U ||
        !items->record_count || record_index >= items->record_count ||
        items->record_length != items->record_count * items->record_size ||
        items->record_offset > (uint32_t)dgn_size ||
        items->record_length > (uint32_t)dgn_size - items->record_offset ||
        fnv1a64(dgn_data + items->record_offset, items->record_length) !=
            items->record_fnv1a64) {
        *out_receipt = receipt; return 0;
    }
    offset = (uint64_t)items->record_offset + (uint64_t)record_index * 8U;
    if (offset > (uint64_t)dgn_size || 8U > (uint64_t)dgn_size - offset) {
        *out_receipt = receipt; return 0;
    }
    record = dgn_data + offset;
    if (record[0] != 0x10U) { *out_receipt = receipt; return 0; }
    receipt.valid = receipt.directory_bound = receipt.item_record_bound = 1;
    receipt.coordinate_pair_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.record_index = record_index;
    receipt.record_offset = (uint32_t)offset;
    receipt.record_fnv1a64 = fnv1a64(record, 8U);
    receipt.source_tag = record[0];
    receipt.x = record[1];
    receipt.y = record[2];
    receipt.cell_ordinal = (uint16_t)((uint16_t)record[2] * 64U + record[1]);
    memcpy(receipt.opaque_tail, record + 3U, sizeof(receipt.opaque_tail));
    *out_receipt = receipt;
    return 1;
}
