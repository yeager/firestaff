#include "nexus_v1_structure3_entry_tag_admission.h"
#include "asset_find_by_hash.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "failed: %s\n", #condition); goto done; } } while (0)

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= bytes[index]; hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static void write_be16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value >> 8); bytes[1] = (uint8_t)value;
}

static void write_be32(uint8_t *bytes, uint32_t value)
{
    bytes[0] = (uint8_t)(value >> 24); bytes[1] = (uint8_t)(value >> 16);
    bytes[2] = (uint8_t)(value >> 8); bytes[3] = (uint8_t)value;
}

static int md5_file(const char *path, char out[33])
{
    return asset_file_md5_hex(path, out);
}

int main(void)
{
    uint8_t data[128] = {0};
    char path[] = "/tmp/firestaff-nexus-s3-entry-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity;
    Nexus_V1_Structure3TargetAdmissionReceipt target;
    Nexus_V1_Structure3EntryAdmissionReceipt receipt;
    Nexus_V1_Structure3EntryTagAdmissionReceipt tag;
    int fd = mkstemp(path);
    int result = 1;

    CHECK(fd >= 0);
    write_be32(data, 0x00000100U);
    write_be16(data + 4U, 2U);
    write_be16(data + 6U, 1U);
    write_be32(data + 8U, 40U);
    write_be32(data + 16U, 64U);
    write_be32(data + 20U, 76U);
    memset(data + 40U, 0xa1, 24U);
    memset(data + 64U, 0xb2, 12U);
    memset(data + 76U, 0xc3, 12U);
    CHECK(write(fd, data, sizeof(data)) == (ssize_t)sizeof(data));
    CHECK(close(fd) == 0); fd = -1;
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1; identity.level_index = 11U; identity.byte_count = sizeof(data);
    identity.fnv1a64 = fnv1a64(data, sizeof(data));
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    CHECK(md5_file(path, identity.md5));
    memset(&target, 0, sizeof(target));
    target.valid = target.field_bound = target.directory_bound = 1;
    target.target_span_bound = target.no_draw_only = 1;
    target.level_index = identity.level_index; target.package_fnv1a64 = identity.fnv1a64;
    target.target_length = 88U; target.target_fnv1a64 = fnv1a64(data, target.target_length);
    CHECK(nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &receipt));
    CHECK(nexus_v1_structure3_entry_tag_admit(
              &identity, data, sizeof(data), &receipt, &tag));
    CHECK(receipt.valid && receipt.raw_tag == 0x100U && receipt.first_region_count == 2U &&
          receipt.second_region_count == 1U && receipt.first_region_offset == 40U &&
          receipt.second_region_offset == 64U && receipt.third_region_offset == 76U &&
          receipt.first_region_length == 24U && receipt.second_region_length == 12U &&
          receipt.third_region_length == 12U && receipt.first_region_fnv1a64 &&
          receipt.second_region_fnv1a64 && receipt.third_region_fnv1a64 &&
          !receipt.geometry_semantics_permitted && !receipt.material_semantics_permitted &&
          !receipt.texture_semantics_permitted && !receipt.draw_permitted);
    CHECK(tag.valid && tag.tag_offset == 0U &&
          !memcmp(tag.raw_bytes, data, sizeof(tag.raw_bytes)) &&
          !tag.entry_kind_semantics_permitted && !tag.geometry_semantics_permitted &&
          !tag.draw_permitted);
    target.target_fnv1a64 ^= 1U;
    CHECK(!nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &receipt) &&
          !receipt.valid);
    target.target_fnv1a64 ^= 1U;
    write_be32(data + 16U, 63U);
    fd = open(path, O_WRONLY | O_TRUNC);
    CHECK(fd >= 0 && write(fd, data, sizeof(data)) == (ssize_t)sizeof(data) &&
          close(fd) == 0);
    fd = -1;
    identity.fnv1a64 = fnv1a64(data, sizeof(data));
    target.package_fnv1a64 = identity.fnv1a64;
    target.target_fnv1a64 = fnv1a64(data, target.target_length);
    CHECK(md5_file(path, identity.md5));
    CHECK(!nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &receipt) &&
          !receipt.valid);
    result = 0;
done:
    if (fd >= 0) close(fd);
    unlink(path);
    if (!result) puts("Structure3 entry admission fixture: PASS");
    return result;
}
