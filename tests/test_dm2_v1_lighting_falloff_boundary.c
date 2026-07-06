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
static int s_last_asset_index = 0;
static int s_projectile_seven_frame_fixture = 0;
static int s_projectile_flip_fixture = 0;
static int s_creature_directional_frame_fixture = 0;

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
    static const uint8_t door_panel[4] = { 8, 9, 10, 11 };
    static const uint8_t door_frame[4] = { 15, 1, 2, 3 };
    static const uint8_t door_button[4] = { 4, 5, 6, 7 };
    static const uint8_t wall_button[4] = { 12, 13, 14, 15 };
    static const uint8_t creature_atlas[21 * 7] = {
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14
    };
    static const uint8_t creature_directional_atlas[28 * 7] = {
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9,
        4,4,4,4,4,4,4, 12,12,12,12,12,12,12, 14,14,14,14,14,14,14, 9,9,9,9,9,9,9
    };
    static const uint8_t item_atlas[21 * 7] = {
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8,
        2,2,2,2,2,2,2, 6,6,6,6,6,6,6, 8,8,8,8,8,8,8
    };
    static const uint8_t projectile_atlas[21 * 7] = {
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15,
        5,5,5,5,5,5,5, 13,13,13,13,13,13,13, 15,15,15,15,15,15,15
    };
    static const uint8_t projectile_directional_atlas[49 * 7] = {
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13,
        1,1,1,1,1,1,1, 2,2,2,2,2,2,2, 3,3,3,3,3,3,3, 10,10,10,10,10,10,10, 11,11,11,11,11,11,11, 12,12,12,12,12,12,12, 13,13,13,13,13,13,13
    };
    static uint8_t projectile_flip_atlas[49 * 7];
    static int projectile_flip_atlas_init = 0;
    (void)user;
    if (!projectile_flip_atlas_init) {
        for (int y = 0; y < 7; ++y) {
            for (int frame = 0; frame < 7; ++frame) {
                for (int x = 0; x < 7; ++x) {
                    projectile_flip_atlas[y * 49 + frame * 7 + x] =
                        (uint8_t)(frame == 4 ? 21 + x : 1 + frame);
                }
            }
        }
        projectile_flip_atlas_init = 1;
    }
    ++s_asset_fetch_calls;
    s_last_asset_index = gdat_index;
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
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_RELEASED &&
               DM2_V1_VIEWPORT_GFX_DOOR_BUTTON_FIELD_BASE - gdat_index < 0x08) {
        if (out_pixels) *out_pixels = door_button;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT &&
               DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index < 0x04) {
        if (out_pixels) *out_pixels = door_panel;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) >= 0x10 &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) <= 0x15) {
        if (out_pixels) *out_pixels = item_atlas;
        if (out_w) *out_w = 21;
        if (out_h) *out_h = 7;
        if (out_stride) *out_stride = 21;
        return 0;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) >= 0x0d &&
               (((DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) & 0xff) <= 0x15) {
        if (out_pixels) *out_pixels = s_projectile_flip_fixture
            ? projectile_flip_atlas
            : (s_projectile_seven_frame_fixture
                ? projectile_directional_atlas
                : projectile_atlas);
        if (out_w) *out_w = (s_projectile_seven_frame_fixture ||
                             s_projectile_flip_fixture) ? 49 : 21;
        if (out_h) *out_h = 7;
        if (out_stride) *out_stride =
            (s_projectile_seven_frame_fixture ||
             s_projectile_flip_fixture) ? 49 : 21;
        return 0;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = s_creature_directional_frame_fixture
            ? creature_directional_atlas : creature_atlas;
        if (out_w) *out_w = s_creature_directional_frame_fixture ? 28 : 21;
        if (out_h) *out_h = 7;
        if (out_stride) *out_stride =
            s_creature_directional_frame_fixture ? 28 : 21;
        return 0;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = wall_button;
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

static int rect_equals(const DM2_V1_ViewportRect *rect,
                       int x,
                       int y,
                       int w,
                       int h)
{
    return rect &&
        rect->x == x &&
        rect->y == y &&
        rect->w == w &&
        rect->h == h;
}

static void test_door_rect_contracts(void)
{
    DM2_V1_ViewportRect rect;

    CHECK("DM2 D0C maps to skproject viewport cell 0",
          dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D0C) == 0);
    CHECK("DM2 D1C maps to skproject viewport cell 3",
          dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D1C) == 3);
    CHECK("DM2 D2C maps to skproject viewport cell 6",
          dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D2C) == 6);
    CHECK("DM2 side squares are not default-door-button cells",
          dm2_v1_viewport_skproject_cell_for_square(DM2_SQ_D1L) < 0 &&
              dm2_v1_viewport_door_button_rectno_for_square(DM2_SQ_D1L) < 0);
    CHECK("DM2 D0C/D1C/D2C default buttons use skproject rectnos",
          dm2_v1_viewport_door_button_rectno_for_square(DM2_SQ_D0C) == 4 &&
              dm2_v1_viewport_door_button_rectno_for_square(DM2_SQ_D1C) == 3 &&
              dm2_v1_viewport_door_button_rectno_for_square(DM2_SQ_D2C) == 2);
    CHECK("DM2 default door clickability follows skproject rectno gate",
          dm2_v1_viewport_door_button_clickable_for_square(DM2_SQ_D0C) &&
              dm2_v1_viewport_door_button_clickable_for_square(DM2_SQ_D1C) &&
              !dm2_v1_viewport_door_button_clickable_for_square(DM2_SQ_D2C));
    CHECK("DM2 custom wall button asset index packs WALL_GFX index and field",
          dm2_v1_viewport_wall_button_graphic_index(0x2a, 0x07) ==
              DM2_V1_VIEWPORT_GFX_WALL_BUTTON_FIELD_BASE -
                  ((0x2a << DM2_V1_VIEWPORT_GFX_WALL_BUTTON_INDEX_SHIFT) | 0x07));
    CHECK("DM2 custom wall button asset index rejects invalid arguments",
          dm2_v1_viewport_wall_button_graphic_index(-1, 0) == 0 &&
              dm2_v1_viewport_wall_button_graphic_index(0, 0x100) == 0);

    CHECK("DM2 D0C door panel rect is the startup front-door bound",
          dm2_v1_viewport_door_panel_rect_for_square(DM2_SQ_D0C, &rect) &&
              rect_equals(&rect, 80, 0, 160, 135));
    CHECK("DM2 D1C door panel rect is the startup near-door bound",
          dm2_v1_viewport_door_panel_rect_for_square(DM2_SQ_D1C, &rect) &&
              rect_equals(&rect, 60, 9, 104, 110));
    CHECK("DM2 D2C door panel rect is the startup mid-door bound",
          dm2_v1_viewport_door_panel_rect_for_square(DM2_SQ_D2C, &rect) &&
              rect_equals(&rect, 60, 20, 103, 71));
    CHECK("DM2 non-center squares do not expose a door panel rect",
          !dm2_v1_viewport_door_panel_rect_for_square(DM2_SQ_D0L, &rect) &&
              rect_equals(&rect, 0, 0, 0, 0));

    CHECK("DM2 D0C default door button rect follows skproject rectno 4",
          dm2_v1_viewport_door_button_rect_for_square(DM2_SQ_D0C, &rect) &&
              rect_equals(&rect, 212, 58, 16, 18));
    CHECK("DM2 D1C default door button rect follows skproject rectno 3",
          dm2_v1_viewport_door_button_rect_for_square(DM2_SQ_D1C, &rect) &&
              rect_equals(&rect, 142, 57, 12, 14));
    CHECK("DM2 D2C default door button rect follows skproject rectno 2",
          dm2_v1_viewport_door_button_rect_for_square(DM2_SQ_D2C, &rect) &&
              rect_equals(&rect, 147, 51, 8, 9));
    CHECK("DM2 non-center squares do not expose a door button rect",
          !dm2_v1_viewport_door_button_rect_for_square(DM2_SQ_D0R, &rect) &&
              rect_equals(&rect, 0, 0, 0, 0));
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
    CHECK("door pass fetches the DM2 front panel and door-frame assets",
          s_asset_fetch_calls == 2 &&
              viewport.asset_door_panel_drawn_count == 1 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.fallback_door_drawn_count == 0);
    CHECK("front door-frame asset is scaled into the forward cell",
          framebuffer[0] == 15 &&
              framebuffer[223] == 1 &&
              framebuffer[(135 * 320)] == 2);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    viewport.squares[DM2_SQ_D0C].door_button_state = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("door pass fetches a stateful default button asset",
          s_asset_fetch_calls == 3 &&
              viewport.asset_door_panel_drawn_count == 1 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.asset_door_button_drawn_count == 1 &&
              viewport.fallback_door_drawn_count == 0);
    CHECK("default door button is scaled onto the front door",
          framebuffer[(58 * 320) + 212] == 4 &&
              framebuffer[(58 * 320) + 213] == 4);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_button = 0;
    viewport.squares[DM2_SQ_D0C].door_wall_button = 1;
    viewport.squares[DM2_SQ_D0C].door_wall_button_index = 0x2a;
    viewport.squares[DM2_SQ_D0C].door_wall_button_field = 0x07;
    s_asset_fetch_calls = 0;
    s_last_asset_index = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("door pass fetches a custom WALL_GFX button asset when no default button exists",
          s_asset_fetch_calls == 3 &&
              s_last_asset_index ==
                  dm2_v1_viewport_wall_button_graphic_index(0x2a, 0x07) &&
              viewport.asset_door_panel_drawn_count == 1 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.asset_door_button_drawn_count == 1);
    CHECK("custom wall-gfx door button is scaled through the same rectno path",
          framebuffer[(58 * 320) + 212] == 12 &&
              framebuffer[(58 * 320) + 213] == 12);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D1C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D2C].flags |= DM2_SQF_HAS_DOOR;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("door pass fetches D0C/D1C/D2C panel and frame assets",
          s_asset_fetch_calls == 6 &&
              viewport.asset_door_panel_drawn_count == 3 &&
              viewport.asset_door_frame_drawn_count == 3 &&
              viewport.fallback_door_drawn_count == 0);
}

