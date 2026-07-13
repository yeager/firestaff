#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(c, m) do { if (!(c)) { printf("FAIL: %s\n", m); ++failures; } } while (0)

int main(void) {
    Nexus_V1_Level level;
    Nexus_V1_DgnStructure1FSpatialReceipt receipt;

    memset(&level, 0, sizeof(level));
    level.width = NEXUS_MAX_MAP_SIZE;
    level.height = NEXUS_MAX_MAP_SIZE;
    level.geometry_info.structure1f_valid = 1;
    level.geometry_info.structure1f_total_entry_count = 3;
    level.structure1f_entry_count = 3;
    level.structure1f_entries[0].family = NEXUS_V1_DGN_STRUCTURE1F_ITEMS;
    level.structure1f_entries[0].x = 4;
    level.structure1f_entries[0].y = 7;
    level.structure1f_entries[1].family = NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS;
    level.structure1f_entries[1].x = 4;
    level.structure1f_entries[1].y = 7;
    level.structure1f_entries[2].family = NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS;
    level.structure1f_entries[2].x = 5;
    level.structure1f_entries[2].y = 7;

    CHECK(nexus_v1_level_structure1f_spatial_receipt(&level, &receipt) == 0,
          "Structure1F spatial receipt builds");
    CHECK(receipt.valid && receipt.direct_coordinate_entry_count == 3 &&
          receipt.direct_coordinate_unique_cell_count == 2 &&
          receipt.direct_coordinate_duplicate_cell_count == 1,
          "direct Structure1F coordinates retain bounded unique-cell coverage");
    CHECK(receipt.structure1a_bound_entry_count == 0,
          "direct-coordinate receipt does not promote Structure1A-bound records");
    if (failures) return 1;
    puts("test_nexus_v1_structure1f_spatial_receipt: PASS");
    return 0;
}
