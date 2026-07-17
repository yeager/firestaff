#include "nexus_v1_structure1a_target_admission.h"

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

int nexus_v1_structure1a_target_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *structure1f_directory,
    const Nexus_V1_Structure1ATargetReference *reference,
    Nexus_V1_Structure1ATargetAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1ATargetAdmissionReceipt receipt;
    Nexus_V1_DgnStructure1Layout layout;
    const Nexus_V1_Structure1FDirectoryFamilyReceipt *family;
    uint64_t package_fnv1a64;
    uint64_t source_end;
    uint64_t target_offset;
    uint64_t directory_offset;
    uint64_t directory_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !structure1f_directory ||
        !reference || !structure1f_directory->valid ||
        !structure1f_directory->direct_identity_bound ||
        !structure1f_directory->parser_layout_bound ||
        !structure1f_directory->family_directory_bound ||
        !structure1f_directory->no_draw_only || !identity->valid ||
        identity->level_index != structure1f_directory->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    if (reference->source_tag != 0x20U && reference->source_tag != 0x21U) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    family = &structure1f_directory->families[
        reference->source_tag == 0x20U ? NEXUS_V1_DGN_STRUCTURE1F_ALCOVES :
                                         NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS];
    source_end = (uint64_t)reference->source_record_offset + family->record_size;
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != structure1f_directory->package_fnv1a64 ||
        family->source_tag != reference->source_tag || family->record_size != 12U ||
        !family->record_count ||
        reference->source_record_offset < family->record_offset ||
        source_end > (uint64_t)family->record_offset + family->record_length ||
        source_end > (uint64_t)dgn_size ||
        fnv1a64(dgn_data + reference->source_record_offset, family->record_size) !=
            reference->source_record_fnv1a64 ||
        dgn_data[reference->source_record_offset] != reference->source_tag ||
        (uint16_t)(((uint16_t)dgn_data[reference->source_record_offset + 2U] << 8) |
                   dgn_data[reference->source_record_offset + 3U]) !=
            reference->structure1a_index ||
        nexus_v1_dgn_structure1_layout(&layout, dgn_data, dgn_size) != 0 ||
        !layout.valid || !layout.structure1a.valid ||
        layout.structure1a.entry_count <= 0 ||
        reference->structure1a_index >= (uint32_t)layout.structure1a.entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    directory_offset = (uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1a.relative_offset;
    directory_length = (uint64_t)layout.structure1a.size;
    target_offset = directory_offset +
        (uint64_t)reference->structure1a_index * NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
    if (directory_offset > (uint64_t)dgn_size || directory_length > (uint64_t)dgn_size - directory_offset ||
        target_offset > (uint64_t)dgn_size ||
        NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES > (uint64_t)dgn_size - target_offset) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.directory_bound = receipt.source_reference_bound = 1;
    receipt.target_record_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.source_tag = reference->source_tag;
    receipt.source_record_offset = reference->source_record_offset;
    receipt.source_record_fnv1a64 = reference->source_record_fnv1a64;
    receipt.structure1a_index = reference->structure1a_index;
    receipt.directory_offset = (uint32_t)directory_offset;
    receipt.directory_length = (uint32_t)directory_length;
    receipt.directory_fnv1a64 = fnv1a64(dgn_data + directory_offset, directory_length);
    receipt.target_record_offset = (uint32_t)target_offset;
    receipt.target_record_length = NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES;
    receipt.target_record_fnv1a64 = fnv1a64(dgn_data + target_offset,
                                             NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES);
    *out_receipt = receipt;
    return 1;
}
