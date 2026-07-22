/* Source: skproject SKWIN/SkWinCore.cpp QUERY_CREATURE_PICST, DRAW_ITEM,
 * DRAW_TEMP_PICST, and INTERFACE_GENERAL dt07/0x0A Rect14 placement. */
#include "dm2_v1_viewport_renderer.h"

#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) { ++passed; } \
    else { printf("FAIL: %s\n", label); } \
} while (0)

static int fetch_asset(void *user,
                       int gdat_index,
                       const uint8_t **out_pixels,
                       int *out_w,
                       int *out_h,
                       int *out_stride)
{
    static const uint8_t pixels[16] = {
        1, 0, 2, 3, 1, 0, 2, 3,
        1, 0, 2, 3, 1, 0, 2, 3
    };
    (void)user;
    (void)gdat_index;
    *out_pixels = pixels;
    *out_w = 4;
    *out_h = 4;
    *out_stride = 4;
    return 0;
}

static int fetch_palette(void *user,
                         int gdat_index,
                         uint8_t out_palette16[16],
                         uint32_t *out_hash)
{
    (void)user;
    (void)gdat_index;
    for (int i = 0; i < 16; ++i) out_palette16[i] = (uint8_t)(0x60 + i);
    if (out_hash) *out_hash = 0x50414c45u;
    return 0;
}

static void setup_viewport(DM2_V1_ViewportState *viewport,
                           uint8_t *framebuffer)
{
    dm2_v1_viewport_init(viewport, framebuffer, DM2_VP_WIDTH);
    dm2_v1_viewport_set_asset_provider(viewport, fetch_asset, NULL);
    dm2_v1_viewport_set_asset_palette_provider(viewport, fetch_palette, NULL);
    dm2_v1_viewport_set_source_materials_required(viewport, 1);
}

int main(void)
{
    DM2_V1_ViewportState viewport;
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_ItemRenderPlan item_plan;
    DM2_V1_ProjectileRenderPlan projectile_plan;
    DM2_V1_ItemAssetBlit item_blit;
    DM2_V1_ProjectileAssetBlit projectile_blit;
    uint8_t rect14_rows[2][14] = {
        { 0, 8, 1, 2, 3, 4, 64, 64, 64, 64, 0, 0, 0, 0 },
        { 1, -8, 5, 6, 7, 8, 32, 32, 32, 32, 1, 1, 1, 1 }
    };
    int base_item_index;
    int rect14_item_index;
    int base_projectile_index;
    int rect14_projectile_index;

    memset(framebuffer, 0, sizeof(framebuffer));
    setup_viewport(&viewport, framebuffer);

    /* Item with frame_index 0 selects Rect14 row 0. */
    viewport.item_count = 2;
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].frame_index = 0;
    viewport.items[0].direction = 0;
    viewport.items[0].depth = 2;
    viewport.items[0].screen_x = 100;
    viewport.items[0].screen_y = 80;
    viewport.items[0].source_static_object_admitted = 0;

    /* Static-object-admitted item must not receive Rect14 through this path. */
    viewport.items[1].item_category = 0x10;
    viewport.items[1].item_type = 0x22;
    viewport.items[1].frame_index = 0;
    viewport.items[1].direction = 0;
    viewport.items[1].depth = 2;
    viewport.items[1].screen_x = 120;
    viewport.items[1].screen_y = 80;
    viewport.items[1].source_static_object_admitted = 1;

    base_item_index = dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0);
    rect14_item_index = dm2_v1_viewport_item_graphic_index(0x10, 0x22, 1);

    CHECK("item plan builds without Rect14",
          dm2_v1_viewport_build_item_render_plan(&viewport, &item_plan) &&
          item_plan.item_count == 2);
    CHECK("item without Rect14 keeps base gdat index",
          item_plan.items[0].gdat_index == base_item_index &&
          item_plan.items[0].rect14_applied == 0);
    CHECK("static-object item skipped for Rect14 enrichment",
          item_plan.items[1].rect14_applied == 0);

    /* Bind the Rect14 table and rebuild. */
    dm2_v1_viewport_set_gdat_interface_rect14(
        &viewport, rect14_rows[0], 2, 0xdeadbeefu);
    CHECK("item plan enriches with Rect14",
          dm2_v1_viewport_build_item_render_plan(&viewport, &item_plan) &&
          item_plan.items[0].rect14_applied == 1 &&
          item_plan.items[0].gdat_index == rect14_item_index &&
          item_plan.items[0].rect14_scale64 == 64 &&
          item_plan.items[0].rect14_lateral_offset == 8 &&
          item_plan.items[0].rect14_flip_mirror == 0 &&
          item_plan.items[0].rect14_row_hash != 0u &&
          item_plan.items[0].rect14_placement_hash != 0u);
    CHECK("static-object item still skipped after Rect14 bind",
          item_plan.items[1].rect14_applied == 0);

    CHECK("Rect14 item asset blit uses source-stretched destination",
          dm2_v1_viewport_item_asset_blit(
              &item_plan.items[0], 4, 4, 4, viewport.party_dir,
              4, 32, &item_blit) == 1 &&
          item_blit.dst_rect.w == 4 && item_blit.dst_rect.h == 4 &&
          item_blit.flip_mirror == 0);

    /* Projectile with frame_index 1 selects Rect14 row 1. */
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 1;
    viewport.projectiles[0].direction = 0;
    viewport.projectiles[0].depth = 2;
    viewport.projectiles[0].screen_x = 100;
    viewport.projectiles[0].screen_y = 80;
    viewport.projectiles[0].render_kind = DM2_V1_PROJECTILE_RENDER_MISSILE;

    base_projectile_index = dm2_v1_viewport_projectile_graphic_index(
        0x0d, 0x02, 1);
    rect14_projectile_index = dm2_v1_viewport_projectile_graphic_index(
        0x0d, 0x02, 5);

    CHECK("projectile plan enriches with Rect14",
          dm2_v1_viewport_build_projectile_render_plan(
              &viewport, &projectile_plan) &&
          projectile_plan.projectile_count == 1 &&
          projectile_plan.projectiles[0].rect14_applied == 1 &&
          projectile_plan.projectiles[0].gdat_index == rect14_projectile_index &&
          projectile_plan.projectiles[0].rect14_scale64 == 32 &&
          projectile_plan.projectiles[0].rect14_lateral_offset == -8 &&
          projectile_plan.projectiles[0].rect14_flip_mirror == 1 &&
          projectile_plan.projectiles[0].rect14_row_hash != 0u &&
          projectile_plan.projectiles[0].rect14_placement_hash != 0u);

    CHECK("Rect14 projectile asset blit uses source-stretched destination",
          dm2_v1_viewport_projectile_asset_blit(
              &projectile_plan.projectiles[0], 4, 4, 4,
              viewport.party_dir, 0, NULL, &projectile_blit) == 1 &&
          projectile_blit.dst_rect.w == 2 &&
          projectile_blit.dst_rect.h == 2 &&
          projectile_blit.flip_mirror == 1);

    /* Out-of-range frame_index must not synthesize Rect14 data. */
    viewport.items[0].frame_index = 99;
    CHECK("item with out-of-range frame index keeps base plan",
          dm2_v1_viewport_build_item_render_plan(&viewport, &item_plan) &&
          item_plan.items[0].rect14_applied == 0 &&
          item_plan.items[0].gdat_index ==
              dm2_v1_viewport_item_graphic_index(0x10, 0x22, 99));

    printf("DM2 item/projectile Rect14 render plan: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
