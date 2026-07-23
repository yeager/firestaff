/*
 * firestaff_dm2_v1_creature_occupancy_probe.c — DM2 V1 creature occupancy and
 * V5 viewport field verification probe.
 *
 * Walks every runtime-admitted map of the verified DM2 PC G1 corpus and, for
 * each declared direct DB4 root, proves the record-owned animation cursor
 * fields (b5/w8/w10), resolves the FB/FC/FD V5 viewport field route where the
 * GDAT tables exist, and exercises the _4976_5aa4 occupancy grid coordinate
 * mapping and display-order index from DRAW_STATIC_OBJECT.
 *
 * Source-lock:
 *   skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST / QUERY_CREATURE_5x5_POS
 *   skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT occupancy walk
 *   skproject/SKWIN/DME.h Creature (b5/w8/w10 record fields)
 *
 * Run:
 *   ./build/firestaff_dm2_v1_creature_occupancy_probe [dm2-data-root]
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        passed++; \
    } \
} while (0)

static const char *data_root(int argc, char **argv, char *fallback,
                             size_t fallback_size)
{
    const char *root;
    const char *home;

    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) return root;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(fallback, fallback_size, "%s/.firestaff/data/dm2/data", home);
    return fallback;
}

int main(int argc, char **argv)
{
    DM2_V1_BootProfile profile;
    char fallback[1024];
    const char *root = data_root(argc, argv, fallback, sizeof(fallback));
    int admitted = 0;
    int fail_closed = 0;
    int records = 0;
    int map;

    /* Synthetic source-table invariants first (no game data required). */
    {
        int gx = -1;
        int gy = -1;

        PROBE_ASSERT(dm2_v1_viewport_creature_occupancy_5x5(0xff, 0, 0) == 12,
                     "unallocated info slot centres the occupancy position");
        PROBE_ASSERT(dm2_v1_viewport_creature_occupancy_5x5(6, 0, 1) == 16,
                     "QUERY_CREATURE_5x5_POS rotates the anchor into view space");
        PROBE_ASSERT(dm2_v1_viewport_occupancy_grid_coords(3, 6, &gx, &gy) == 1 &&
                     gx == 9 && gy == 7,
                     "D1C occupancy grid coordinate follows _4976_43f5/_4976_4415");
        PROBE_ASSERT(dm2_v1_viewport_static_object_display_index(3, 22) == 24 &&
                     dm2_v1_viewport_static_object_display_index(0, 15) == -1,
                     "display-order index follows tlbDisplayOrderCenter bounds");
        PROBE_ASSERT(dm2_v1_viewport_flying_item_scale64(0, 2) == -1 &&
                     dm2_v1_viewport_flying_item_scale64(3, 0) == 0x13,
                     "flying-item scale follows _4976_41a9 bands");
        {
            int flip = 0;
            PROBE_ASSERT(dm2_v1_viewport_flying_item_image_field(
                             0, 0, 1, 3, 4, 0, 1, 0, &flip) == 0xc && flip == 0,
                         "flying-item side-on field/mirror matches DRAW_FLYING_ITEM");
        }
    }

    if (!root) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, root) != 0 ||
        dm2_v1_boot_enter_game(&profile) != 0) {
        puts("SKIP: no accepted canonical DM2 profile");
        dm2_v1_boot_cleanup(&profile);
        return 0;
    }

    {
        const DM2_V1_DungeonData *dungeon =
            (const DM2_V1_DungeonData *)profile.dungeon_data;

        for (map = 0; map < dungeon->level_count; ++map) {
            DM2_V1_G1RuntimeMapCreatureReceipt creatures;
            int i;

            memset(&creatures, 0, sizeof(creatures));
            if (!dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
                    dungeon, map, &creatures) || !creatures.committed) {
                continue;
            }
            for (i = 0; i < creatures.creature_root_count; ++i) {
                const DM2_V1_G1DirectCreatureRoot *record =
                    &creatures.creatures[i];
                DM2_V1_BootDynamicCreatureMaterialReceipt v5;
                int cell;
                int pass;
                int gx = -1;
                int gy = -1;

                ++records;
                memset(&v5, 0, sizeof(v5));
                if (dm2_v1_boot_dynamic_creature_material_receipt(
                        &profile, record->creature_type, 0, 0xffffu,
                        (0 - record->direction) & 3, &v5) && v5.valid) {
                    ++admitted;
                    PROBE_ASSERT(v5.image_field != DM2_GDAT_IMG_MAP_CHIP &&
                                 v5.image.decoded_hash != 0u &&
                                 v5.palette_hash != 0u,
                                 "map %d creature type %d admits the V5 field "
                                 "route (dtImage 0x%02x)",
                                 map, record->creature_type, v5.image_field);
                } else {
                    ++fail_closed;
                }

                /* Occupancy geometry around a synthetic D1C party placement
                 * (party two squares south, facing north). */
                if (dm2_v1_viewport_static_object_cell_for_map(
                        record->x, record->y, 0, record->x, record->y + 2,
                        &cell, &pass)) {
                    PROBE_ASSERT(cell == 3 && pass == 17 &&
                                 dm2_v1_viewport_occupancy_grid_coords(
                                     cell, 12, &gx, &gy) == 1 &&
                                 gx == 10 && gy == 6 &&
                                 dm2_v1_viewport_static_object_display_index(
                                     cell, 12) == 14,
                                 "map %d record (%d,%d): D1C occupancy grid "
                                 "coordinate and display index",
                                 map, record->x, record->y);
                }
            }
        }
    }

    PROBE_ASSERT(records >= 10,
                 "canonical corpus yields direct DB4 roots (%d)", records);
    PROBE_ASSERT(fail_closed == records,
                 "every record stays fail-closed on the canonical corpus "
                 "(8bpp global-palette or palette-less V5 images, no local "
                 "palette evidence)");
    PROBE_ASSERT(admitted == 0,
                 "no record leaves the map-chip route without complete GDAT "
                 "evidence");

    printf("DM2 V1 creature occupancy probe: %d passed, %d failed "
           "(%d records, %d V5-admitted, %d fail-closed)\n",
           passed, errors, records, admitted, fail_closed);
    dm2_v1_boot_cleanup(&profile);
    return errors == 0 ? 0 : 1;
}
