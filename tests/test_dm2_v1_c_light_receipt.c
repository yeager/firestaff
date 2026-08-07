#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else printf("FAIL: %s\n", label); \
} while (0)

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *f;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0u;
    if (!path || !out_data || !out_size) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    size = ftell(f);
    if (size <= 0) {
        fclose(f);
        return 0;
    }
    rewind(f);
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return 0;
    }
    if (fread(data, 1u, (size_t)size, f) != (size_t)size) {
        free(data);
        fclose(f);
        return 0;
    }
    fclose(f);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

static int candidate_path(char *out, size_t out_size, const char *suffix)
{
    const char *data = getenv("FIRESTAFF_DATA");
    const char *home = getenv("HOME");

    if (!out || out_size == 0u || !suffix) return 0;
    if (data && data[0]) {
        snprintf(out, out_size, "%s/%s", data, suffix);
        return 1;
    }
    if (home && home[0]) {
        snprintf(out, out_size, "%s/.firestaff/data/%s", home, suffix);
        return 1;
    }
    return 0;
}

static int load_dungeon(uint8_t **out_data, size_t *out_size,
                        char *path, size_t path_size)
{
    static const char *suffixes[] = {
        "dm2/DUNGEON.DAT",
        "dm2/dungeon.dat",
        "dm2/data/DUNGEON.DAT",
        "dm2/data/dungeon.dat"
    };
    size_t i;

    for (i = 0u; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i) {
        if (candidate_path(path, path_size, suffixes[i]) &&
            read_file(path, out_data, out_size)) {
            return 1;
        }
    }
    return 0;
}

static void test_real_dungeon_c_light_receipts(
    const DM2_V1_GdatSceneLightM11Receipt *scene)
{
    uint8_t *data = NULL;
    size_t size = 0u;
    char path[1024];
    DM2_V1_DungeonData real_dungeon;
    int descriptors_seen = 0;
    int evaluated = 0;
    int dynamic_maps = 0;
    int fixed_maps = 0;
    int map_bound = 1;
    int dynamic_blocked = 1;

    if (!load_dungeon(&data, &size, path, sizeof(path))) {
        printf("SKIP: real DM2 DUNGEON.DAT not found for c_light scan\n");
        return;
    }
    CHECK("real DM2 DUNGEON.DAT loads for c_light descriptor scan",
          dm2_v1_dungeon_load(&real_dungeon, data, (int)size) == 0);
    if (real_dungeon.raw_data) {
        for (int level = 0; level < real_dungeon.level_count; ++level) {
            DM2_V1_CLightMapDescriptorReceipt map;
            DM2_V1_CLightSourceState real_source;
            DM2_V1_CLightM11Receipt real_receipt;
            uint32_t first_hash;

            memset(&map, 0, sizeof(map));
            if (!dm2_v1_dungeon_c_light_map_descriptor_receipt(
                    &real_dungeon, level, &map)) {
                continue;
            }
            ++descriptors_seen;
            memset(&real_source, 0, sizeof(real_source));
            real_source.dynamic_map = map.dynamic_light;
            if (map.dynamic_light) {
                /* A real DUNGEON.DAT descriptor proves only the dynamic
                 * branch selector. It does not contain v1e0974, party-hand
                 * charges, spell effects, rain globals, or v1e0978. Never
                 * manufacture those values in a real-data regression. */
                if (dm2_v1_c_light_m11_receipt_build_for_map(
                        scene, &map, &real_source, &real_receipt)) {
                    dynamic_blocked = 0;
                }
                ++dynamic_maps;
                continue;
            }
            real_source.valid = 1;
            real_source.source_state_hash = map.descriptor_hash;
            if (!dm2_v1_c_light_m11_receipt_build_for_map(
                    scene, &map, &real_source, &real_receipt)) {
                map_bound = 0;
                continue;
            }
            ++evaluated;
            ++fixed_maps;
            first_hash = real_receipt.receipt_hash;
            ++map.descriptor_hash;
            if (dm2_v1_c_light_m11_receipt_build_for_map(
                    scene, &map, &real_source, &real_receipt) &&
                real_receipt.receipt_hash == first_hash) {
                map_bound = 0;
            }
        }
    }
    CHECK("real DUNGEON.DAT exposes c_light map descriptors",
          descriptors_seen > 0);
    CHECK("real c_light receipts are bound to descriptor hashes", map_bound);
    CHECK("real DUNGEON.DAT exposes dynamic c_light branch", dynamic_maps > 0);
    CHECK("real dynamic c_light branch stays blocked without source state",
          dynamic_blocked);
    CHECK("real DUNGEON.DAT exposes fixed or bounded c_light branch",
          fixed_maps > 0 || (dynamic_maps > 0 && evaluated == 0));
    dm2_v1_dungeon_free(&real_dungeon);
    free(data);
}

