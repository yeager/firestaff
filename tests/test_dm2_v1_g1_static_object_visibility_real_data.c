/* Canonical PC G1 real-data proof for the DRAW_ITEM 5x5 visibility mask and
 * view-rotated source plan.  Every declared direct DB5/DB9 root on the
 * runtime-admitted G1 map contributes exactly the source-owned mask bit
 * 1 << QUERY_OBJECT_5x5_POS(record, view_dir) (SKWIN/SkWinCore.cpp lines
 * 45361-45370), and the M11 delivery gate accepts each record's own bit for
 * every party direction.
 *
 * Source: skproject/SKWIN/SkWinCore.cpp DRAW_STATIC_OBJECT (_32cb_3b9d),
 * DRAW_ITEM (_32cb_3672), QUERY_OBJECT_5x5_POS (_48ae_07fd) and
 * SkGlobal.cpp _4976_4a04. */

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

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

static void check_record(int x, int y, int direction, int category,
                         const DM2_V1_G1RuntimeMapWeaponReceipt *weapons,
                         const DM2_V1_G1RuntimeMapContainerReceipt *containers)
{
    int view;

    CHECK("record direction is source-bounded", direction <= 3);
    for (view = 0; view < 4; ++view) {
        int pos = dm2_v1_viewport_object_5x5_pos(direction, view);
        uint32_t bit = dm2_v1_viewport_static_object_visibility_bit(
            direction, view);
        uint32_t mask = square_mask(weapons, containers, x, y, view);
        DM2_V1_StaticObjectSourcePlan plan;

        CHECK("real record anchors at a source corner position",
              pos == 6 || pos == 8 || pos == 16 || pos == 18);
        CHECK("real record visibility bit is its anchor bit",
              bit == (1u << (unsigned)pos));
        CHECK("square mask contains the record's own bit",
              (mask & bit) == bit);
        /* A party two squares south of the record facing north sees it at
         * D1C (cell 3, table1d7029 pass 17); the source plan must accept the
         * real mask and anchor at the record's view-rotated position. */
        if (dm2_v1_viewport_static_object_source_plan(
                3, 17, category, direction, 0, 0, view, 1u, mask,
                &plan) == 1) {
            ++g_checks;
            if (plan.position_5x5 == pos &&
                (plan.visibility_mask_5x5 &
                 (1u << (unsigned)plan.position_5x5)) != 0u) {
                ++g_passed;
            } else {
                fprintf(stderr,
                        "FAIL: plan anchor diverges from visibility bit "
                        "(dir %d view %d)\n", direction, view);
            }
        } else {
            /* D2C-only containers are still proven through cell 6 below. */
            DM2_V1_StaticObjectSourcePlan deep_plan;
            CHECK("record admitted through its proven source cell",
                  dm2_v1_viewport_static_object_source_plan(
                      6, 14, category, direction, 0, 0, view, 1u, mask,
                      &deep_plan) == 1 &&
                  deep_plan.position_5x5 == pos);
        }
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
    DM2_V1_G1RuntimeMapWeaponReceipt weapons;
    DM2_V1_G1RuntimeMapContainerReceipt containers;
    int i;

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
    memset(&weapons, 0, sizeof(weapons));
    memset(&containers, 0, sizeof(containers));
    if (dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        puts("SKIP: no local canonical DM2 data");
        free(dungeon_bytes);
        return 0;
    }
    CHECK("canonical G1 map 17 weapon roots materialize",
          dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
              &dungeon, 17, &weapons) == 1 && weapons.committed &&
          weapons.weapon_root_count >= 1);
    CHECK("canonical G1 map 17 container roots materialize",
          dm2_v1_dungeon_materialize_g1_runtime_map_containers(
              &dungeon, 17, &containers) == 1 && containers.committed);

    for (i = 0; i < weapons.weapon_root_count; ++i) {
        check_record(weapons.weapons[i].x, weapons.weapons[i].y,
                     weapons.weapons[i].direction, 0x10,
                     &weapons, &containers);
    }
    for (i = 0; i < containers.container_root_count; ++i) {
        check_record(containers.containers[i].x, containers.containers[i].y,
                     containers.containers[i].direction, 0x14,
                     &weapons, &containers);
    }

    printf("DM2 V1 G1 static-object visibility real data: %d/%d passed "
           "(%d weapon roots, %d container roots)\n",
           g_passed, g_checks, weapons.weapon_root_count,
           containers.container_root_count);
    dm2_v1_dungeon_free(&dungeon);
    free(dungeon_bytes);
    return g_passed == g_checks ? 0 : 1;
}
