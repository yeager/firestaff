#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else printf("FAIL: %s\n", label); \
} while (0)

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

    printf("DM2 c_light receipt: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
