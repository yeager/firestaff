#include "nexus_v1_structure3_normal_admission.h"
#include "asset_find_by_hash.h"
#include "nexus_v1_structure3_face_vertex_set_admission.h"
#include "nexus_v1_structure3_face_tail_admission.h"
#include "nexus_v1_structure3_face_index_prefix_admission.h"
#include "nexus_v1_structure3_first_region_row_admission.h"
#include "nexus_v1_structure3_second_region_row_admission.h"
#include "nexus_v1_structure3_third_region_row_admission.h"
#include "nexus_v1_structure1f_wall_decoration_admission.h"
#include "nexus_v1_structure1f_wall_decoration_structure3_row_admission.h"
#include "nexus_v1_structure1f_alcove_admission.h"
#include "nexus_v1_structure1f_alcove_structure3_row_admission.h"
#include "nexus_v1_structure1f_payload_owner_admission.h"

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
    uint8_t data[192] = {0};
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
    Nexus_V1_Structure3ThirdRegionRowAdmissionReceipt third_region_row;
    Nexus_V1_Structure1FDirectoryAdmissionReceipt directory;
    Nexus_V1_Structure1FWallDecorationAdmissionReceipt wall_decoration;
    Nexus_V1_Structure1FWallDecorationStructure3RowAdmissionReceipt wall_row;
    Nexus_V1_Structure1FAlcoveAdmissionReceipt alcove;
    Nexus_V1_Structure1FAlcoveStructure3RowAdmissionReceipt alcove_row;
    Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt floor_payload;
    Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt sensor_payload;
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
    data[112U] = 0x21U;
    data[113U] = 0U;
    write_be16(data + 114U, 0x0042U);
    memset(data + 116U, 0xd4, 8U);
    data[124U] = 0x20U;
    data[125U] = 0U;
    write_be16(data + 126U, 0x0123U);
    memset(data + 128U, 0xe5, 8U);
    data[136U] = 0x11U;
    data[137U] = 7U;
    data[138U] = 9U;
    memset(data + 139U, 0xf1, 9U);
    data[148U] = 0x12U;
    data[149U] = 3U;
    data[150U] = 60U;
    memset(data + 151U, 0xf2, 13U);
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
    memset(&directory, 0, sizeof(directory));
    directory.valid = directory.direct_identity_bound = directory.parser_layout_bound = 1;
    directory.family_directory_bound = directory.no_draw_only = 1;
    directory.level_index = identity.level_index;
    directory.package_fnv1a64 = identity.fnv1a64;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].source_tag = 0x21U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].record_offset = 112U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].record_length = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].record_count = 1U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].record_size = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS].record_fnv1a64 =
        fnv1a64(data + 112U, 12U);
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].source_tag = 0x20U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].record_offset = 124U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].record_length = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].record_count = 1U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].record_size = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_ALCOVES].record_fnv1a64 =
        fnv1a64(data + 124U, 12U);
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].source_tag = 0x11U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].record_offset = 136U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].record_length = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].record_count = 1U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].record_size = 12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS].record_fnv1a64 =
        fnv1a64(data + 136U, 12U);
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].source_tag = 0x12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_offset = 148U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_length = 16U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_count = 1U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_size = 16U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_fnv1a64 =
        fnv1a64(data + 148U, 16U);
    CHECK(nexus_v1_structure1f_wall_decoration_admit(
              &identity, data, sizeof(data), &directory, 0U, &wall_decoration));
    CHECK(nexus_v1_structure1f_alcove_admit(
              &identity, data, sizeof(data), &directory, 0U, &alcove));
    CHECK(nexus_v1_structure1f_payload_owner_admit(
              &identity, data, sizeof(data), &directory,
              NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS, 0U, &floor_payload));
    CHECK(nexus_v1_structure1f_payload_owner_admit(
              &identity, data, sizeof(data), &directory,
              NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS, 0U, &sensor_payload));
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
    CHECK(nexus_v1_structure1f_wall_decoration_structure3_row_admit(
              &identity, data, sizeof(data), &wall_decoration, &entry, &receipt,
              &second_region_row, &wall_row));
    CHECK(nexus_v1_structure1f_alcove_structure3_row_admit(
              &identity, data, sizeof(data), &alcove, &entry, &receipt,
              &second_region_row, &alcove_row));
    CHECK(nexus_v1_structure3_third_region_row_admit(
              &identity, data, sizeof(data), &entry, 0U, &third_region_row));
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
    CHECK(wall_row.valid && wall_row.raw_selector == 0U &&
          wall_row.structure1f_record_offset == 112U &&
          wall_row.structure3_row_ordinal == 0U && wall_row.structure3_row_offset == 88U &&
          !wall_row.face_semantics_permitted && !wall_row.topology_semantics_permitted &&
          !wall_row.geometry_semantics_permitted && !wall_row.material_semantics_permitted &&
          !wall_row.texture_semantics_permitted && !wall_row.draw_permitted);
    wall_decoration.raw_face_selector = 1U;
    CHECK(!nexus_v1_structure1f_wall_decoration_structure3_row_admit(
              &identity, data, sizeof(data), &wall_decoration, &entry, &receipt,
              &second_region_row, &wall_row) && !wall_row.valid);
    wall_decoration.raw_face_selector = 0U;
    CHECK(alcove_row.valid && alcove_row.raw_selector == 0U &&
          alcove_row.structure1f_record_offset == 124U &&
          alcove_row.structure3_row_ordinal == 0U && alcove_row.structure3_row_offset == 88U &&
          !alcove_row.portal_semantics_permitted && !alcove_row.face_semantics_permitted &&
          !alcove_row.topology_semantics_permitted && !alcove_row.geometry_semantics_permitted &&
          !alcove_row.material_semantics_permitted && !alcove_row.texture_semantics_permitted &&
          !alcove_row.draw_permitted);
    alcove.raw_face_selector = 1U;
    CHECK(!nexus_v1_structure1f_alcove_structure3_row_admit(
              &identity, data, sizeof(data), &alcove, &entry, &receipt,
              &second_region_row, &alcove_row) && !alcove_row.valid);
    alcove.raw_face_selector = 0U;
    CHECK(floor_payload.valid && floor_payload.source_tag == 0x11U &&
          floor_payload.record_offset == 136U && floor_payload.payload_offset == 139U &&
          floor_payload.payload_length == 9U &&
          !memcmp(floor_payload.raw_payload, data + 139U, floor_payload.payload_length) &&
          !floor_payload.structure3_relation_permitted && !floor_payload.object_semantics_permitted &&
          !floor_payload.sensor_semantics_permitted && !floor_payload.placement_semantics_permitted &&
          !floor_payload.geometry_semantics_permitted && !floor_payload.material_semantics_permitted &&
          !floor_payload.texture_semantics_permitted && !floor_payload.draw_permitted);
    CHECK(sensor_payload.valid && sensor_payload.source_tag == 0x12U &&
          sensor_payload.record_offset == 148U && sensor_payload.payload_offset == 151U &&
          sensor_payload.payload_length == 13U &&
          !memcmp(sensor_payload.raw_payload, data + 151U, sensor_payload.payload_length) &&
          !sensor_payload.structure3_relation_permitted && !sensor_payload.object_semantics_permitted &&
          !sensor_payload.sensor_semantics_permitted && !sensor_payload.placement_semantics_permitted &&
          !sensor_payload.geometry_semantics_permitted && !sensor_payload.material_semantics_permitted &&
          !sensor_payload.texture_semantics_permitted && !sensor_payload.draw_permitted);
    CHECK(!nexus_v1_structure1f_payload_owner_admit(
              &identity, data, sizeof(data), &directory, 0U, 0U, &floor_payload) &&
          !floor_payload.valid);
    CHECK(!nexus_v1_structure3_second_region_row_admit(
              &identity, data, sizeof(data), &entry, 1U, &second_region_row) &&
          !second_region_row.valid);
    CHECK(third_region_row.valid && third_region_row.row_ordinal == 0U &&
          third_region_row.row_offset == 100U &&
          !memcmp(third_region_row.raw_bytes, data + 100U,
                  sizeof(third_region_row.raw_bytes)) &&
          !third_region_row.vector_semantics_permitted &&
          !third_region_row.lighting_semantics_permitted &&
          !third_region_row.geometry_semantics_permitted &&
          !third_region_row.material_semantics_permitted &&
          !third_region_row.texture_semantics_permitted && !third_region_row.draw_permitted);
    CHECK(!nexus_v1_structure3_third_region_row_admit(
              &identity, data, sizeof(data), &entry, 1U, &third_region_row) &&
          !third_region_row.valid);
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
