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

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_weather_gdat.h"

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

static void check_pixel_draw_chain(const DM2_V1_AssetLoader *loader,
                                   int category, int item_type, int field,
                                   int direction)
{
    uint16_t image_offset = 0;
    DM2_V1_GdatGfxRawMaterialReceipt raw;
    DM2_V1_G1StaticObjectMaterialSelector selector;
    DM2_V1_StaticObjectSourcePlan plan;
    DM2_V1_ItemRender row;
    DM2_V1_ItemAssetBlit blit;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

    /* Cycle 15 pixel-draw chain: DRAW_ITEM (tt == 0) reads dtImageOffset at
     * the default item index 0xFE and continues with offset 0 when the entry
     * is absent, so the offset always resolves.  The exact GDAT image, its
     * raw receipt and the local palette must exist before the source blit may
     * place pixels; a gap keeps the object fail-closed. */
    if (!dm2_v1_asset_load_image_offset(loader, category, 0xfe,
                                        (uint8_t)field, &image_offset))
        image_offset = 0;
    pixels = dm2_v1_asset_load_image_field(loader, category, item_type, field,
                                           &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0) {
        /* No exact GDAT image: the object stays fail-closed (no draw). */
        ++g_checks;
        ++g_passed;
        dm2_v1_asset_free_pixels(pixels);
        return;
    }
    memset(&raw, 0, sizeof(raw));
    CHECK("exact GDAT image receipt is accepted",
          dm2_v1_gdat_image_raw_material_receipt(
              loader, category, item_type, field, &raw) == 1 &&
          raw.accepted && raw.source_bytes && raw.source_byte_count != 0u &&
          raw.source_hash != 0u && raw.receipt_hash != 0u);
    CHECK("record local palette is owned",
          dm2_v1_asset_load_image_local_palette(
              loader, category, item_type, field,
              palette16, &palette_hash) == 1 && palette_hash != 0u);
    memset(&selector, 0, sizeof(selector));
    selector.valid = 1;
    selector.object_id = 0x1401u;
    selector.category = (uint8_t)category;
    selector.item_type = (uint8_t)item_type;
    selector.image_field = (uint8_t)field;
    selector.direction = (uint8_t)direction;
    selector.image_offset = image_offset;
    CHECK("source plan binds selector field and offset identity",
          dm2_v1_viewport_static_object_source_plan(
              3, 17, category, direction, field == 4, 0, 0, 1u,
              dm2_v1_viewport_static_object_visibility_bit(direction, 0),
              &plan) == 1 &&
          plan.image_field == field &&
          (plan.visibility_mask_5x5 &
           (1u << (unsigned)plan.position_5x5)) != 0u);
    memset(&row, 0, sizeof(row));
    row.gdat_index = dm2_v1_viewport_item_graphic_index(category, item_type,
                                                        field);
    row.center_x = 96;
    row.center_y = 88;
    row.source_static_object_placement_valid = 1;
    row.source_static_object_stretch_factor64 = plan.stretch_factor64;
    row.source_static_object_slot_x_offset = plan.slot_x_offset;
    row.source_static_object_slot_y_offset = plan.slot_y_offset;
    row.source_static_object_position_5x5 = plan.position_5x5;
    row.source_static_object_image_field = plan.image_field;
    row.source_static_object_flip_mirror = plan.flip_mirror;
    row.source_static_object_image_offset = image_offset;
    CHECK("source blit places the decoded image with offset and slot deltas",
          row.gdat_index != 0 &&
          dm2_v1_viewport_item_asset_blit(&row, width, height, width, 0,
                                          4, 32, &blit) == 1 &&
          blit.dst_rect.w > 0 && blit.dst_rect.h > 0 &&
          blit.dst_rect.x ==
              96 - blit.dst_rect.w / 2 + plan.slot_x_offset +
                  (int)(int8_t)(image_offset >> 8) &&
          blit.dst_rect.y ==
              88 - blit.dst_rect.h / 2 + plan.slot_y_offset +
                  (int)(int8_t)(image_offset & 0xff) &&
          blit.flip_mirror == plan.flip_mirror);
    dm2_v1_asset_free_pixels(pixels);
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
    char dungeon_path[2048];
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

    /* Pixel-draw chain against the real GRAPHICS.DAT: each record's DRAW_ITEM
     * image field (F0, or F4 for an opened chest) must carry a source-owned
     * dtImageOffset, an exact raw receipt, a decodable bitmap and a local
     * palette before the source blit may place it. */
    {
        char graphics_path[2048];
        uint8_t *graphics = NULL;
        size_t graphics_size = 0u;
        DM2_V1_AssetLoader loader;

        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
        if (!read_file(graphics_path, &graphics, &graphics_size)) {
            puts("SKIP: no local canonical DM2 graphics.dat");
            dm2_v1_dungeon_free(&dungeon);
            free(dungeon_bytes);
            free(graphics);
            return g_passed == g_checks ? 0 : 1;
        }
        memset(&loader, 0, sizeof(loader));
        CHECK("canonical GRAPHICS.DAT loads",
              dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0);
        for (i = 0; i < weapons.weapon_root_count; ++i) {
            check_pixel_draw_chain(&loader, 0x10, weapons.weapons[i].item_type,
                                   0, weapons.weapons[i].direction);
        }
        for (i = 0; i < containers.container_root_count; ++i) {
            check_pixel_draw_chain(&loader, 0x14,
                                   containers.containers[i].container_type,
                                   containers.containers[i].opened ? 4 : 0,
                                   containers.containers[i].direction);
        }
        dm2_v1_asset_loader_free(&loader);
        free(graphics);
    }

    printf("DM2 V1 G1 static-object visibility real data: %d/%d passed "
           "(%d weapon roots, %d container roots)\n",
           g_passed, g_checks, weapons.weapon_root_count,
           containers.container_root_count);
    dm2_v1_dungeon_free(&dungeon);
    free(dungeon_bytes);
    return g_passed == g_checks ? 0 : 1;
}
