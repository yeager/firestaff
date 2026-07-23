/*
 * firestaff_dm2_v1_draw_item_source_pass_probe.c — DM2 V1 DRAW_ITEM source
 * pass verification probe.
 *
 * Exercises the source-locked DRAW_ITEM/DRAW_STATIC_OBJECT helpers
 * (5x5 anchor rotation, visibility mask, display order, draw positions,
 * view-rotated source plan and the M11 delivery gate) against every
 * runtime-admitted map of the verified DM2 PC G1 DUNGEON.DAT.
 *
 * Source: skproject/SKWIN/SkWinCore.cpp DRAW_ITEM (_32cb_3672),
 * DRAW_PUT_DOWN_ITEM (_32cb_3991), DRAW_STATIC_OBJECT (_32cb_3b9d),
 * QUERY_OBJECT_5x5_POS (_48ae_07fd), DIR_FROM_5x5_POS (_48ae_07bf) and
 * SkGlobal.cpp _4976_4a04/tlbDisplayOrder*.
 *
 * Run:
 *   ./build/firestaff_dm2_v1_draw_item_source_pass_probe [dm2-data-root]
 */

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

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out = bytes;
    *out_size = (size_t)size;
    return 1;
}

static const char *resolve_dm2_data_root(int argc, char **argv,
                                         char *buf, size_t buf_size)
{
    const char *root;
    const char *home;
    if (argc >= 2) return argv[1];
    root = getenv("FIRESTAFF_DM2_DATA_DIR");
    if (root && root[0]) return root;
    home = getenv("HOME");
    if (home && home[0]) {
        snprintf(buf, buf_size, "%s/.firestaff/data/dm2/data", home);
        return buf;
    }
    return NULL;
}

static uint32_t square_mask(const DM2_V1_G1RuntimeMapWeaponReceipt *weapons,
                            const DM2_V1_G1RuntimeMapContainerReceipt *containers,
                            int x, int y, int view_dir)
{
    uint32_t mask = 0u;
    int j;

    for (j = 0; j < weapons->weapon_root_count; ++j) {
        if (weapons->weapons[j].x == x && weapons->weapons[j].y == y) {
            mask |= dm2_v1_viewport_static_object_visibility_bit(
                weapons->weapons[j].direction, view_dir);
        }
    }
    for (j = 0; j < containers->container_root_count; ++j) {
        if (containers->containers[j].x == x && containers->containers[j].y == y) {
            mask |= dm2_v1_viewport_static_object_visibility_bit(
                containers->containers[j].direction, view_dir);
        }
    }
    return mask;
}

static void probe_record(int map, int x, int y, int direction, int category,
                         const DM2_V1_G1RuntimeMapWeaponReceipt *weapons,
                         const DM2_V1_G1RuntimeMapContainerReceipt *containers)
{
    int view;

    PROBE_ASSERT(direction <= 3,
                 "map %d record (%d,%d) direction %d is source-bounded",
                 map, x, y, direction);
    for (view = 0; view < 4; ++view) {
        int pos = dm2_v1_viewport_object_5x5_pos(direction, view);
        uint32_t bit = dm2_v1_viewport_static_object_visibility_bit(
            direction, view);
        uint32_t mask = square_mask(weapons, containers, x, y, view);
        uint8_t positions[25];
        int draw_count;
        int found;
        int i;
        DM2_V1_StaticObjectSourcePlan plan;

        PROBE_ASSERT(pos == 6 || pos == 8 || pos == 16 || pos == 18,
                     "map %d record (%d,%d) dir %d view %d anchors at a source "
                     "corner 5x5 position (%d)", map, x, y, direction, view, pos);
        PROBE_ASSERT(bit == (1u << (unsigned)pos),
                     "map %d record (%d,%d) dir %d view %d visibility bit is "
                     "its anchor bit", map, x, y, direction, view);
        PROBE_ASSERT((mask & bit) == bit,
                     "map %d square (%d,%d) view %d mask contains the record's "
                     "own bit", map, x, y, view);
        PROBE_ASSERT(dm2_v1_viewport_dir_from_5x5_pos(pos) ==
                         ((direction - view) & 3),
                     "map %d record (%d,%d) dir %d view %d DIR_FROM_5x5_POS "
                     "matches the view-relative direction",
                     map, x, y, direction, view);

        /* DRAW_STATIC_OBJECT draws the masked positions in source display
         * order; the record's own position must survive the filter. */
        draw_count = dm2_v1_viewport_static_object_draw_positions(
            3, mask, positions);
        found = 0;
        for (i = 0; i < draw_count; ++i) {
            if (positions[i] == pos) found = 1;
            PROBE_ASSERT((mask & (1u << positions[i])) != 0u,
                         "map %d square (%d,%d) view %d draw position %d is "
                         "mask-covered", map, x, y, view, positions[i]);
        }
        PROBE_ASSERT(found,
                     "map %d record (%d,%d) dir %d view %d survives the "
                     "DRAW_STATIC_OBJECT display-order filter",
                     map, x, y, direction, view);

        /* The view-rotated source plan must accept the real mask on both
         * proven centre cells (D1C pass 17 / D2C pass 14). */
        PROBE_ASSERT(dm2_v1_viewport_static_object_source_plan(
                         3, 17, category, direction, 0, 0, view, 1u, mask,
                         &plan) == 1 &&
                     plan.position_5x5 == pos &&
                     (plan.visibility_mask_5x5 &
                      (1u << (unsigned)plan.position_5x5)) != 0u,
                     "map %d record (%d,%d) dir %d view %d D1C source plan "
                     "accepts the real mask", map, x, y, direction, view);
        PROBE_ASSERT(dm2_v1_viewport_static_object_source_plan(
                         6, 14, category, direction, 0, 0, view, 1u, mask,
                         &plan) == 1 &&
                     plan.position_5x5 == pos,
                     "map %d record (%d,%d) dir %d view %d D2C source plan "
                     "accepts the real mask", map, x, y, direction, view);
    }
}

