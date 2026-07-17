#include "nexus_v1_structure3_face_admission.h"

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

int nexus_v1_structure3_face_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    uint32_t face_ordinal, Nexus_V1_Structure3FaceAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure3FaceAdmissionReceipt receipt;
    const uint8_t *entry_bytes;
    const uint8_t *face;
    uint64_t package_fnv1a64;
    uint64_t face_offset;
    unsigned int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !entry || !entry->valid ||
        !entry->target_bound || !entry->fixed_header_bound ||
        !entry->counted_regions_bound || !entry->no_draw_only || !identity->valid ||
        identity->level_index != entry->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        !entry->first_region_count || face_ordinal >= entry->second_region_count ||
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
        entry->first_region_offset != NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES ||
        entry->first_region_length != (uint32_t)entry->first_region_count * 12U ||
        entry->second_region_length != (uint32_t)entry->second_region_count * 12U ||
        entry->second_region_offset != entry->first_region_offset + entry->first_region_length ||
        fnv1a64(entry_bytes, entry->target_length) != entry->target_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    /* Recheck the retained source segments before exposing one row. */
    if (fnv1a64(entry_bytes, NEXUS_DGN_STRUCTURE3_ENTRY_HEADER_BYTES) !=
            entry->header_fnv1a64 ||
        fnv1a64(entry_bytes + entry->first_region_offset, entry->first_region_length) !=
            entry->first_region_fnv1a64 ||
        fnv1a64(entry_bytes + entry->second_region_offset, entry->second_region_length) !=
            entry->second_region_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    face_offset = (uint64_t)entry->target_offset + entry->second_region_offset +
        (uint64_t)face_ordinal * 12U;
    if (face_offset > (uint64_t)dgn_size || 12U > (uint64_t)dgn_size - face_offset) {
        *out_receipt = receipt;
        return 0;
    }
    face = dgn_data + face_offset;
    for (index = 0U; index < 4U; ++index) {
        receipt.vertex_indexes[index] = read_be16(face + index * 2U);
        if (receipt.vertex_indexes[index] >= entry->first_region_count) {
            *out_receipt = receipt;
            return 0;
        }
    }
    receipt.valid = receipt.entry_bound = receipt.face_row_bound = 1;
    receipt.vertex_indexes_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.entry_offset = entry->target_offset;
    receipt.entry_fnv1a64 = entry->target_fnv1a64;
    receipt.face_ordinal = face_ordinal;
    receipt.face_offset = (uint32_t)face_offset;
    receipt.face_fnv1a64 = fnv1a64(face, 12U);
    receipt.raw_control = face[8];
    receipt.raw_auxiliary = face[9];
    receipt.raw_fill_selector = read_be16(face + 10U);
    receipt.fourth_index_repeats_third =
        receipt.vertex_indexes[3] == receipt.vertex_indexes[2];
    *out_receipt = receipt;
    return 1;
}
