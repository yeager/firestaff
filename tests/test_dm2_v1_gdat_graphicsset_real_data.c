/* Original-data admission audit for a map's exact GRAPHICSSET index.
 *
 * skproject/SKWIN/DME.h::Map_definitions::MapGraphicsStyle() is used
 * unchanged as glbMapGraphicsSet by SkWinCore.cpp (2676:07E9).  A material
 * route may therefore not borrow control words or images from a different
 * set when the map's exact index is incomplete. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_weather_gdat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **out, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *bytes;

    if (!path || !out || !out_size) return 0;
    *out = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
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

static int load_canonical_files(uint8_t **graphics, size_t *graphics_size,
                                uint8_t **dungeon, size_t *dungeon_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    const char *home = getenv("HOME");
    char base[1024];
    char graphics_path[1100];
    char dungeon_path[1100];
    const char *bases[3];
    int base_count = 0;

    if (root && root[0]) bases[base_count++] = root;
    if (home && home[0]) {
        snprintf(base, sizeof(base), "%s/.firestaff/data/dm2/data", home);
        bases[base_count++] = base;
    }
    for (int i = 0; i < base_count; ++i) {
        snprintf(graphics_path, sizeof(graphics_path), "%s/graphics.dat", bases[i]);
        snprintf(dungeon_path, sizeof(dungeon_path), "%s/dungeon.dat", bases[i]);
        if (read_file(graphics_path, graphics, graphics_size) &&
            read_file(dungeon_path, dungeon, dungeon_size)) {
            return 1;
        }
        free(*graphics);
        free(*dungeon);
        *graphics = NULL;
        *dungeon = NULL;
    }
    return 0;
}

static int graphicsset_has_scene_controls(const DM2_V1_AssetLoader *loader,
                                           int graphicsset)
{
    static const int fields[] = {
        DM2_GDAT_GFXSET_SCENE_COLORKEY,
        DM2_GDAT_GFXSET_SCENE_FLAGS
    };
    uint16_t value;
    for (size_t i = 0; i < sizeof(fields) / sizeof(fields[0]); ++i) {
        if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_GRAPHICSSET,
                                          graphicsset, fields[i], &value)) {
            return 0;
        }
    }
    return 1;
}

static int source_img3_bits_per_pixel(const uint8_t *raw, size_t raw_size,
                                      unsigned int *out_bits_per_pixel)
{
    uint16_t cy;
    uint16_t w4;
    int offset_y;

    if (!raw || raw_size < 6u || !out_bits_per_pixel) return 0;
    cy = (uint16_t)raw[2] | ((uint16_t)raw[3] << 8);
    w4 = (uint16_t)raw[4] | ((uint16_t)raw[5] << 8);
    offset_y = ((int)(int16_t)cy) >> 10;
    if (offset_y == 31) {
        *out_bits_per_pixel = 8u;
        return 1;
    }
    if (offset_y == -32) {
        if (w4 != 4u && w4 != 8u) return 0;
        *out_bits_per_pixel = w4;
        return 1;
    }
    *out_bits_per_pixel = 4u;
    return 1;
}

static int verify_source_c8_selector_gate(DM2_V1_AssetLoader *loader,
                                          uint8_t *graphics,
                                          size_t graphics_size,
                                          const uint8_t *raw,
                                          size_t raw_size,
                                          int graphicsset,
                                          int field,
                                          int *out_checks)
{
    unsigned int bits_per_pixel = 0u;
    size_t raw_offset;
    uint8_t saved_selector;
    uint8_t *decoded;

    if (!loader || !graphics || !raw || raw_size < 8u || !out_checks ||
        !source_img3_bits_per_pixel(raw, raw_size, &bits_per_pixel)) {
        return 0;
    }
    if (bits_per_pixel != 8u) return 1;
    if (raw < graphics || raw >= graphics + graphics_size) return 0;
    raw_offset = (size_t)(raw - graphics);
    if (raw_offset > graphics_size - 8u ||
        (graphics[raw_offset + 6u] != 1u &&
         graphics[raw_offset + 6u] != 2u &&
         graphics[raw_offset + 6u] != 3u)) {
        return 0;
    }

    /* skproject DECODE_IMG9 accepts selectors 1, 2, and 3. A source C8
     * plane with any other selector must fail before it reaches a scene. */
    saved_selector = graphics[raw_offset + 6u];
    graphics[raw_offset + 6u] = 0u;
    decoded = dm2_v1_asset_load_image_field(
        loader, DM2_GDAT_CATEGORY_GRAPHICSSET, graphicsset, field,
        NULL, NULL, NULL);
    graphics[raw_offset + 6u] = saved_selector;
    if (decoded) {
        free(decoded);
        return 0;
    }
    ++*out_checks;
    return 1;
}

