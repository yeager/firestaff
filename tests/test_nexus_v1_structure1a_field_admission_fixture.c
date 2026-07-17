#include "nexus_v1_structure1a_field_admission.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603); size_t index;
    for (index = 0U; index < size; ++index) { hash ^= bytes[index]; hash *= UINT64_C(1099511628211); }
    return hash;
}

int main(void)
{
    static const uint8_t record[24] = { 0x5aU, 12U, 0xe1U, 0x10U, 0x20U, 0x30U,
        0x40U, 0x50U, 0x60U, 0x70U, 0x80U, 0x90U, 0xa0U, 0xb0U, 0xc0U, 0xd0U,
        0xe0U, 0xf0U, 1U, 2U, 3U, 4U, 5U, 6U };
    static const char md5[] = "283ef4a9a414af8373213c3d41067cda";
    char path[] = "/tmp/firestaff-nexus-structure1a-field-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity, other;
    Nexus_V1_Structure1ATargetAdmissionReceipt target;
    Nexus_V1_Structure1AFieldAdmissionReceipt receipt;
    int file = mkstemp(path);

    if (file < 0 || write(file, record, sizeof(record)) != (ssize_t)sizeof(record) || close(file) != 0) {
        if (file >= 0) close(file); unlink(path); return 1;
    }
    memset(&identity, 0, sizeof(identity)); identity.valid = 1; identity.level_index = 9U;
    identity.byte_count = sizeof(record); identity.fnv1a64 = fnv1a64(record, sizeof(record));
    memcpy(identity.md5, md5, sizeof(md5)); snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    memset(&target, 0, sizeof(target)); target.valid = target.target_record_bound = target.no_draw_only = 1;
    target.level_index = identity.level_index; target.package_fnv1a64 = identity.fnv1a64;
    target.target_record_length = sizeof(record); target.target_record_fnv1a64 = identity.fnv1a64;
    if (!nexus_v1_structure1a_field_admit(&identity, record, sizeof(record), &target, &receipt) ||
        !receipt.valid || receipt.raw_kind != 0x5aU || receipt.structure3_model_index != 12U ||
        receipt.raw_rotation_selector != 0xe1U || memcmp(receipt.raw_tail, record + 3U, 21U) ||
        receipt.face_semantics_permitted || receipt.mesh_semantics_permitted ||
        receipt.material_semantics_permitted || receipt.draw_permitted) { unlink(path); return 1; }
    other = identity; other.level_index++;
    if (nexus_v1_structure1a_field_admit(&other, record, sizeof(record), &target, &receipt) || receipt.valid) { unlink(path); return 1; }
    target.target_record_fnv1a64 ^= 1U;
    if (nexus_v1_structure1a_field_admit(&identity, record, sizeof(record), &target, &receipt) || receipt.valid) { unlink(path); return 1; }
    unlink(path); puts("Structure1A field admission fixture: PASS"); return 0;
}