int main(int argc, char **argv)
{
    char root_buf[1024];
    char dungeon_path[1024];
    const char *root = resolve_dm2_data_root(argc, argv, root_buf,
                                             sizeof(root_buf));
    uint8_t *dungeon_bytes = NULL;
    size_t dungeon_size = 0u;
    DM2_V1_DungeonData dungeon;
    int map;
    int record_total = 0;

    /* Synthetic source-table invariants first (no game data required). */
    {
        static const uint8_t expect_center[25] = {
            0, 4, 1, 3, 2, 5, 9, 6, 8, 7, 10, 14, 11,
            13, 12, 15, 19, 16, 18, 17, 20, 24, 21, 23, 22
        };
        uint8_t order[25];
        uint8_t positions[25];
        int count;

        PROBE_ASSERT(dm2_v1_viewport_static_object_display_order(3, order) ==
                         25 && memcmp(order, expect_center, 25) == 0,
                     "D1C display order matches tlbDisplayOrderCenter");
        PROBE_ASSERT(dm2_v1_viewport_static_object_display_order(0, order) ==
                         15,
                     "cell 0 iterates only the first 15 display-order entries");
        count = dm2_v1_viewport_static_object_draw_positions(
            3, (1u << 6) | (1u << 18), positions);
        PROBE_ASSERT(count == 2 && positions[0] == 6 && positions[1] == 18,
                     "draw positions follow the source center display order");
    }

    if (!root) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    if (!read_file(dungeon_path, &dungeon_bytes, &dungeon_size)) {
        puts("SKIP: no local canonical DM2 data");
        free(dungeon_bytes);
        return 0;
    }
    memset(&dungeon, 0, sizeof(dungeon));
    if (dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        puts("SKIP: no local canonical DM2 data");
        free(dungeon_bytes);
        return 0;
    }

    for (map = 0; map < dungeon.level_count; ++map) {
        DM2_V1_G1RuntimeMapWeaponReceipt weapons;
        DM2_V1_G1RuntimeMapContainerReceipt containers;
        int i;

        memset(&weapons, 0, sizeof(weapons));
        memset(&containers, 0, sizeof(containers));
        if (!dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
                &dungeon, map, &weapons) || !weapons.committed ||
            !dm2_v1_dungeon_materialize_g1_runtime_map_containers(
                &dungeon, map, &containers) || !containers.committed) {
            continue;
        }
        for (i = 0; i < weapons.weapon_root_count; ++i) {
            probe_record(map, weapons.weapons[i].x, weapons.weapons[i].y,
                         weapons.weapons[i].direction, 0x10,
                         &weapons, &containers);
            ++record_total;
        }
        for (i = 0; i < containers.container_root_count; ++i) {
            probe_record(map, containers.containers[i].x,
                         containers.containers[i].y,
                         containers.containers[i].direction, 0x14,
                         &weapons, &containers);
            ++record_total;
        }
    }

    printf("DM2 V1 DRAW_ITEM source pass probe: %d passed, %d failed "
           "(%d real DB5/DB9 records across %d maps)\n",
           passed, errors, record_total, dungeon.level_count);
    dm2_v1_dungeon_free(&dungeon);
    free(dungeon_bytes);
    return errors == 0 ? 0 : 1;
}