int main(void)
{
    uint8_t *graphics = NULL;
    uint8_t *dungeon_bytes = NULL;
    size_t graphics_size = 0u;
    size_t dungeon_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    int referenced[0x100] = { 0 };
    int exact_material_sets = 0;
    int referenced_sets = 0;
    int c8_selector_checks = 0;
    int failures = 0;

    if (!load_canonical_files(&graphics, &graphics_size,
                              &dungeon_bytes, &dungeon_size)) {
        puts("SKIP: no local canonical DM2 data");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&dungeon, 0, sizeof(dungeon));
    if (dm2_v1_asset_loader_init(&loader, graphics, graphics_size) != 0 ||
        dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)dungeon_size) != 0 ||
        dungeon_bytes[2] != 0x47u || dungeon_bytes[3] != 0x31u) {
        fputs("FAIL: canonical G1/GDAT source was not accepted\n", stderr);
        dm2_v1_asset_loader_free(&loader);
        dm2_v1_dungeon_free(&dungeon);
        free(graphics);
        free(dungeon_bytes);
        return 1;
    }
    for (int level = 0; level < dungeon.level_count; ++level) {
        int style = dm2_v1_dungeon_get_map_graphics_style(&dungeon, level);
        if (style < 0 || style > 0xff) {
            ++failures;
        } else {
            referenced[style] = 1;
        }
    }
    for (int style = 0; style <= 0xff; ++style) {
        DM2_V1_GdatImageMetadata floor;
        DM2_V1_GdatImageMetadata ceiling;
        const uint8_t *floor_raw;
        const uint8_t *ceiling_raw;
        size_t floor_size = 0u;
        size_t ceiling_size = 0u;
        int floor_type = -1;
        int ceiling_type = -1;
        unsigned int floor_data_index = 0u;
        unsigned int ceiling_data_index = 0u;
        unsigned int source_floor_bpp = 0u;
        unsigned int source_ceiling_bpp = 0u;
        int decoded_floor_width = 0;
        int decoded_floor_height = 0;
        int decoded_ceiling_width = 0;
        int decoded_ceiling_height = 0;
        DM2_ImageFormat decoded_floor_format = DM2_IMG_FMT_UNKNOWN;
        DM2_ImageFormat decoded_ceiling_format = DM2_IMG_FMT_UNKNOWN;
        uint8_t *decoded_floor = NULL;
        uint8_t *decoded_ceiling = NULL;
        uint16_t ambient_darkness = 0u;
        uint16_t ambient_light = 0u;
        int has_ambient_light;
        int has_ambient_darkness;
        int complete = dm2_v1_asset_load_image_metadata(
                &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
                DM2_GDAT_GFXSET_FLOOR, &floor) &&
            dm2_v1_asset_load_image_metadata(
                &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
                DM2_GDAT_GFXSET_CEIL, &ceiling) &&
            graphicsset_has_scene_controls(&loader, style);
        floor_raw = dm2_v1_asset_load_sized(&loader,
                                             DM2_GDAT_CATEGORY_GRAPHICSSET,
                                             style, DM2_GDAT_GFXSET_FLOOR,
                                             &floor_size);
        ceiling_raw = dm2_v1_asset_load_sized(&loader,
                                               DM2_GDAT_CATEGORY_GRAPHICSSET,
                                               style, DM2_GDAT_GFXSET_CEIL,
                                               &ceiling_size);
        has_ambient_darkness = dm2_v1_asset_load_word_value(
            &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
            DM2_GDAT_GFXSET_AMBIANT_DARKNESS, &ambient_darkness);
        has_ambient_light = dm2_v1_asset_load_word_value(
            &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
            DM2_GDAT_GFXSET_AMBIANT_LIGHT, &ambient_light);
        for (uint16_t entry = 0u; entry < loader.entry_count; ++entry) {
            const DM2_V1_GdatEntry *candidate = &loader.entries[entry];
            if (candidate->cls1 != DM2_GDAT_CATEGORY_GRAPHICSSET ||
                candidate->cls2 != (uint8_t)style) {
                continue;
            }
            if (candidate->cls3 != DM2_GDAT_ENTRY_TYPE_IMAGE) {
                continue;
            }
            if (candidate->cls4 == DM2_GDAT_GFXSET_FLOOR) {
                floor_type = candidate->cls3;
                floor_data_index = candidate->data_index;
            } else if (candidate->cls4 == DM2_GDAT_GFXSET_CEIL) {
                ceiling_type = candidate->cls3;
                ceiling_data_index = candidate->data_index;
            }
        }
        if (floor_raw && ceiling_raw &&
            (!source_img3_bits_per_pixel(floor_raw, floor_size,
                                         &source_floor_bpp) ||
             !source_img3_bits_per_pixel(ceiling_raw, ceiling_size,
                                         &source_ceiling_bpp) ||
             floor.bits_per_pixel != source_floor_bpp ||
             ceiling.bits_per_pixel != source_ceiling_bpp)) {
            ++failures;
        }
        if (referenced[style]) {
            DM2_V1_GdatSceneM11CommandPlan command_plan;
            memset(&command_plan, 0, sizeof(command_plan));
            /* skproject EXTRACT_GDAT_IMAGE dispatches IMG3::Getpf() to
             * DECODE_IMG9 or DECODE_IMG3_UNDERLAY_LOCAL before dungeon
             * planes can reach c_gui_vp. Metadata alone is not admission. */
            decoded_floor = dm2_v1_asset_load_image_field(
                &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
                DM2_GDAT_GFXSET_FLOOR, &decoded_floor_width,
                &decoded_floor_height, &decoded_floor_format);
            decoded_ceiling = dm2_v1_asset_load_image_field(
                &loader, DM2_GDAT_CATEGORY_GRAPHICSSET, style,
                DM2_GDAT_GFXSET_CEIL, &decoded_ceiling_width,
                &decoded_ceiling_height, &decoded_ceiling_format);
            if (!decoded_floor || !decoded_ceiling ||
                decoded_floor_width != (int)floor.width ||
                decoded_floor_height != (int)floor.height ||
                decoded_ceiling_width != (int)ceiling.width ||
                decoded_ceiling_height != (int)ceiling.height ||
                decoded_floor_format == DM2_IMG_FMT_UNKNOWN ||
                decoded_ceiling_format == DM2_IMG_FMT_UNKNOWN) {
                ++failures;
            }
            if (!dm2_v1_gdat_scene_m11_command_plan_build(
                    &loader, (uint8_t)style, &command_plan) ||
                !command_plan.valid || command_plan.command_hash == 0u ||
                command_plan.commands[0].field != DM2_GDAT_GFXSET_FLOOR ||
                command_plan.commands[1].field != DM2_GDAT_GFXSET_CEIL ||
                !command_plan.commands[0].material_source_bytes ||
                !command_plan.commands[1].material_source_bytes ||
                !command_plan.commands[0].material_source_byte_count ||
                !command_plan.commands[1].material_source_byte_count ||
                !command_plan.commands[0].material_receipt_hash ||
                !command_plan.commands[1].material_receipt_hash ||
                !command_plan.commands[0].pixels || !command_plan.commands[1].pixels ||
                command_plan.commands[0].palette_hash == 0u ||
                command_plan.commands[1].palette_hash == 0u ||
                (has_ambient_light
                    ? command_plan.ambient_light != ambient_light
                    : command_plan.ambient_light != 0u) ||
                !has_ambient_darkness || ambient_darkness > 8u ||
                command_plan.ambient_darkness != ambient_darkness) {
                ++failures;
            }
            dm2_v1_gdat_scene_m11_command_plan_free(&command_plan);
            if (!verify_source_c8_selector_gate(
                    &loader, graphics, graphics_size, floor_raw, floor_size,
                    style, DM2_GDAT_GFXSET_FLOOR, &c8_selector_checks) ||
                !verify_source_c8_selector_gate(
                    &loader, graphics, graphics_size, ceiling_raw, ceiling_size,
                    style, DM2_GDAT_GFXSET_CEIL, &c8_selector_checks)) {
                ++failures;
            }
        }
        if (complete) ++exact_material_sets;
        if (referenced[style]) {
            ++referenced_sets;
            printf("style=%d exact=%d floor(type=%d,data=%u)=%ux%u "
                   "ceil(type=%d,data=%u)=%ux%u\n", style,
                   complete, floor_type, floor_data_index, floor.width,
                   floor.height, ceiling_type, ceiling_data_index,
                   ceiling.width, ceiling.height);
            if (floor_raw && floor_size >= 6u && ceiling_raw && ceiling_size >= 6u) {
                printf("  raw floor=%zu [%02x %02x %02x %02x %02x %02x]"
                       " ceil=%zu [%02x %02x %02x %02x %02x %02x]\n",
                       floor_size, floor_raw[0], floor_raw[1], floor_raw[2],
                       floor_raw[3], floor_raw[4], floor_raw[5], ceiling_size,
                       ceiling_raw[0], ceiling_raw[1], ceiling_raw[2],
                       ceiling_raw[3], ceiling_raw[4], ceiling_raw[5]);
            }
            if (!complete) ++failures;
        }
        free(decoded_floor);
        free(decoded_ceiling);
    }
    printf("entries=%u referenced=%d exact-material-sets=%d "
           "c8-selector-gates=%d\n", (unsigned int)loader.entry_count,
           referenced_sets, exact_material_sets, c8_selector_checks);
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
    free(dungeon_bytes);
    if (failures != 0 || referenced_sets == 0 || exact_material_sets == 0 ||
        c8_selector_checks == 0) {
        fputs("FAIL: a map GRAPHICSSET cannot borrow a different set\n", stderr);
        return 1;
    }
    puts("PASS: every referenced G1 MapGraphicsStyle owns exact, decodable scene material");
    return 0;
}
