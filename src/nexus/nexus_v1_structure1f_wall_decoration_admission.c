#include "nexus_v1_structure1f_wall_decoration_admission.h"

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

int nexus_v1_structure1f_wall_decoration_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t record_index,
    Nexus_V1_Structure1FWallDecorationAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1FWallDecorationAdmissionReceipt receipt;
    const Nexus_V1_Structure1FDirectoryFamilyReceipt *family;
    const uint8_t *record;
    uint64_t package_fnv1a64;
    uint64_t record_offset;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !directory ||
        !directory->valid || !directory->direct_identity_bound ||
        !directory->parser_layout_bound || !directory->family_directory_bound ||
        !directory->no_draw_only || !identity->valid ||
        identity->level_index != directory->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    family = &directory->families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS];
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != directory->package_fnv1a64 ||
        family->source_tag != 0x21U || family->record_size != 12U ||
        !family->record_count || record_index >= family->record_count ||
        family->record_length != family->record_count * family->record_size ||
        family->record_offset > (uint32_t)dgn_size ||
        family->record_length > (uint32_t)dgn_size - family->record_offset ||
        fnv1a64(dgn_data + family->record_offset, family->record_length) !=
            family->record_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    record_offset = (uint64_t)family->record_offset +
        (uint64_t)record_index * family->record_size;
    if (record_offset > (uint64_t)dgn_size ||
        family->record_size > (uint64_t)dgn_size - record_offset) {
        *out_receipt = receipt;
        return 0;
    }
    record = dgn_data + record_offset;
    if (record[0] != 0x21U) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.directory_bound = receipt.wall_decoration_record_bound = 1;
    receipt.face_selector_bound = receipt.structure1a_index_bound = 1;
    receipt.raw_layout_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.record_index = record_index;
    receipt.record_offset = (uint32_t)record_offset;
    receipt.record_fnv1a64 = fnv1a64(record, family->record_size);
    receipt.source_tag = record[0];
    receipt.raw_face_selector = record[1];
    receipt.raw_structure1a_index =
        (uint16_t)(((uint16_t)record[2] << 8) | record[3]);
    memcpy(receipt.raw_payload, record + 4U, sizeof(receipt.raw_payload));
    *out_receipt = receipt;
    return 1;
}
