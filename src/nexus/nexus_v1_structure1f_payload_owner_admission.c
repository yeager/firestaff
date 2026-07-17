#include "nexus_v1_structure1f_payload_owner_admission.h"

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

int nexus_v1_structure1f_payload_owner_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t family_index, uint32_t record_index,
    Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt receipt;
    const Nexus_V1_Structure1FDirectoryFamilyReceipt *family;
    uint8_t expected_tag;
    uint32_t expected_size;
    uint32_t payload_length;
    uint64_t package_fnv1a64;
    uint64_t record_offset;
    const uint8_t *record;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !directory || !identity->valid ||
        !directory->valid || !directory->direct_identity_bound ||
        !directory->parser_layout_bound || !directory->family_directory_bound ||
        !directory->no_draw_only || identity->level_index != directory->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        (family_index != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS &&
         family_index != NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS) ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt;
        return 0;
    }
    expected_tag = family_index == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS ?
        0x11U : 0x12U;
    expected_size = family_index == NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS ?
        12U : 16U;
    payload_length = expected_size - 3U;
    family = &directory->families[family_index];
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    if (package_fnv1a64 != identity->fnv1a64 || package_fnv1a64 != directory->package_fnv1a64 ||
        family->source_tag != expected_tag || family->record_size != expected_size ||
        !family->record_count || record_index >= family->record_count ||
        family->record_length != family->record_count * family->record_size ||
        family->record_offset > (uint32_t)dgn_size ||
        family->record_length > (uint32_t)dgn_size - family->record_offset ||
        fnv1a64(dgn_data + family->record_offset, family->record_length) != family->record_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    record_offset = (uint64_t)family->record_offset + (uint64_t)record_index * expected_size;
    if (record_offset > (uint64_t)dgn_size || expected_size > (uint64_t)dgn_size - record_offset) {
        *out_receipt = receipt;
        return 0;
    }
    record = dgn_data + record_offset;
    if (record[0] != expected_tag) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = receipt.directory_bound = receipt.family_record_bound = 1;
    receipt.payload_owner_bound = receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.family_index = family_index;
    receipt.source_tag = expected_tag;
    receipt.record_index = record_index;
    receipt.record_offset = (uint32_t)record_offset;
    receipt.record_fnv1a64 = fnv1a64(record, expected_size);
    receipt.payload_offset = (uint32_t)record_offset + 3U;
    receipt.payload_length = payload_length;
    receipt.payload_fnv1a64 = fnv1a64(record + 3U, payload_length);
    memcpy(receipt.raw_payload, record + 3U, payload_length);
    *out_receipt = receipt;
    return 1;
}
