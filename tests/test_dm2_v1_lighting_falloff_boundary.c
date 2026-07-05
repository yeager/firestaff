/*
 * test_dm2_v1_lighting_falloff_boundary.c — DM2 V1 object lighting gate
 *
 * Covers exactly one deterministic runtime rule:
 * - source light falls off linearly by integer division inside the radius.
 * - distance_tiles == source->light_radius must extinguish to 0.
 *
 * Source: ReDMCSB DUNVIEW.C:4960-5039 — object depth scale and
 *         palette-change selection before object bitmap draw.
 */

#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>
#include <string.h>

static int s_tests_run = 0;
static int s_tests_passed = 0;
static int s_asset_fetch_calls = 0;

#define CHECK(name_, cond_) do { \
    printf("  %s...\n", name_); \
    s_tests_run++; \
    if (cond_) { s_tests_passed++; printf("    PASS\n"); } \
    else      { printf("    FAIL\n"); } \
} while (0)

static int test_dm2_asset_fetch(void *user,
                                int gdat_index,
                                const uint8_t **out_pixels,
                                int *out_w,
                                int *out_h,
                                int *out_stride)
{
    static const uint8_t ceiling[4] = { 2, 3, 4, 5 };
    static const uint8_t floor[4] = { 6, 7, 8, 9 };
    static const uint8_t wall[4] = { 11, 12, 13, 14 };
    static const uint8_t door_frame[4] = { 15, 1, 2, 3 };
    (void)user;
    ++s_asset_fetch_calls;
    if (gdat_index == -2) {
        if (out_pixels) *out_pixels = ceiling;
    } else if (gdat_index == -1) {
        if (out_pixels) *out_pixels = floor;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_WALL_FIELD_FIRST &&
               DM2_V1_VIEWPORT_GFX_WALL_FIELD_BASE - gdat_index < 0x40) {
        if (out_pixels) *out_pixels = wall;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FRONT &&
               DM2_V1_VIEWPORT_GFX_DOOR_FRAME_FIELD_BASE - gdat_index < 0x20) {
        if (out_pixels) *out_pixels = door_frame;
    } else {
        if (out_pixels) *out_pixels = NULL;
        if (out_w) *out_w = 0;
        if (out_h) *out_h = 0;
        if (out_stride) *out_stride = 0;
        return -1;
    }
    if (out_w) *out_w = 2;
    if (out_h) *out_h = 2;
    if (out_stride) *out_stride = 2;
    return 0;
}

static void test_floor_ceiling_asset_provider(void)
{
    uint8_t framebuffer[320 * 200];
    DM2_V1_ViewportState viewport;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_render_floor_ceiling(&viewport);
    CHECK("floor/ceiling fallback draws when no asset provider is installed",
          viewport.asset_floor_ceiling_drawn_count == 0 &&
              viewport.fallback_floor_ceiling_drawn_count == 2 &&
              framebuffer[0] == 1 &&
              framebuffer[(66 * 320)] == 5);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_floor_ceiling(&viewport);
    CHECK("floor/ceiling pass fetches DM2 ceiling and floor assets",
          s_asset_fetch_calls == 2 &&
              viewport.asset_floor_ceiling_drawn_count == 2 &&
              viewport.fallback_floor_ceiling_drawn_count == 0);
    CHECK("ceiling asset tiles across the top region",
          framebuffer[0] == 2 &&
              framebuffer[1] == 3 &&
              framebuffer[320] == 4 &&
              framebuffer[321] == 5 &&
              framebuffer[2] == 2);
    CHECK("floor asset tiles across the floor region",
          framebuffer[(66 * 320)] == 6 &&
              framebuffer[(66 * 320) + 1] == 7 &&
              framebuffer[(67 * 320)] == 8 &&
              framebuffer[(67 * 320) + 1] == 9 &&
              framebuffer[(66 * 320) + 2] == 6);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_render_walls(&viewport);
    CHECK("wall fallback counts when no asset provider is installed",
          viewport.asset_wall_drawn_count == 0 &&
              viewport.fallback_wall_drawn_count == 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_walls(&viewport);
    CHECK("wall pass fetches the DM2 viewport-cell wall assets",
          s_asset_fetch_calls == 10 &&
              viewport.asset_wall_drawn_count == 10 &&
              viewport.fallback_wall_drawn_count == 0);
    CHECK("wall assets are scaled into the left and right forward cells",
          framebuffer[0] == 11 &&
              framebuffer[31] == 12 &&
              framebuffer[(135 * 320)] == 13 &&
              framebuffer[192] == 11 &&
              framebuffer[223] == 12);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    dm2_v1_render_doors(&viewport);
    CHECK("door fallback counts when no asset provider is installed",
          viewport.asset_door_frame_drawn_count == 0 &&
              viewport.fallback_door_drawn_count == 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("door pass fetches the DM2 front door-frame asset",
          s_asset_fetch_calls == 1 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.fallback_door_drawn_count == 0);
    CHECK("front door-frame asset is scaled into the forward cell",
          framebuffer[0] == 15 &&
              framebuffer[223] == 1 &&
              framebuffer[(135 * 320)] == 2);
}

int main(void)
{
    printf("=== DM2 V1 Lighting/Palette Runtime Gate ===\n\n");

    DM2_CreatureSprite source = { 0 };
    source.light_radius = 4;
    CHECK("distance 0 keeps deterministic base brightness",
          dm2_v1_viewport_object_light_level(15, 0, &source) == 15);
    CHECK("distance 1 has deterministic integer falloff",
          dm2_v1_viewport_object_light_level(15, 1, &source) == 11);
    CHECK("distance 2 has deterministic integer falloff",
          dm2_v1_viewport_object_light_level(15, 2, &source) == 7);
    CHECK("distance 3 is the last lit tile before boundary",
          dm2_v1_viewport_object_light_level(15, 3, &source) == 3);
    CHECK("at boundary distance == radius is 0",
          dm2_v1_viewport_object_light_level(15, 4, &source) == 0);
    CHECK("beyond radius clamps to 0",
          dm2_v1_viewport_object_light_level(15, 8, &source) == 0);
    source.light_radius = 0;
    CHECK("zero-radius light source stays dark",
          dm2_v1_viewport_object_light_level(15, 0, &source) == 0);
    CHECK("null source keeps base tile light",
          dm2_v1_viewport_object_light_level(7, 4, NULL) == 7);

    {
        const char *e = dm2_v1_viewport_source_evidence();
        CHECK("source evidence cites DUNVIEW object draw path",
              e != NULL && strstr(e, "DUNVIEW.C:4960-5039") != NULL);
        CHECK("source evidence cites DM2 palette documentation",
              e != NULL && strstr(e, "docs/dm2_palette.md") != NULL);
    }
    test_floor_ceiling_asset_provider();

    printf("\nDM2 V1 Lighting/Palette Runtime Gate: %d/%d passed\n",
           s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
