#include "nexus_v1_structure1a_target_admission.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FIXTURE_SIZE 0x10800U
#define STRUCTURE1_OFFSET 0x800U
#define STRUCTURE1B_OFFSET 0x100U
#define STRUCTURE1F_OFFSET 0x8100U
#define STRUCTURE1F_RECORD_OFFSET (STRUCTURE1_OFFSET + STRUCTURE1F_OFFSET + 16U)

static void be16(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value >> 8); p[1] = (uint8_t)value;
}

static void be32(uint8_t *p, uint32_t value)
{
    p[0] = (uint8_t)(value >> 24); p[1] = (uint8_t)(value >> 16);
    p[2] = (uint8_t)(value >> 8); p[3] = (uint8_t)value;
}

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index]; hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int file_md5(const char *path, char out[33])
{
    return asset_file_md5_hex(path, out);
}

int main(void)
{
    uint8_t *dgn = calloc(1U, FIXTURE_SIZE);
    char path[] = "/tmp/firestaff-nexus-structure1a-target-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity;
    Nexus_V1_LevCorpusDirectLevelIdentity other_identity;
    Nexus_V1_Structure1FDirectoryAdmissionReceipt structure1f;
    Nexus_V1_Structure1ATargetReference reference;
    Nexus_V1_Structure1ATargetAdmissionReceipt receipt;
    uint8_t *structure1;
    int file;

    if (!dgn) return 1;
    be16(dgn + 0x0cU, 1U); be16(dgn + 0x0eU, 32U);
    be32(dgn + 0x10U, STRUCTURE1F_OFFSET + 28U);
    structure1 = dgn + STRUCTURE1_OFFSET;
    be32(structure1 + 0x0cU, 1U);
    be32(structure1 + 0x10U, 0x38U);
    be32(structure1 + 0x14U, STRUCTURE1B_OFFSET);
    be32(structure1 + 0x34U, STRUCTURE1F_OFFSET);
    structure1[0x38U] = 0xa1U;
    structure1[0x39U] = 0xb2U;
    structure1[0x3aU] = 0xc3U;
    be16(structure1 + STRUCTURE1F_OFFSET + 10U, 1U);
    structure1[STRUCTURE1F_OFFSET + 16U] = 0x20U;
    structure1[STRUCTURE1F_OFFSET + 17U] = 4U;
    be16(structure1 + STRUCTURE1F_OFFSET + 18U, 0U);

    file = mkstemp(path);
    if (file < 0 || write(file, dgn, FIXTURE_SIZE) != (ssize_t)FIXTURE_SIZE ||
        close(file) != 0) {
        if (file >= 0) close(file);
        unlink(path); free(dgn); return 1;
    }
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1; identity.level_index = 8U; identity.byte_count = FIXTURE_SIZE;
    identity.fnv1a64 = fnv1a64(dgn, FIXTURE_SIZE);
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    if (!file_md5(path, identity.md5) ||
        !nexus_v1_structure1f_directory_admit(&identity, dgn, FIXTURE_SIZE, &structure1f)) {
        unlink(path); free(dgn); return 1;
    }
    memset(&reference, 0, sizeof(reference));
    reference.source_tag = 0x20U;
    reference.source_record_offset = STRUCTURE1F_RECORD_OFFSET;
    reference.source_record_fnv1a64 = fnv1a64(dgn + STRUCTURE1F_RECORD_OFFSET, 12U);
    if (!nexus_v1_structure1a_target_admit(
            &identity, dgn, FIXTURE_SIZE, &structure1f, &reference, &receipt) ||
        !receipt.valid || receipt.structure1a_index != 0U ||
        receipt.target_record_offset != STRUCTURE1_OFFSET + 0x38U ||
        receipt.target_record_length != NEXUS_DGN_STRUCTURE1A_ENTRY_BYTES ||
        receipt.face_semantics_permitted || receipt.mesh_semantics_permitted ||
        receipt.material_semantics_permitted || receipt.draw_permitted) {
        unlink(path); free(dgn); return 1;
    }
    other_identity = identity; other_identity.level_index++;
    if (nexus_v1_structure1a_target_admit(
            &other_identity, dgn, FIXTURE_SIZE, &structure1f, &reference, &receipt) ||
        receipt.valid) { unlink(path); free(dgn); return 1; }
    reference.structure1a_index = 1U;
    if (nexus_v1_structure1a_target_admit(
            &identity, dgn, FIXTURE_SIZE, &structure1f, &reference, &receipt) ||
        receipt.valid) { unlink(path); free(dgn); return 1; }
    reference.structure1a_index = 0U;
    reference.source_record_fnv1a64 ^= 1U;
    if (nexus_v1_structure1a_target_admit(
            &identity, dgn, FIXTURE_SIZE, &structure1f, &reference, &receipt) ||
        receipt.valid) { unlink(path); free(dgn); return 1; }
    unlink(path); free(dgn);
    puts("Structure1A target admission fixture: PASS");
    return 0;
}