int main(void)
{
    DM2_V1_GdatSceneLightM11Receipt scene;
    DM2_V1_CLightSourceState source;
    DM2_V1_CLightM11Receipt receipt;
    DM2_V1_ViewportState viewport;
    DM2_V1_DungeonData dungeon;
    DM2_V1_CLightMapDescriptorReceipt map_receipt;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t raw_dungeon[44u + 16u];

    memset(&scene, 0, sizeof(scene));
    memset(&source, 0, sizeof(source));
    scene.valid = 1;
    scene.graphicsset = 2u;
    scene.scene_control_hash = 0x53434e45u;
    source.valid = 1;
    source.source_state_hash = 0x434c4954u;

    source.dynamic_map = 0u;
    source.base_light = 5u; /* Must be ignored: non-dynamic base is one. */
    source.darkness_offset = 0u;
    CHECK("non-dynamic c_light starts at the source level one",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.valid && receipt.light_level == 1u &&
              receipt.graphicsset == scene.graphicsset &&
              receipt.receipt_hash != 0u);

    source.dynamic_map = 1u;
    source.base_light = 5u;
    source.darkness_offset = 2u;
    CHECK("dynamic c_light subtracts observed darkness from v1e0974",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.light_level == 3u && receipt.dynamic_map == 1u);

    source.darkness_offset = 12u;
    CHECK("c_light clamps a dark dynamic result to zero",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              receipt.light_level == 0u);

    source.darkness_offset = 2u;
    CHECK("authenticated c_light binds only its matching scene transaction",
          dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt));
    memset(&viewport, 0, sizeof(viewport));
    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_source_materials_required(&viewport, 1);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, scene.graphicsset, scene.scene_control_hash,
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_c_light_receipt(&viewport, &receipt);
    CHECK("c_light reaches source-required dungeon-square metadata",
          viewport.gdat_c_light_receipt_ready &&
              viewport.gdat_c_light_level == 3u &&
              viewport.squares[DM2_SQ_D0C].light_level == 3u);
    dm2_v1_viewport_render(&viewport);
    CHECK("M11 frame consumes authenticated c_light metadata without pixels",
          viewport.gdat_c_light_consumed_count == 1 &&
              viewport.gdat_c_light_receipt_hash == receipt.receipt_hash);
    {
        uint8_t palette_darkness = 0u;
        CHECK("DISPLAY_VIEWPORT palette parameter is c_light times ten",
              dm2_v1_c_light_m11_palette_darkness(
                  &scene, &receipt, &palette_darkness) &&
                  palette_darkness == 30u);
    }
    {
        uint8_t floor_darkness = 0xffu;
        uint8_t ceiling_darkness = 0xffu;
        CHECK("_32cb_0804 keeps the c_light parameter for stationary floor/ceiling",
              dm2_v1_gdat_scene_m11_plane_palette_darkness(
                  DM2_GDAT_GFXSET_FLOOR, 30u, &floor_darkness) &&
                  dm2_v1_gdat_scene_m11_plane_palette_darkness(
                      DM2_GDAT_GFXSET_CEIL, 30u, &ceiling_darkness) &&
                  floor_darkness == 30u && ceiling_darkness == 30u);
        CHECK("plane lighting rejects non-plane fields and out-of-range input",
              !dm2_v1_gdat_scene_m11_plane_palette_darkness(
                  2u, 30u, &floor_darkness) && floor_darkness == 0u &&
                  !dm2_v1_gdat_scene_m11_plane_palette_darkness(
                      DM2_GDAT_GFXSET_FLOOR, 65u, &floor_darkness));
    }
    {
        uint8_t field = 0xffu;
        CHECK("moving _32cb_0804 selects dt07/9 and dt07/10 for scene planes",
              dm2_v1_gdat_scene_m11_plane_translation_field(
                  DM2_GDAT_GFXSET_FLOOR, 1, &field) && field == 9u &&
                  dm2_v1_gdat_scene_m11_plane_translation_field(
                      DM2_GDAT_GFXSET_CEIL, 1, &field) && field == 10u);
        CHECK("stationary _32cb_0804 retains dt07/0 and dt07/1",
              dm2_v1_gdat_scene_m11_plane_translation_field(
                  DM2_GDAT_GFXSET_FLOOR, 0, &field) && field == 0u &&
                  dm2_v1_gdat_scene_m11_plane_translation_field(
                      DM2_GDAT_GFXSET_CEIL, 0, &field) && field == 1u);
    }
    {
        uint8_t palette[] = { 0u, 1u, 42u, 255u };
        uint8_t translation[256];
        uint32_t translation_hash = 0u;

        for (unsigned i = 0u; i < sizeof(translation); ++i) {
            translation[i] = (uint8_t)(255u - i);
        }
        CHECK("TRANSLATE_PALETTE consumes the exact dt07 lookup bytes",
              dm2_v1_gdat_scene_m11_translate_palette(
                  palette, sizeof(palette), translation, sizeof(translation),
                  &translation_hash) && translation_hash != 0u &&
                  palette[0] == 255u && palette[1] == 254u &&
                  palette[2] == 213u && palette[3] == 0u);
        CHECK("TRANSLATE_PALETTE rejects a partial dt07 lookup",
              !dm2_v1_gdat_scene_m11_translate_palette(
                  palette, sizeof(palette), translation, 255u,
                  &translation_hash) && translation_hash == 0u);
    }
    ++scene.scene_control_hash;
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport, 1, scene.graphicsset, scene.scene_control_hash,
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u);
    dm2_v1_viewport_set_c_light_receipt(&viewport, &receipt);
    CHECK("mismatched scene transaction clears c_light instead of reusing it",
          !viewport.gdat_c_light_receipt_ready &&
              viewport.squares[DM2_SQ_D0C].light_level == 0u);
    {
        uint8_t palette_darkness = 0xffu;
        CHECK("mismatched scene cannot publish a palette parameter",
              !dm2_v1_c_light_m11_palette_darkness(
                  &scene, &receipt, &palette_darkness) &&
                  palette_darkness == 0u);
    }

    source.source_state_hash = 0u;
    CHECK("missing raw c_light state cannot borrow scene controls",
          !dm2_v1_c_light_m11_receipt_build(&scene, &source, &receipt) &&
              !receipt.valid);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw_dungeon, 0, sizeof(raw_dungeon));
    dungeon.level_count = 1;
    dungeon.raw_data = raw_dungeon;
    dungeon.raw_size = (int)sizeof(raw_dungeon);
    raw_dungeon[44u + 12u] = 0x00u;
    raw_dungeon[44u + 13u] = 0x30u;
    CHECK("c_light reads dynamic branch from raw map Difficulty",
          dm2_v1_dungeon_c_light_map_descriptor_receipt(
              &dungeon, 0, &map_receipt) && map_receipt.valid &&
              map_receipt.difficulty == 3u && map_receipt.dynamic_light &&
              map_receipt.descriptor_hash != 0u);
    raw_dungeon[44u + 13u] = 0x00u;
    CHECK("difficulty-zero map selects c_light fixed-light branch",
          dm2_v1_dungeon_c_light_map_descriptor_receipt(
              &dungeon, 0, &map_receipt) && !map_receipt.dynamic_light &&
              map_receipt.descriptor_hash != 0u);
    source.valid = 1;
    source.source_state_hash = 0x434c4954u;
    source.dynamic_map = 0u;
    source.base_light = 5u;
    source.darkness_offset = 0u;
    scene.scene_control_hash = 0x53434e45u;
    scene.valid = 1;
    CHECK("c_light state must match source map fixed-light branch",
          dm2_v1_c_light_m11_receipt_build_for_map(
              &scene, &map_receipt, &source, &receipt) &&
              receipt.light_level == 1u &&
              receipt.map_descriptor_hash == map_receipt.descriptor_hash);
    {
        uint32_t fixed_receipt_hash = receipt.receipt_hash;

        ++map_receipt.descriptor_hash;
        CHECK("c_light map-bound receipt hash changes with descriptor identity",
              dm2_v1_c_light_m11_receipt_build_for_map(
                  &scene, &map_receipt, &source, &receipt) &&
                  receipt.light_level == 1u &&
                  receipt.receipt_hash != fixed_receipt_hash);
    }
    source.dynamic_map = 1u;
    CHECK("c_light rejects state from a different map light branch",
          !dm2_v1_c_light_m11_receipt_build_for_map(
              &scene, &map_receipt, &source, &receipt) && !receipt.valid);

    test_real_dungeon_c_light_receipts(&scene);

    printf("DM2 c_light receipt: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
