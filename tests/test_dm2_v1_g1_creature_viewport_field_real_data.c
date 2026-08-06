/* Canonical PC G1 real-data proof for the creature viewport field route and
 * the occupancy/flying-item evidence boundaries.
 *
 * The FB/FC/FD V5 animation chain (SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST's
 * live non-static route) selects a concrete CREATURES/type/dtImage field for
 * records whose type owns the tables; records without the chain stay on the
 * map-chip route or fail closed.  The canonical corpus has no direct
 * dbMissile/dbCloud roots, so the DRAW_FLYING_ITEM pass stays fail-closed on
 * this data. */

#include "dm2_v1_boot.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_dungeon_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_checks;
static int g_passed;

#define CHECK(label, condition) do { \
    ++g_checks; \
    if (condition) ++g_passed; \
    else fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); \
} while (0)

static const char *data_root(char *fallback, size_t fallback_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");

    if (root && root[0]) return root;
    if (!home || !home[0]) return NULL;
    snprintf(fallback, fallback_size, "%s/.firestaff/data/dm2/data", home);
    return fallback;
}

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

int main(void)
{
    DM2_V1_BootProfile profile;
    char fallback[1024];
    const char *root = data_root(fallback, sizeof(fallback));
    int admitted = 0;
    int fail_closed = 0;
    int records = 0;
    int cloud_roots = 0;
    int map;

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
        const uint8_t *rect14_rows = NULL;
        uint32_t rect14_row_count = 0;
        uint32_t rect14_hash = 0;

        CHECK("canonical corpus has no Rect14 table, occupancy anchors stay "
              "unproven",
              dm2_v1_boot_interface_rect14_table(
                  &profile, &rect14_rows, &rect14_row_count,
                  &rect14_hash) == 0);

        for (map = 0; map < dungeon->level_count; ++map) {
            DM2_V1_G1RuntimeMapCreatureReceipt creatures;
            DM2_V1_G1RuntimeMapValidationReceipt validation;
            int column_index = 0;
            int level;
            int x;

            memset(&creatures, 0, sizeof(creatures));
            if (!dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
                    dungeon, map, &creatures) || !creatures.committed) {
                continue;
            }
            for (int i = 0; i < creatures.creature_root_count; ++i) {
                const DM2_V1_G1DirectCreatureRoot *record =
                    &creatures.creatures[i];
                DM2_V1_BootDynamicCreatureMaterialReceipt v5;

                ++records;
                CHECK("record carries its cursor and info-slot fields",
                      record->hit_points_1 != 0u || record->cursor_w8 != 0u ||
                      record->cursor_w10 != 0u || record->info_slot != 0xffu);
                memset(&v5, 0, sizeof(v5));
                if (dm2_v1_boot_dynamic_creature_material_receipt(
                        &profile, record->creature_type, 0, 0xffffu,
                        (0 - record->direction) & 3, &v5) && v5.valid) {
                    ++admitted;
                    CHECK("V5 field route selects a concrete dtImage field, "
                          "never the F9 map chip",
                          v5.image_field != DM2_GDAT_IMG_MAP_CHIP &&
                          v5.image.category == DM2_GDAT_CATEGORY_CREATURES &&
                          v5.image.entry_index == record->creature_type &&
                          v5.image.field == v5.image_field &&
                          v5.image.decoded_hash != 0u &&
                          v5.image.raw_hash != 0u &&
                          v5.palette_hash != 0u &&
                          v5.raw_material_hash != 0u &&
                          v5.raw_material_receipt_hash != 0u &&
                          v5.animation_table_hash != 0u);
                } else {
                    ++fail_closed;
                }
            }

            /* The same corpus scan the renderer relies on: no direct
             * dbMissile (14) or dbCloud (15) roots, so the flying-item pass
             * stays fail-closed on this data. */
            memset(&validation, 0, sizeof(validation));
            if (!dm2_v1_dungeon_validate_g1_runtime_map(
                    dungeon, map, &validation) || !validation.committed) {
                continue;
            }
            for (level = 0; level < map; ++level) {
                column_index += dungeon->level_widths[level];
            }
            for (x = 0; x < validation.width; ++x) {
                int stack = (int)rd16(
                    dungeon->raw_data + dungeon->column_index_base +
                    (column_index + x) * 2);
                int y;
                for (y = 0; y < validation.height; ++y) {
                    int raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                    uint16_t root_id;
                    int type;
                    if (raw < 0 || (raw & 0x10) == 0) {
                        ++stack;
                        continue;
                    }
                    root_id = rd16(dungeon->raw_data +
                                   dungeon->square_first_thing_base +
                                   stack * 2);
                    type = (root_id >> 10) & 0x0f;
                    if (type == 14 || type == 15) ++cloud_roots;
                    ++stack;
                }
            }
        }
    }

    /* The hash-verified PC corpus does expose authenticated FB/FC/FD V5
     * material (currently CREATURES/02/dtImage/12).  It belongs to that
     * GDAT creature type, not to an arbitrary G1 DB4 record.  None of the
     * current-map roots above owns that chain, so they remain fail-closed. */
    {
        int chain_types = 0;
        for (int type = 0; type < 256 && chain_types < 1; ++type) {
            DM2_V1_BootDynamicCreatureMaterialReceipt v5;
            memset(&v5, 0, sizeof(v5));
            if (dm2_v1_boot_dynamic_creature_material_receipt(
                    &profile, type, 0, 0xffffu, 2, &v5) && v5.valid) {
                ++chain_types;
            }
        }
        CHECK("canonical GDAT exposes an authenticated V5 material chain",
              chain_types == 1);
    }
    CHECK("no current-map creature record owns the full V5 chain",
          admitted == 0);
    CHECK("every creature record without the chain stays fail-closed",
          fail_closed == records);
    CHECK("canonical corpus has no direct dbMissile/dbCloud roots",
          cloud_roots == 0);
    printf("DM2 V1 G1 creature viewport field real data: %d/%d passed "
           "(%d records, %d admitted, %d fail-closed, %d cloud roots)\n",
           g_passed, g_checks, records, admitted, fail_closed, cloud_roots);
    dm2_v1_boot_cleanup(&profile);
    return g_passed == g_checks ? 0 : 1;
}
