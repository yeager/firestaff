#include "nexus_v1_structure3_normal_admission.h"
#include "nexus_v1_structure3_face_vertex_set_admission.h"
#include "nexus_v1_structure3_face_tail_admission.h"
#include "nexus_v1_structure3_face_index_prefix_admission.h"
#include "nexus_v1_structure3_first_region_row_admission.h"
#include "nexus_v1_structure3_second_region_row_admission.h"

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
    char command[512];
    FILE *pipe;
    if (snprintf(command, sizeof(command), "md5 -q '%s'", path) >= (int)sizeof(command) ||
        !(pipe = popen(command, "r"))) return 0;
    if (!fgets(out, 33, pipe) || pclose(pipe) != 0) return 0;
    out[32] = '\0';
    return 1;
}

static int rewrite_identity(const char *path, const uint8_t *data, size_t size,
                            Nexus_V1_LevCorpusDirectLevelIdentity *identity)
{
    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd < 0 || write(fd, data, size) != (ssize_t)size || close(fd) != 0)
        return 0;
    identity->fnv1a64 = fnv1a64(data, size);
    return md5_file(path, identity->md5);
}

int main(void)
{
    uint8_t data[128] = {0};
    char path[] = "/tmp/firestaff-nexus-s3-face-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity;
    Nexus_V1_Structure3TargetAdmissionReceipt target;
    Nexus_V1_Structure3EntryAdmissionReceipt entry;
    Nexus_V1_Structure3FaceAdmissionReceipt receipt;
    Nexus_V1_Structure3NormalAdmissionReceipt normal;
    Nexus_V1_Structure3FaceVertexAdmissionReceipt vertex;
    Nexus_V1_Structure3FaceVertexSetAdmissionReceipt vertex_set;
    Nexus_V1_Structure3FaceTailAdmissionReceipt tail;
    Nexus_V1_Structure3FaceIndexPrefixAdmissionReceipt index_prefix;
    Nexus_V1_Structure3FirstRegionRowAdmissionReceipt first_region_row;
    Nexus_V1_Structure3SecondRegionRowAdmissionReceipt second_region_row;
    int fd = mkstemp(path);
    int result = 1;

    CHECK(fd >= 0);
    write_be32(data, 0x100U);
    write_be16(data + 4U, 4U);
    write_be16(data + 6U, 1U);
    write_be32(data + 8U, 40U);
    write_be32(data + 16U, 88U);
    write_be32(data + 20U, 100U);
    memset(data + 40U, 0xa1, 48U);
    write_be16(data + 88U, 0U); write_be16(data + 90U, 1U);
    write_be16(data + 92U, 2U); write_be16(data + 94U, 2U);
    data[96] = 0x41U; data[97] = 0x7eU;
    write_be16(data + 98U, 0x00a5U);
    memset(data + 100U, 0xc3, 12U);
    CHECK(write(fd, data, sizeof(data)) == (ssize_t)sizeof(data));
    CHECK(close(fd) == 0); fd = -1;
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1; identity.level_index = 12U; identity.byte_count = sizeof(data);
    identity.fnv1a64 = fnv1a64(data, sizeof(data));
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    CHECK(md5_file(path, identity.md5));
    memset(&target, 0, sizeof(target));
    target.valid = target.field_bound = target.directory_bound = 1;
    target.target_span_bound = target.no_draw_only = 1;
    target.level_index = identity.level_index; target.package_fnv1a64 = identity.fnv1a64;
    target.target_length = 112U; target.target_fnv1a64 = fnv1a64(data, target.target_length);
    CHECK(nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &entry));
    CHECK(nexus_v1_structure3_face_admit(&identity, data, sizeof(data), &entry, 0U, &receipt));
    CHECK(nexus_v1_structure3_normal_admit(
              &identity, data, sizeof(data), &entry, &receipt, &normal));
    CHECK(nexus_v1_structure3_face_vertex_admit(
              &identity, data, sizeof(data), &entry, &receipt, 1U, &vertex));
    CHECK(nexus_v1_structure3_face_vertex_set_admit(
              &identity, data, sizeof(data), &entry, &receipt, &vertex_set));
    CHECK(nexus_v1_structure3_face_tail_admit(
              &identity, data, sizeof(data), &entry, &receipt, &tail));
    CHECK(nexus_v1_structure3_face_index_prefix_admit(
              &identity, data, sizeof(data), &entry, &receipt, &index_prefix));
    CHECK(nexus_v1_structure3_first_region_row_admit(
              &identity, data, sizeof(data), &entry, 1U, &first_region_row));
    CHECK(nexus_v1_structure3_second_region_row_admit(
              &identity, data, sizeof(data), &entry, 0U, &second_region_row));
    CHECK(receipt.valid && receipt.vertex_indexes[0] == 0U && receipt.vertex_indexes[1] == 1U &&
          receipt.vertex_indexes[2] == 2U && receipt.vertex_indexes[3] == 2U &&
          receipt.raw_control == 0x41U && receipt.raw_auxiliary == 0x7eU &&
          receipt.raw_fill_selector == 0x00a5U && receipt.fourth_index_repeats_third &&
          !receipt.geometry_semantics_permitted && !receipt.material_semantics_permitted &&
          !receipt.texture_semantics_permitted && !receipt.draw_permitted);
    CHECK(normal.valid && normal.face_ordinal == 0U && normal.row_offset == 100U &&
          !memcmp(normal.raw_bytes, data + 100U, sizeof(normal.raw_bytes)) &&
          !normal.geometry_semantics_permitted && !normal.lighting_semantics_permitted &&
          !normal.material_semantics_permitted && !normal.draw_permitted);
    CHECK(vertex.valid && vertex.index_slot == 1U && vertex.raw_index == 1U &&
          vertex.row_offset == 52U && !memcmp(vertex.raw_bytes, data + 52U,
                                                sizeof(vertex.raw_bytes)) &&
          !vertex.coordinate_semantics_permitted && !vertex.geometry_semantics_permitted &&
          !vertex.material_semantics_permitted && !vertex.draw_permitted);
    CHECK(vertex_set.valid && vertex_set.slots[0].raw_index == 0U &&
          vertex_set.slots[1].raw_index == 1U && vertex_set.slots[2].raw_index == 2U &&
          vertex_set.slots[3].raw_index == 2U && !vertex_set.topology_semantics_permitted &&
          !vertex_set.geometry_semantics_permitted && !vertex_set.material_semantics_permitted &&
          !vertex_set.draw_permitted);
    CHECK(tail.valid && tail.tail_offset == 96U &&
          !memcmp(tail.raw_bytes, data + 96U, sizeof(tail.raw_bytes)) &&
          !tail.material_semantics_permitted && !tail.texture_semantics_permitted &&
          !tail.draw_permitted);
    CHECK(index_prefix.valid && index_prefix.prefix_offset == 88U &&
          !memcmp(index_prefix.raw_bytes, data + 88U,
                  sizeof(index_prefix.raw_bytes)) &&
          !index_prefix.topology_semantics_permitted &&
          !index_prefix.geometry_semantics_permitted && !index_prefix.draw_permitted);
    CHECK(first_region_row.valid && first_region_row.row_ordinal == 1U &&
          first_region_row.row_offset == 52U &&
          !memcmp(first_region_row.raw_bytes, data + 52U,
                  sizeof(first_region_row.raw_bytes)) &&
          !first_region_row.coordinate_semantics_permitted &&
          !first_region_row.geometry_semantics_permitted &&
          !first_region_row.material_semantics_permitted &&
          !first_region_row.texture_semantics_permitted && !first_region_row.draw_permitted);
    CHECK(!nexus_v1_structure3_first_region_row_admit(
              &identity, data, sizeof(data), &entry, 4U, &first_region_row) &&
          !first_region_row.valid);
    CHECK(second_region_row.valid && second_region_row.row_ordinal == 0U &&
          second_region_row.row_offset == 88U &&
          !memcmp(second_region_row.raw_bytes, data + 88U,
                  sizeof(second_region_row.raw_bytes)) &&
          !second_region_row.topology_semantics_permitted &&
          !second_region_row.geometry_semantics_permitted &&
          !second_region_row.material_semantics_permitted &&
          !second_region_row.texture_semantics_permitted && !second_region_row.draw_permitted);
    CHECK(!nexus_v1_structure3_second_region_row_admit(
              &identity, data, sizeof(data), &entry, 1U, &second_region_row) &&
          !second_region_row.valid);
    CHECK(!nexus_v1_structure3_face_vertex_admit(
              &identity, data, sizeof(data), &entry, &receipt, 4U, &vertex) && !vertex.valid);
    CHECK(!nexus_v1_structure3_face_admit(&identity, data, sizeof(data), &entry, 1U, &receipt) &&
          !receipt.valid);
    data[97] ^= 1U;
    CHECK(rewrite_identity(path, data, sizeof(data), &identity));
    target.package_fnv1a64 = identity.fnv1a64;
    target.target_fnv1a64 = fnv1a64(data, target.target_length);
    CHECK(nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &entry));
    CHECK(!nexus_v1_structure3_normal_admit(
              &identity, data, sizeof(data), &entry, &receipt, &normal) && !normal.valid);
    CHECK(nexus_v1_structure3_face_admit(&identity, data, sizeof(data), &entry, 0U, &receipt));
    CHECK(nexus_v1_structure3_normal_admit(
              &identity, data, sizeof(data), &entry, &receipt, &normal));
    CHECK(nexus_v1_structure3_face_tail_admit(
              &identity, data, sizeof(data), &entry, &receipt, &tail));
    write_be16(data + 94U, 4U);
    CHECK(rewrite_identity(path, data, sizeof(data), &identity));
    target.package_fnv1a64 = identity.fnv1a64;
    target.target_fnv1a64 = fnv1a64(data, target.target_length);
    CHECK(nexus_v1_structure3_entry_admit(&identity, data, sizeof(data), &target, &entry));
    CHECK(!nexus_v1_structure3_face_admit(&identity, data, sizeof(data), &entry, 0U, &receipt) &&
          !receipt.valid);
    result = 0;
done:
    if (fd >= 0) close(fd);
    unlink(path);
    if (!result) puts("Structure3 face admission fixture: PASS");
    return result;
}
