#include "nexus_v1_structure1a_field_admission.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index]; hash *= UINT64_C(1099511628211);
    }
    return hash;
}

int nexus_v1_structure1a_field_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1ATargetAdmissionReceipt *target,
    Nexus_V1_Structure1AFieldAdmissionReceipt *out_receipt)
{
    Nexus_V1_Structure1AFieldAdmissionReceipt receipt;
    const uint8_t *record;
    uint64_t package_fnv1a64;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!identity || !dgn_data || dgn_size <= 0 || !target || !target->valid ||
        !target->target_record_bound || !target->no_draw_only || !identity->valid ||
        identity->level_index != target->level_index ||
        identity->byte_count != (uint64_t)dgn_size ||
        target->target_record_length != NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES ||
        target->target_record_offset > (uint32_t)dgn_size ||
        target->target_record_length > (uint32_t)dgn_size - target->target_record_offset ||
        !nexus_v1_lev_corpus_direct_identity_still_matches(identity)) {
        *out_receipt = receipt; return 0;
    }
    package_fnv1a64 = fnv1a64(dgn_data, (size_t)dgn_size);
    record = dgn_data + target->target_record_offset;
    if (package_fnv1a64 != identity->fnv1a64 ||
        package_fnv1a64 != target->package_fnv1a64 ||
        fnv1a64(record, NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES) !=
            target->target_record_fnv1a64) {
        *out_receipt = receipt; return 0;
    }
    receipt.valid = receipt.target_record_bound = receipt.raw_kind_bound = 1;
    receipt.structure3_model_reference_bound = receipt.raw_rotation_selector_bound = 1;
    receipt.no_draw_only = 1;
    receipt.level_index = identity->level_index;
    receipt.package_fnv1a64 = package_fnv1a64;
    receipt.target_record_offset = target->target_record_offset;
    receipt.target_record_fnv1a64 = target->target_record_fnv1a64;
    receipt.raw_kind = record[0];
    receipt.structure3_model_index = record[1];
    receipt.raw_rotation_selector = record[2];
    memcpy(receipt.raw_tail, record + 3U, sizeof(receipt.raw_tail));
    *out_receipt = receipt;
    return 1;
}