static void test_sprite_asset_provider(void)
{
    uint8_t framebuffer[320 * 200];
    DM2_V1_ViewportState viewport;

    CHECK("DM2 creature asset index packs type and frame",
          dm2_v1_viewport_creature_graphic_index(0x12, 0x03) ==
              DM2_V1_VIEWPORT_GFX_CREATURE_FIELD_BASE -
                  ((0x12 << DM2_V1_VIEWPORT_GFX_CREATURE_INDEX_SHIFT) | 0x03));
    CHECK("DM2 item DB pool maps to viewport GDAT category",
          dm2_v1_viewport_item_category_for_db_pool(5) == 0x10 &&
              dm2_v1_viewport_item_category_for_db_pool(6) == 0x11 &&
              dm2_v1_viewport_item_category_for_db_pool(7) == 0x12 &&
              dm2_v1_viewport_item_category_for_db_pool(10) == 0x15);
    CHECK("DM2 item asset index packs category, type and frame",
          dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0x04) ==
              DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE -
                  ((0x10 << DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) |
                   (0x22 << DM2_V1_VIEWPORT_GFX_ITEM_INDEX_SHIFT) | 0x04));
    CHECK("DM2 projectile asset index packs category, type and frame",
          dm2_v1_viewport_projectile_graphic_index(0x0d, 0x02, 0x01) ==
              DM2_V1_VIEWPORT_GFX_PROJECTILE_FIELD_BASE -
                  ((0x0d << DM2_V1_VIEWPORT_GFX_PROJECTILE_CATEGORY_SHIFT) |
                   (0x02 << DM2_V1_VIEWPORT_GFX_PROJECTILE_INDEX_SHIFT) | 0x01));
    CHECK("DM2 map-chip atlas width resolves to square frame width",
          dm2_v1_viewport_map_chip_frame_width(21, 7) == 7 &&
              dm2_v1_viewport_map_chip_frame_count(21, 7) == 3);
    CHECK("DM2 single map-chip bitmap remains one frame",
          dm2_v1_viewport_map_chip_frame_width(8, 8) == 8 &&
              dm2_v1_viewport_map_chip_frame_count(8, 8) == 1);
    CHECK("DM2 map-chip frame index wraps inside source atlas count",
          dm2_v1_viewport_map_chip_frame_index(4, 3) == 1 &&
              dm2_v1_viewport_map_chip_frame_index(-1, 3) == 0);
    CHECK("DM2 projectile directional frame keeps short atlases animated",
          dm2_v1_viewport_projectile_frame_for_direction(5, 1, 0, 3) == 2);
    CHECK("DM2 projectile directional frame follows view-relative missile frames",
          dm2_v1_viewport_projectile_frame_for_direction(0, 1, 0, 7) == 4 &&
              dm2_v1_viewport_projectile_frame_for_direction(0, 3, 0, 7) == 6);
    CHECK("DM2 projectile map-chip frame applies skproject adjustment table",
          dm2_v1_viewport_projectile_frame_for_map_chip(
              0, 1, 1, 0, 7, DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL) == 6 &&
              dm2_v1_viewport_projectile_frame_for_map_chip(
                  0, 1, 0, 0, 7,
                  DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL) == 4 &&
              dm2_v1_viewport_projectile_frame_for_map_chip(
                  0, 2, 2, 0, 7,
                  DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL) == 3);
    CHECK("DM2 projectile map-chip frame handles non-directional classes",
          dm2_v1_viewport_projectile_frame_for_map_chip(
              0, 1, 1, 0, 7, DM2_V1_PROJECTILE_FRAME_CLASS_BASE_FRONT) == 3 &&
              dm2_v1_viewport_projectile_frame_for_map_chip(
                  0, 1, 1, 0, 7,
                  DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY) == 0 &&
              dm2_v1_viewport_projectile_frame_for_map_chip(
                  0, 1, 1, 0, 7,
                  DM2_V1_PROJECTILE_FRAME_CLASS_FLAT) == 0);
    CHECK("DM2 projectile flip follows skproject missile flip table",
          dm2_v1_viewport_projectile_flip_for_direction(0, 0) == 0 &&
              dm2_v1_viewport_projectile_flip_for_direction(1, 0) == 1 &&
              dm2_v1_viewport_projectile_flip_for_direction(2, 0) == 3 &&
              dm2_v1_viewport_projectile_flip_for_direction(3, 0) == 2);
    CHECK("DM2 cloud frame follows skproject tick alternation",
          dm2_v1_viewport_cloud_frame_for_tick(0, 7) == 1 &&
              dm2_v1_viewport_cloud_frame_for_tick(1, 7) == 2);
    CHECK("DM2 creature directional frame keeps short atlases animated",
          dm2_v1_viewport_creature_frame_for_direction(3, 1, 0, 2) == 1);
    CHECK("DM2 creature directional frame follows view-relative parity frames",
          dm2_v1_viewport_creature_frame_for_direction(2, 2, 0, 4) == 2 &&
              dm2_v1_viewport_creature_frame_for_direction(2, 1, 0, 4) == 3);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 0x12;
    viewport.creatures[0].frame_index = 0x01;
    viewport.creatures[0].screen_x = 40;
    viewport.creatures[0].screen_y = 50;
    viewport.creatures[0].health_pct = 100;
    dm2_v1_render_creatures(&viewport);
    CHECK("creature fallback draws when no sprite asset provider is installed",
          viewport.asset_creature_drawn_count == 0 &&
              viewport.fallback_creature_drawn_count == 1 &&
              framebuffer[(50 * 320) + 40] == (uint8_t)(11 + (0x12 & 7)));

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 0x12;
    viewport.creatures[0].frame_index = 0x01;
    viewport.creatures[0].screen_x = 40;
    viewport.creatures[0].screen_y = 50;
    viewport.creatures[0].health_pct = 100;
    s_asset_fetch_calls = 0;
    s_last_asset_index = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_creatures(&viewport);
    CHECK("creature pass fetches DM2 map-chip sprite assets",
          s_asset_fetch_calls == 1 &&
              s_last_asset_index ==
                  dm2_v1_viewport_creature_graphic_index(0x12, 0x01) &&
              viewport.asset_creature_drawn_count == 1 &&
              viewport.fallback_creature_drawn_count == 0);
    CHECK("creature map-chip atlas draws only the selected frame",
          framebuffer[((50 - 4) * 320) + (40 - 4)] == 12 &&
              framebuffer[(50 * 320) + 40] == 12);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_party(&viewport, 0, 0, 0);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 0x12;
    viewport.creatures[0].frame_index = 0x02;
    viewport.creatures[0].direction = 1;
    viewport.creatures[0].screen_x = 40;
    viewport.creatures[0].screen_y = 50;
    viewport.creatures[0].health_pct = 100;
    s_creature_directional_frame_fixture = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_creatures(&viewport);
    s_creature_directional_frame_fixture = 0;
    CHECK("creature render uses view-relative directional atlas frame",
          viewport.asset_creature_drawn_count == 1 &&
              framebuffer[((50 - 4) * 320) + (40 - 4)] == 9 &&
              framebuffer[(50 * 320) + 40] == 9);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.item_count = 1;
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].frame_index = 0x04;
    viewport.items[0].screen_x = 80;
    viewport.items[0].screen_y = 90;
    dm2_v1_render_items(&viewport);
    CHECK("item fallback draws when no sprite asset provider is installed",
          viewport.asset_item_drawn_count == 0 &&
              viewport.fallback_item_drawn_count == 1 &&
              framebuffer[(90 * 320) + 80] == 3);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.item_count = 1;
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].frame_index = 0x04;
    viewport.items[0].screen_x = 80;
    viewport.items[0].screen_y = 90;
    s_asset_fetch_calls = 0;
    s_last_asset_index = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_items(&viewport);
    CHECK("item pass fetches DM2 map-chip sprite assets",
          s_asset_fetch_calls == 1 &&
              s_last_asset_index ==
                  dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0x04) &&
              viewport.asset_item_drawn_count == 1 &&
              viewport.fallback_item_drawn_count == 0);
    CHECK("item map-chip atlas draws only the selected frame",
          framebuffer[((90 - 3) * 320) + (80 - 3)] == 6 &&
              framebuffer[(90 * 320) + 80] == 6);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.carried_item_present = 1;
    viewport.carried_item.item_category = 0x15;
    viewport.carried_item.item_type = 0x22;
    viewport.carried_item.frame_index = 0x04;
    viewport.carried_item.screen_x = 120;
    viewport.carried_item.screen_y = 70;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_carried_item(&viewport);
    CHECK("carried leader-hand item uses the item map-chip asset path",
          s_asset_fetch_calls == 1 &&
              viewport.asset_carried_item_drawn_count == 1 &&
              viewport.fallback_carried_item_drawn_count == 0 &&
              framebuffer[(70 * 320) + 120] == 6);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 0x01;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    viewport.projectiles[0].velocity_x = 3;
    dm2_v1_render_projectiles(&viewport);
    CHECK("projectile fallback draws when no sprite asset provider is installed",
          viewport.asset_projectile_drawn_count == 0 &&
              viewport.fallback_projectile_drawn_count == 1 &&
              framebuffer[(70 * 320) + 120] == 15);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 0x01;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    viewport.projectiles[0].velocity_x = 3;
    s_asset_fetch_calls = 0;
    s_last_asset_index = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    CHECK("projectile pass fetches DM2 map-chip sprite assets",
          s_asset_fetch_calls == 1 &&
              s_last_asset_index ==
                  dm2_v1_viewport_projectile_graphic_index(0x0d, 0x02, 0x01) &&
              viewport.asset_projectile_drawn_count == 1 &&
              viewport.fallback_projectile_drawn_count == 0);
    CHECK("projectile map-chip atlas draws only the selected frame",
          framebuffer[((70 - 3) * 320) + (120 - 3)] == 13 &&
              framebuffer[(70 * 320) + 120] == 13);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_party(&viewport, 0, 0, 0);
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 0x01;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].object_direction = 1;
    viewport.projectiles[0].frame_class = DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    s_projectile_seven_frame_fixture = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    s_projectile_seven_frame_fixture = 0;
    CHECK("projectile render applies skproject adjusted missile atlas frame",
          viewport.asset_projectile_drawn_count == 1 &&
              framebuffer[((70 - 3) * 320) + (120 - 3)] == 13 &&
              framebuffer[(70 * 320) + 120] == 13);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_party(&viewport, 0, 0, 0);
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 0x00;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].object_direction = 0;
    viewport.projectiles[0].frame_class = DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    s_projectile_flip_fixture = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    s_projectile_flip_fixture = 0;
    CHECK("projectile render applies skproject horizontal flip mirror",
          viewport.asset_projectile_drawn_count == 1 &&
              framebuffer[((70 - 3) * 320) + (120 - 3)] == 27 &&
              framebuffer[((70 - 3) * 320) + (120 + 3)] == 21);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_party(&viewport, 0, 0, 0);
    viewport.tick_count = 0;
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x87;
    viewport.projectiles[0].frame_index = 0x00;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].render_kind = DM2_V1_PROJECTILE_RENDER_CLOUD;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    s_projectile_seven_frame_fixture = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    CHECK("cloud render uses tick frame instead of directional missile frame",
          viewport.asset_projectile_drawn_count == 1 &&
              framebuffer[((70 - 3) * 320) + (120 - 3)] == 2 &&
              framebuffer[(70 * 320) + 120] == 2);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_party(&viewport, 0, 0, 0);
    viewport.tick_count = 1;
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x87;
    viewport.projectiles[0].frame_index = 0x00;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].render_kind = DM2_V1_PROJECTILE_RENDER_CLOUD;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    s_projectile_seven_frame_fixture = 0;
    CHECK("cloud render alternates to the next tick frame",
          viewport.asset_projectile_drawn_count == 1 &&
              framebuffer[((70 - 3) * 320) + (120 - 3)] == 3 &&
              framebuffer[(70 * 320) + 120] == 3);
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
    test_door_rect_contracts();
    test_floor_ceiling_asset_provider();
    test_sprite_asset_provider();

    printf("\nDM2 V1 Lighting/Palette Runtime Gate: %d/%d passed\n",
           s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
