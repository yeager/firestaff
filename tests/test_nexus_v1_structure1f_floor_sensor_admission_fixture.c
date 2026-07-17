#include "nexus_v1_structure1f_floor_sensor_admission.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

int main(void)
{
    static const uint8_t row[16] = {
        0x12U, 3U, 60U, 0x80U, 1U, 0x19U, 0x2aU, 0x3bU,
        0x4cU, 0x5dU, 0x6eU, 0x7fU, 0xa0U, 0xb1U, 0xc2U, 0xd3U
    };
    static const char row_md5[] = "5db6febed61faee77a6526606595ed9c";
    char path[] = "/tmp/firestaff-nexus-structure1f-sensor-XXXXXX";
    Nexus_V1_LevCorpusDirectLevelIdentity identity;
    Nexus_V1_LevCorpusDirectLevelIdentity other_identity;
    Nexus_V1_Structure1FDirectoryAdmissionReceipt directory;
    Nexus_V1_Structure1FFloorSensorAdmissionReceipt receipt;
    int file;

    file = mkstemp(path);
    if (file < 0 || write(file, row, sizeof(row)) != (ssize_t)sizeof(row) ||
        close(file) != 0) {
        if (file >= 0) close(file);
        unlink(path);
        return 1;
    }
    memset(&identity, 0, sizeof(identity));
    identity.valid = 1;
    identity.level_index = 5U;
    identity.byte_count = sizeof(row);
    identity.fnv1a64 = fnv1a64(row, sizeof(row));
    memcpy(identity.md5, row_md5, sizeof(row_md5));
    snprintf(identity.direct_path, sizeof(identity.direct_path), "%s", path);
    memset(&directory, 0, sizeof(directory));
    directory.valid = directory.direct_identity_bound = 1;
    directory.parser_layout_bound = directory.family_directory_bound = 1;
    directory.no_draw_only = 1;
    directory.level_index = identity.level_index;
    directory.package_fnv1a64 = identity.fnv1a64;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].source_tag = 0x12U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_count = 1U;
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_size = sizeof(row);
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_length = sizeof(row);
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_fnv1a64 = identity.fnv1a64;

    if (!nexus_v1_structure1f_floor_sensor_admit(
            &identity, row, sizeof(row), &directory, 0U, &receipt) ||
        !receipt.valid || receipt.source_tag != 0x12U || receipt.x != 3U ||
        receipt.y != 60U || receipt.cell_ordinal != 3843U ||
        memcmp(receipt.raw_payload, row + 3U, sizeof(receipt.raw_payload)) != 0 ||
        receipt.face_semantics_permitted || receipt.mesh_semantics_permitted ||
        receipt.texture_semantics_permitted || receipt.draw_permitted) {
        unlink(path);
        return 1;
    }
    other_identity = identity;
    other_identity.level_index++;
    if (nexus_v1_structure1f_floor_sensor_admit(
            &other_identity, row, sizeof(row), &directory, 0U, &receipt) ||
        receipt.valid) {
        unlink(path);
        return 1;
    }
    if (nexus_v1_structure1f_floor_sensor_admit(
            &identity, row, sizeof(row), &directory, 1U, &receipt) || receipt.valid) {
        unlink(path);
        return 1;
    }
    directory.families[NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS].record_length--;
    if (nexus_v1_structure1f_floor_sensor_admit(
            &identity, row, sizeof(row), &directory, 0U, &receipt) || receipt.valid) {
        unlink(path);
        return 1;
    }
    unlink(path);
    puts("Structure1F floor-sensor fixture admission: PASS");
    return 0;
}
