#include "nexus_v1_structure1f_alcove_structure3_row_admission.h"

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

int nexus_v1_structure1f_alcove_structure3_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FAlcoveAdmissionReceipt *alcove,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    const Nexus_V1_Structure3SecondRegionRowAdmissionReceipt *row,
    Nexus_V1_Structure1FAlcoveStructure3RowAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1FAlcoveStructure3RowAdmissionReceipt receipt;
    uint64_t package_fnv1a64;
    uint64_t expected_row_offset;
    const uint8_t *alcove_bytes;
    const uint8_t *row_bytes;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !alcove || !entry || !face || !row ||
        !identity->valid || !alcove->valid || !alcove->directory_bound ||
        !alcove->alcove_record_bound || !alcove->face_selector_bound ||
        !alcove->raw_layout_bound || !alcove->no_draw_only || !entry->valid ||
        !entry->target_bound || !entry->fixed_header_bound ||
        !entry->counted_regions_bound || !entry->no_draw_only || !face->valid ||
        !face->entry_bound || !face->face_row_bound || !face->vertex_indexes_bound ||
        !face->no_draw_only || !row->valid || !row->entry_bound ||
        !row->second_region_bound || !row->ordinal_row_bound || !row->no_draw_only ||
        identity->byte_count != (uint64_t)dgn_size ||
        identity->level_index != alcove->level_index ||
        identity->level_index != entry->level_index ||
        identity->level_index != face->level_index || identity->level_index != row->level_index ||
        alcove->package_fnv1a64 != entry->package_fnv1a64 ||
        entry->package_fnv1a64 != face->package_fnv1a64 ||
        entry->package_fnv1a64 != row->package_fnv1a64 ||
        face->entry_offset != entry->target_offset || face->entry_fnv1a64 != entry->target_fnv1a64 ||
        row->entry_offset != entry->target_offset || row->entry_fnv1a64 != entry->target_fnv1a64 ||
        row->row_ordinal != face->face_ordinal || alcove->raw_face_selector != face->face_ordinal ||
        face->face_ordinal >= entry->second_region_count ||
        alcove->record_offset > (uint32_t)dgn_size || 12U > (uint32_t)dgn_size - alcove->record_offset ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    expected_row_offset = (uint64_t)entry->target_offset + entry->second_region_offset +
        (uint64_t)face->face_ordinal * 12U;
    if (package_fnv1a64 != identity->fnv1a64 || package_fnv1a64 != alcove->package_fnv1a64 ||
        fnv1a64(dgn_data + entry->target_offset, entry->target_length) != entry->target_fnv1a64 ||
        fnv1a64(dgn_data + entry->target_offset + entry->second_region_offset,
                entry->second_region_length) != entry->second_region_fnv1a64 ||
        expected_row_offset > (uint64_t)dgn_size || 12U > (uint64_t)dgn_size - expected_row_offset ||
        face->face_offset != (uint32_t)expected_row_offset || row->row_offset != (uint32_t)expected_row_offset) {
        *out_receipt = receipt;
        return 0;
    }
    alcove_bytes = dgn_data + alcove->record_offset;
    row_bytes = dgn_data + row->row_offset;
    if (alcove_bytes[0] != 0x20U || alcove_bytes[1] != alcove->raw_face_selector ||
        fnv1a64(alcove_bytes, 12U) != alcove->record_fnv1a64 ||
        fnv1a64(row_bytes, 12U) != row->row_fnv1a64 ||
        fnv1a64(row_bytes, 12U) != face->face_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.structure1f_record_bound = receipt.structure3_entry_bound = 1;
    receipt.structure3_row_bound = receipt.selector_row_ordinal_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.structure1f_record_offset = alcove->record_offset;
    receipt.structure1f_record_fnv1a64 = alcove->record_fnv1a64;
    receipt.raw_selector = alcove->raw_face_selector;
    receipt.structure3_row_ordinal = row->row_ordinal;
    receipt.structure3_row_offset = row->row_offset;
    receipt.structure3_row_fnv1a64 = row->row_fnv1a64;
    *out_receipt = receipt;
    return 1;
}
