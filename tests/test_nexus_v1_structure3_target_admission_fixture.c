#include "nexus_v1_structure3_target_admission.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "failed: %s\n", #c); goto done; } } while (0)

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603); size_t index;
    for (index = 0U; index < size; ++index) { hash ^= bytes[index]; hash *= UINT64_C(1099511628211); }
    return hash;
}

static int md5_file(const char *path, char out[33])
{
    return asset_file_md5_hex(path, out);
}

int main(void)
{
    uint8_t data[64] = {0}; char path[] = "/tmp/firestaff-nexus-s3-target-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity;
    Nexus_V1_Structure1AFieldAdmissionReceipt field;
    Nexus_V1_Structure3TargetAdmissionReceipt receipt;
    uint64_t directory_fnv1a64;
    int fd = mkstemp(path), result = 1;

    CHECK(fd >= 0);
    data[0] = 0x5aU; data[2] = 0xe1U;
    data[24] = 0U; data[25] = 0U; data[26] = 0U; data[27] = 1U;
    data[28] = 0U; data[29] = 0U; data[30] = 0U; data[31] = 8U;
    data[32] = 0xa1U;
    CHECK(write(fd, data, sizeof(data)) == (ssize_t)sizeof(data));
    CHECK(close(fd) == 0); fd = -1;
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1; identity.level_index = 10U; identity.byte_count = sizeof(data);
    identity.fnv1a64 = fnv1a64(data, sizeof(data));
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    CHECK(md5_file(path, identity.md5));
    CHECK(nexus_v1_lev_corpus_direct_identity_still_matches(&identity));
    memset(&field, 0, sizeof(field));
    field.valid = field.target_record_bound = field.structure3_model_reference_bound = field.no_draw_only = 1;
    field.level_index = identity.level_index; field.package_fnv1a64 = identity.fnv1a64;
    field.target_record_fnv1a64 = fnv1a64(data, 24U); field.structure3_model_index = 0U;
    directory_fnv1a64 = fnv1a64(data + 24U, 8U);
    CHECK(nexus_v1_structure3_target_admit(&identity, data, 64, &field, 24U, 40U, directory_fnv1a64, &receipt));
    CHECK(receipt.valid && receipt.target_offset == 32U && receipt.target_length == 32U);
    CHECK(!receipt.face_semantics_permitted && !receipt.mesh_semantics_permitted && !receipt.texture_semantics_permitted && !receipt.draw_permitted);
    data[27] = 0U;
    CHECK(!nexus_v1_structure3_target_admit(&identity, data, 64, &field, 24U, 40U, directory_fnv1a64, &receipt) && !receipt.valid);
    data[27] = 1U; identity.fnv1a64 ^= 1U;
    CHECK(!nexus_v1_structure3_target_admit(&identity, data, 64, &field, 24U, 40U, directory_fnv1a64, &receipt) && !receipt.valid);
    result = 0;
done:
    if (fd >= 0) close(fd); unlink(path);
    if (!result) puts("Structure3 target admission fixture: PASS");
    return result;
}
