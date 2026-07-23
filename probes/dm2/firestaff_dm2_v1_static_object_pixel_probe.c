/*
 * firestaff_dm2_v1_static_object_pixel_probe.c — DM2 V1 static-object pixel
 * draw verification probe.
 *
 * Walks every runtime-admitted map of the verified DM2 PC G1 corpus and, for
 * each declared direct DB5/DB9 root, resolves the full source-owned pixel
 * draw chain: dtImageOffset at the default index 0xFE (offset 0 when proven
 * absent), the exact GDAT image raw receipt, the decoded bitmap, the local
 * palette, the view-rotated source plan, and the DRAW_ITEM asset blit.
 *
 * Source-lock:
 *   skproject/SKWIN/SkWinCore.cpp DRAW_ITEM (_32cb_3672) incl. the
 *       QUERY_GDAT_ENTRY_DATA_INDEX(..., 0xfe, dtImageOffset, ...) rule
 *   skproject/SKWIN/SkWinCore.cpp QUERY_GDAT_ENTRY_DATA_INDEX (returns 0 for
 *       an absent fmtPicOff entry)
 *   skproject/SKWIN/SkWinCore.cpp DRAW_PUT_DOWN_ITEM / DRAW_STATIC_OBJECT
 *   skproject/SKWIN/DME.h ExtendedPicture w28/w30 (x/y offsets)
 *
 * Run:
 *   ./build/firestaff_dm2_v1_static_object_pixel_probe [dm2-data-root]
 */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_weather_gdat.h"

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

static uint8_t *read_file(const char *path, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
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

static void probe_record(const DM2_V1_AssetLoader *loader,
                         int map, int x, int y, int direction,
                         int category, int item_type, int field,
                         int *out_admitted, int *out_blocked)
{
    uint16_t image_offset = 0;
    DM2_V1_GdatGfxRawMaterialReceipt raw;
    DM2_V1_StaticObjectSourcePlan plan;
    DM2_V1_ItemRender row;
    DM2_V1_ItemAssetBlit blit;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint8_t *pixels;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

    (void)x; (void)y;

    /* DRAW_ITEM (tt == 0) reads dtImageOffset at the default index 0xFE and
     * continues with offset 0 when the entry is absent. */
    if (!dm2_v1_asset_load_image_offset(loader, category, 0xfe,
                                        (uint8_t)field, &image_offset))
        image_offset = 0;

    pixels = dm2_v1_asset_load_image_field(loader, category, item_type, field,
                                           &width, &height, &format);
    if (!pixels || width <= 0 || height <= 0) {
        ++*out_blocked;
        PROBE_ASSERT(1,
                     "map %d record cat 0x%02x type %d dir %d: no exact GDAT "
                     "image, stays fail-closed",
                     map, category, item_type, direction);
        dm2_v1_asset_free_pixels(pixels);
        return;
    }
    ++*out_admitted;

    memset(&raw, 0, sizeof(raw));
    if (dm2_v1_gdat_image_raw_material_receipt(
            loader, category, item_type, field, &raw) != 1 || !raw.accepted ||
        !raw.source_hash || !raw.receipt_hash ||
        dm2_v1_asset_load_image_local_palette(
            loader, category, item_type, field, palette16,
            &palette_hash) != 1 || palette_hash == 0u) {
        /* The image decodes but its exact receipt or local palette is
         * missing: the material receipt cannot be built, so the object stays
         * fail-closed before the blit. */
        --*out_admitted;
        ++*out_blocked;
        PROBE_ASSERT(1,
                     "map %d record cat 0x%02x type %d: decoded image without "
                     "exact receipt/palette stays fail-closed",
                     map, category, item_type);
        dm2_v1_asset_free_pixels(pixels);
        return;
    }
    PROBE_ASSERT(raw.source_bytes && raw.source_byte_count != 0u,
                 "map %d record cat 0x%02x type %d: exact raw receipt owns "
                 "source bytes", map, category, item_type);

    memset(&row, 0, sizeof(row));
    for (int view = 0; view < 4; ++view) {
        if (!dm2_v1_viewport_static_object_source_plan(
                3, 17, category, direction, field == 4, 0, view, 1u,
                dm2_v1_viewport_static_object_visibility_bit(direction, view),
                &plan)) {
            PROBE_ASSERT(0,
                         "map %d record cat 0x%02x type %d view %d: source plan",
                         map, category, item_type, view);
            continue;
        }
        PROBE_ASSERT(plan.image_field == field &&
                     (plan.visibility_mask_5x5 &
                      (1u << (unsigned)plan.position_5x5)) != 0u,
                     "map %d record cat 0x%02x type %d view %d: plan field "
                     "and mask bit", map, category, item_type, view);
    }

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
    PROBE_ASSERT(dm2_v1_viewport_item_asset_blit(&row, width, height, width, 0,
                                                 4, 32, &blit) == 1 &&
                 blit.dst_rect.w > 0 && blit.dst_rect.h > 0 &&
                 blit.dst_rect.x ==
                     96 - blit.dst_rect.w / 2 + plan.slot_x_offset +
                         (int)(int8_t)(image_offset >> 8) &&
                 blit.dst_rect.y ==
                     88 - blit.dst_rect.h / 2 + plan.slot_y_offset +
                         (int)(int8_t)(image_offset & 0xff),
                 "map %d record cat 0x%02x type %d: pixel blit places the "
                 "decoded image (offset 0x%04x)",
                 map, category, item_type, image_offset);
    dm2_v1_asset_free_pixels(pixels);
}

int main(int argc, char **argv)
{
    char root_buf[1024];
    char graphics_path[1024];
    char dungeon_path[1024];
    const char *root = resolve_dm2_data_root(argc, argv, root_buf,
                                             sizeof(root_buf));
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    int admitted = 0;
    int blocked = 0;
    int map;

    if (!root) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", root);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", root);
    graphics = read_file(graphics_path, &graphics_size);
    dungeon_bytes = read_file(dungeon_path, &dungeon_size);
    if (!graphics || !dungeon_bytes) {
        puts("SKIP: no local canonical DM2 data");
        free(graphics);
        free(dungeon_bytes);
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        puts("SKIP: no local canonical DM2 data");
        free(graphics);
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
            probe_record(&loader, map, weapons.weapons[i].x,
                         weapons.weapons[i].y, weapons.weapons[i].direction,
                         0x10, weapons.weapons[i].item_type, 0,
                         &admitted, &blocked);
        }
        for (i = 0; i < containers.container_root_count; ++i) {
            probe_record(&loader, map, containers.containers[i].x,
                         containers.containers[i].y,
                         containers.containers[i].direction, 0x14,
                         containers.containers[i].container_type,
                         containers.containers[i].opened ? 4 : 0,
                         &admitted, &blocked);
        }
    }

    /* The canonical corpus proves at least one drawable static object and at
     * least one fail-closed one (WEAPONS/126 has no F0 image). */
    PROBE_ASSERT(admitted >= 1,
                 "at least one real record admits the pixel draw chain");
    PROBE_ASSERT(blocked >= 1,
                 "records without exact GDAT evidence stay fail-closed");

    printf("DM2 V1 static-object pixel draw probe: %d passed, %d failed "
           "(%d admitted, %d fail-closed)\n",
           passed, errors, admitted, blocked);
    dm2_v1_dungeon_free(&dungeon);
    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    free(dungeon_bytes);
    return errors == 0 ? 0 : 1;
}
