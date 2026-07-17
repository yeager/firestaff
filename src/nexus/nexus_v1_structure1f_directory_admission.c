#include "nexus_v1_structure1f_directory_admission.h"

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

int nexus_v1_structure1f_directory_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    Nexus_V1_Structure1FDirectoryAdmissionReceipt *out_receipt)
{
    static const uint8_t expected_tags[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT] =
        { 0x10U, 0x11U, 0x12U, 0x20U, 0x21U, 0x22U };
    Nexus_V1_Structure1FDirectoryAdmissionReceipt receipt;
    Nexus_V1_DgnStructure1Layout layout;
    uint64_t package_fnv1a64;
    uint64_t directory_end;
    uint32_t total_records = 0U;
    int family;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !identity->valid ||
        identity->level_index >= NEXUS_V1_LEV_CORPUS_LEVEL_COUNT ||
        !identity->byte_count || !identity->fnv1a64 || !dgn_data ||
        dgn_size < NEXUS_DGN_BLOCK_SIZE ||
        identity->byte_count != (uint64_t)dgn_size ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    if (package_fnv1a64 != identity->fnv1a64 ||
        nexus_v1_dgn_structure1_layout(&layout, dgn_data, dgn_size) != 0 ||
        !layout.valid || !layout.structure1f.valid ||
        layout.structure1_offset < 0 || layout.structure1f.relative_offset < 0 ||
        layout.structure1f.size < NEXUS_DGN_STRUCTURE1F_HEADER_BYTES) {
        *out_receipt = receipt;
        return 0;
    }
    directory_end = (uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1f.relative_offset +
        (uint64_t)layout.structure1f.size;
    if (directory_end > (uint64_t)dgn_size) {
        *out_receipt = receipt;
        return 0;
    }
    for (family = 0; family < NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT; ++family) {
        Nexus_V1_Structure1FDirectoryFamilyReceipt *out =
            &receipt.families[family];
        uint64_t record_offset = (uint64_t)layout.structure1_offset +
            (uint64_t)layout.structure1f.family_offset[family];
        uint64_t record_length = (uint64_t)layout.structure1f.family_count[family] *
            (uint64_t)layout.structure1f.family_record_size[family];
        uint32_t record;

        if (layout.structure1f.family_count[family] < 0 ||
            layout.structure1f.family_record_size[family] <= 0 ||
            record_offset > (uint64_t)dgn_size ||
            record_length > (uint64_t)dgn_size - record_offset ||
            record_offset < (uint64_t)layout.structure1_offset +
                (uint64_t)layout.structure1f.relative_offset +
                NEXUS_DGN_STRUCTURE1F_HEADER_BYTES ||
            record_offset + record_length > directory_end ||
            record_length > UINT32_MAX ||
            total_records > UINT32_MAX - (uint32_t)layout.structure1f.family_count[family]) {
            *out_receipt = receipt;
            return 0;
        }
        for (record = 0U; record < (uint32_t)layout.structure1f.family_count[family]; ++record) {
            if (dgn_data[record_offset +
                (uint64_t)record * layout.structure1f.family_record_size[family]] !=
                expected_tags[family]) {
                *out_receipt = receipt;
                return 0;
            }
        }
        out->source_tag = expected_tags[family];
        out->record_offset = (uint32_t)record_offset;
        out->record_length = (uint32_t)record_length;
        out->record_count = (uint32_t)layout.structure1f.family_count[family];
        out->record_size = (uint32_t)layout.structure1f.family_record_size[family];
        out->record_fnv1a64 = record_length ?
            fnv1a64(dgn_data + record_offset, (size_t)record_length) : 0U;
        total_records += out->record_count;
    }
    if (total_records != (uint32_t)layout.structure1f.total_entry_count) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.direct_identity_bound = receipt.parser_layout_bound = 1;
    receipt.family_directory_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.directory_offset = (uint32_t)((uint64_t)layout.structure1_offset +
        (uint64_t)layout.structure1f.relative_offset);
    receipt.directory_length = (uint32_t)layout.structure1f.size;
    receipt.directory_fnv1a64 = fnv1a64(dgn_data + receipt.directory_offset,
                                         receipt.directory_length);
    receipt.total_record_count = total_records;
    if (!receipt.directory_fnv1a64) memset(&receipt, 0, sizeof(receipt));
    *out_receipt = receipt;
    return out_receipt->valid;
}
