#include "nexus_v1_structure1f_wall_decoration_structure3_row_admission.h"

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

int nexus_v1_structure1f_wall_decoration_structure3_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FWallDecorationAdmissionReceipt *wall_decoration,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    const Nexus_V1_Structure3SecondRegionRowAdmissionReceipt *row,
    Nexus_V1_Structure1FWallDecorationStructure3RowAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1FWallDecorationStructure3RowAdmissionReceipt receipt;
    uint64_t package_fnv1a64;
    uint64_t expected_face_offset;
    const uint8_t *wall_bytes;
    const uint8_t *face_bytes;
    const uint8_t *row_bytes;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !wall_decoration || !entry ||
        !face || !row || !identity->valid ||
        !wall_decoration->valid || !wall_decoration->directory_bound ||
        !wall_decoration->wall_decoration_record_bound ||
        !wall_decoration->face_selector_bound || !wall_decoration->raw_layout_bound ||
        !wall_decoration->no_draw_only || !entry->valid || !entry->target_bound ||
        !entry->fixed_header_bound || !entry->counted_regions_bound ||
        !entry->no_draw_only || !face->valid || !face->entry_bound ||
        !face->face_row_bound || !face->vertex_indexes_bound || !face->no_draw_only ||
        !row->valid || !row->entry_bound || !row->second_region_bound ||
        !row->ordinal_row_bound || !row->no_draw_only ||
        identity->byte_count != (uint64_t)dgn_size ||
        identity->level_index != wall_decoration->level_index ||
        identity->level_index != entry->level_index ||
        identity->level_index != face->level_index ||
        identity->level_index != row->level_index ||
        wall_decoration->package_fnv1a64 != entry->package_fnv1a64 ||
        entry->package_fnv1a64 != face->package_fnv1a64 ||
        entry->package_fnv1a64 != row->package_fnv1a64 ||
        face->entry_offset != entry->target_offset ||
        face->entry_fnv1a64 != entry->target_fnv1a64 ||
        row->entry_offset != entry->target_offset ||
        row->entry_fnv1a64 != entry->target_fnv1a64 ||
        row->row_ordinal != face->face_ordinal ||
        wall_decoration->raw_face_selector != face->face_ordinal ||
        face->face_ordinal >= entry->second_region_count ||
        wall_decoration->record_offset > (uint32_t)dgn_size ||
        12U > (uint32_t)dgn_size - wall_decoration->record_offset ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    expected_face_offset = (uint64_t)entry->target_offset + entry->second_region_offset +
        (uint64_t)face->face_ordinal * 12U;
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != wall_decoration->package_fnv1a64 ||
        fnv1a64(dgn_data + entry->target_offset, entry->target_length) !=
            entry->target_fnv1a64 ||
        fnv1a64(dgn_data + entry->second_region_offset + entry->target_offset,
                entry->second_region_length) != entry->second_region_fnv1a64 ||
        expected_face_offset > (uint64_t)dgn_size ||
        12U > (uint64_t)dgn_size - expected_face_offset ||
        face->face_offset != (uint32_t)expected_face_offset ||
        row->row_offset != (uint32_t)expected_face_offset) {
        *out_receipt = receipt;
        return 0;
    }
    wall_bytes = dgn_data + wall_decoration->record_offset;
    face_bytes = dgn_data + face->face_offset;
    row_bytes = dgn_data + row->row_offset;
    if (wall_bytes[0] != 0x21U || wall_bytes[1] != wall_decoration->raw_face_selector ||
        fnv1a64(wall_bytes, 12U) != wall_decoration->record_fnv1a64 ||
        fnv1a64(face_bytes, 12U) != face->face_fnv1a64 ||
        fnv1a64(row_bytes, 12U) != row->row_fnv1a64 ||
        memcmp(face_bytes, row_bytes, 12U) != 0) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.structure1f_record_bound = receipt.structure3_entry_bound = 1;
    receipt.structure3_row_bound = receipt.selector_row_ordinal_bound = 1;
    receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.structure1f_record_offset = wall_decoration->record_offset;
    receipt.structure1f_record_fnv1a64 = wall_decoration->record_fnv1a64;
    receipt.raw_selector = wall_decoration->raw_face_selector;
    receipt.structure3_row_ordinal = row->row_ordinal;
    receipt.structure3_row_offset = row->row_offset;
    receipt.structure3_row_fnv1a64 = row->row_fnv1a64;
    *out_receipt = receipt;
    return 1;
}
