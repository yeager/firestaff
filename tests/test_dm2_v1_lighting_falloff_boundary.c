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
static int s_item_flip_fixture = 0;
static int s_fail_asset_index = 0;

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
    static const uint8_t door_overlay[4] = { 11, 12, 13, 14 };
    static const uint8_t door_frame[4] = { 15, 1, 2, 3 };
    static const uint8_t door_button[4] = { 4, 5, 6, 7 };
    static const uint8_t wall_button[4] = { 12, 13, 14, 15 };
    static const uint8_t hud_core[4] = { 12, 14, 15, 1 };
    static const uint8_t hud_portrait[4] = { 3, 4, 5, 6 };
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
    static uint8_t item_flip_atlas[21 * 7];
    static int item_flip_atlas_init = 0;
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
                        (uint8_t)((frame == 1 || frame == 4) ? 21 + x : 1 + frame);
                }
            }
        }
        projectile_flip_atlas_init = 1;
    }
    if (!item_flip_atlas_init) {
        for (int y = 0; y < 7; ++y) {
            for (int frame = 0; frame < 3; ++frame) {
                for (int x = 0; x < 7; ++x) {
                    item_flip_atlas[y * 21 + frame * 7 + x] =
                        (uint8_t)((frame == 0) ? 30 + x : 2 + frame);
                }
            }
        }
        item_flip_atlas_init = 1;
    }
    ++s_asset_fetch_calls;
    s_last_asset_index = gdat_index;
    if (s_fail_asset_index != 0 && gdat_index == s_fail_asset_index) {
        return -1;
    }
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
               DM2_V1_VIEWPORT_GFX_DOOR_ORNATE_FIELD_BASE &&
               gdat_index > DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE) {
        if (out_pixels) *out_pixels = door_overlay;
    } else if (gdat_index <=
                   DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_DOOR_DESTROYED_MASK_FIELD_BASE -
                       gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_OVERLAY_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = door_overlay;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_DOOR_RECORD_PANEL_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_DOOR_PANEL_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = door_panel;
    } else if (gdat_index <=
               DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE -
                   DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FRONT &&
               DM2_V1_VIEWPORT_GFX_DOOR_PANEL_FIELD_BASE - gdat_index < 0x04) {
        if (out_pixels) *out_pixels = door_panel;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE - gdat_index <
                   (0x100 << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT)) {
        if (out_pixels) *out_pixels = hud_portrait;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE &&
               DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE - gdat_index <
                   0x100) {
        if (out_pixels) *out_pixels = hud_core;
    } else if (gdat_index <= DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) >= 0x10 &&
               (((DM2_V1_VIEWPORT_GFX_ITEM_FIELD_BASE - gdat_index) >>
                 DM2_V1_VIEWPORT_GFX_ITEM_CATEGORY_SHIFT) & 0xff) <= 0x15) {
        if (out_pixels) *out_pixels = s_item_flip_fixture
            ? item_flip_atlas : item_atlas;
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
    CHECK("DM2 HUD portrait asset index packs portrait ordinal and field",
          dm2_v1_viewport_hud_portrait_graphic_index(3) ==
              DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD_BASE -
                  ((3 << DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_INDEX_SHIFT) |
                   DM2_V1_VIEWPORT_GFX_HUD_PORTRAIT_FIELD) &&
              dm2_v1_viewport_hud_portrait_graphic_index(-1) == 0 &&
              dm2_v1_viewport_hud_portrait_graphic_index(
                  DM2_V1_HUD_PORTRAIT_COUNT) == 0);
    CHECK("DM2 HUD core asset index packs skproject interface fields",
          dm2_v1_viewport_hud_core_graphic_index(
              DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) ==
              DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE -
                  DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR &&
              dm2_v1_viewport_hud_action_icon_graphic_index(4) ==
                  DM2_V1_VIEWPORT_GFX_HUD_CORE_FIELD_BASE -
                      (DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_ICON_BASE + 4) &&
              dm2_v1_viewport_hud_core_graphic_index(-1) == 0 &&
              dm2_v1_viewport_hud_action_icon_graphic_index(
                  DM2_V1_HUD_ACTION_ICON_COUNT) == 0);

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

static void test_hud_chrome_render_plan(void)
{
    DM2_V1_HudChromeRenderPlan indoor;
    DM2_V1_HudChromeRenderPlan outdoor;
    DM2_V1_HudPartyState party;

    memset(&indoor, 0x55, sizeof(indoor));
    CHECK("DM2 HUD chrome plan rejects null output",
          dm2_v1_viewport_build_hud_chrome_plan(0, NULL) == 0);
    CHECK("DM2 indoor HUD chrome plan builds",
          dm2_v1_viewport_build_hud_chrome_plan(0, &indoor) == 1);
    CHECK("DM2 indoor HUD top bar uses source viewport width",
          rect_equals(&indoor.top_bar_rect, 0, 0,
                      DM2_VP_WIDTH, DM2_VP_CHROME_TOP));
    CHECK("DM2 indoor HUD action strip is anchored at bottom",
          rect_equals(&indoor.action_strip_rect, 0,
                      DM2_VP_HEIGHT - DM2_VP_CHROME_BOT,
                      DM2_VP_WIDTH, DM2_VP_CHROME_BOT));
    CHECK("DM2 HUD exposes five action-icon placements",
          indoor.action_icon_count == DM2_V1_HUD_ACTION_ICON_COUNT &&
              rect_equals(&indoor.action_icons[0].frame_rect, 20, 178,
                          20, 16) &&
              rect_equals(&indoor.action_icons[4].fill_rect, 222, 180,
                          16, 12) &&
              indoor.action_icons[4].gdat_index ==
                  dm2_v1_viewport_hud_action_icon_graphic_index(4));
    CHECK("DM2 indoor HUD exposes four champion slots",
          indoor.champion_slot_count == DM2_V1_HUD_CHAMPION_SLOT_COUNT &&
              rect_equals(&indoor.portrait_panel_rect, 242, 28, 78, 144) &&
              rect_equals(&indoor.champion_slots[3].fill_rect,
                          246, 140, 68, 22) &&
              indoor.top_bar_gdat_index ==
                  dm2_v1_viewport_hud_core_graphic_index(
                      DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR) &&
              indoor.portrait_panel_gdat_index ==
                  dm2_v1_viewport_hud_core_graphic_index(
                      DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL));

    CHECK("DM2 outdoor HUD chrome plan builds",
          dm2_v1_viewport_build_hud_chrome_plan(1, &outdoor) == 1);
    CHECK("DM2 outdoor HUD suppresses champion portrait panel",
          outdoor.outdoor == 1 &&
              outdoor.champion_slot_count == 0 &&
              rect_equals(&outdoor.portrait_panel_rect, 0, 0, 0, 0));

    memset(&party, 0, sizeof(party));
    party.champion_count = 2;
    party.leader_index = 1;
    party.champions[0].occupied = 1;
    party.champions[0].hp_pct = 50;
    party.champions[0].stamina_pct = 70;
    party.champions[0].mana_pct = 10;
    party.champions[0].portrait_index = 3;
    memcpy(party.champions[0].name, "Theron", 6);
    party.champions[1].occupied = 1;
    party.champions[1].leader = 1;
    party.champions[1].hp_pct = 25;
    party.champions[1].stamina_pct = 40;
    party.champions[1].mana_pct = 100;
    memcpy(party.champions[1].name, "Karla", 5);
    CHECK("DM2 HUD party plan binds champion bars and leader marker",
          dm2_v1_viewport_build_hud_chrome_plan_for_party(
              0, &party, &indoor) == 1 &&
              indoor.champion_slots[0].occupied == 1 &&
              indoor.champion_slots[0].hp_pct == 50 &&
              indoor.champion_slots[0].portrait_index == 3 &&
              indoor.champion_slots[0].portrait_fill_color == 11 &&
              rect_equals(&indoor.champion_slots[0].hp_bar_rect,
                          270, 39, 34, 3) &&
              rect_equals(&indoor.champion_slots[0].hp_fill_rect,
                          270, 39, 17, 3) &&
              rect_equals(&indoor.champion_slots[0].stamina_fill_rect,
                          270, 44, 23, 3) &&
              rect_equals(&indoor.champion_slots[0].mana_fill_rect,
                          270, 49, 3, 3) &&
              indoor.champion_slots[1].leader == 1 &&
              rect_equals(&indoor.champion_slots[1].leader_mark_rect,
                          246, 69, 3, 3) &&
              rect_equals(&indoor.champion_slots[1].mana_fill_rect,
                          270, 85, 34, 3));
    CHECK("DM2 outdoor HUD party plan still suppresses champion slots",
          dm2_v1_viewport_build_hud_chrome_plan_for_party(
              1, &party, &outdoor) == 1 &&
              outdoor.outdoor == 1 &&
              outdoor.champion_slot_count == 0);
}

static void test_weather_overlay_render_plan(void)
{
    uint8_t framebuffer[320 * 200];
    DM2_V1_ViewportState viewport;
    DM2_V1_WeatherOverlayRenderPlan plan;
    DM2_V1_ViewportSceneConsumptionReceipt receipt;
    DM2_V1_WeatherOverlayCommandPlan commands;

    memset(framebuffer, 7, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);

    CHECK("DM2 weather plan rejects null output",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, NULL) == 0);
    CHECK("DM2 clear weather builds empty overlay plan",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_NONE);

    viewport.weather = DM2_V1_WEATHER_OVERLAY_RAIN;
    viewport.rain_intensity = 64;
    viewport.tick_count = 3;
    CHECK("DM2 rain plan owns density and scroll",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_RAIN &&
              plan.density == 7 &&
              plan.scroll == 4 &&
              plan.streak_step == 3 &&
              plan.rain_color == 15);
    CHECK("DM2 rain weather command owns streak material",
          dm2_v1_viewport_build_weather_overlay_commands(
              &plan, &commands) == 1 &&
              commands.command_count == 1 &&
              commands.commands[0].kind ==
                  DM2_V1_WEATHER_COMMAND_RAIN_STREAKS &&
              commands.commands[0].density == 7 &&
              commands.commands[0].scroll == 4 &&
              commands.commands[0].streak_step == 3 &&
              commands.commands[0].color == 15);
    dm2_v1_render_weather_overlay(&viewport);
    CHECK("DM2 rain overlay applies planned diagonal streak color",
          framebuffer[1] == 7 &&
              framebuffer[4] == 15 &&
              framebuffer[(3 * 320) + 4] == 15);

    memset(framebuffer, 8, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.weather = DM2_V1_WEATHER_OVERLAY_FOG;
    viewport.rain_intensity = 32;
    CHECK("DM2 fog plan owns alpha blend",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_FOG &&
              plan.alpha == 4 &&
              plan.fog_target_color == 0);
    CHECK("DM2 fog weather command owns blend material",
          dm2_v1_viewport_build_weather_overlay_commands(
              &plan, &commands) == 1 &&
              commands.command_count == 1 &&
              commands.commands[0].kind ==
                  DM2_V1_WEATHER_COMMAND_FOG_BLEND &&
              commands.commands[0].alpha == 4 &&
              commands.commands[0].target_color == 0);
    dm2_v1_render_weather_overlay(&viewport);
    CHECK("DM2 fog overlay applies planned alpha over framebuffer",
          framebuffer[0] == 6);

    memset(framebuffer, 3, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.weather = DM2_V1_WEATHER_OVERLAY_STORM;
    viewport.rain_intensity = 70;
    viewport.tick_count = 121;
    CHECK("DM2 storm plan owns lightning gate",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_STORM &&
              plan.density == 7 &&
              plan.scroll == 1 &&
              plan.lightning_flash == 1);
    CHECK("DM2 storm weather commands preserve pass order",
          dm2_v1_viewport_build_weather_overlay_commands(
              &plan, &commands) == 1 &&
              commands.command_count == 2 &&
              commands.commands[0].kind ==
                  DM2_V1_WEATHER_COMMAND_RAIN_STREAKS &&
              commands.commands[1].kind ==
                  DM2_V1_WEATHER_COMMAND_LIGHTNING_FILL &&
              commands.commands[1].color == 15);
    dm2_v1_render_weather_overlay(&viewport);
    CHECK("DM2 storm lightning applies planned full-screen flash",
          framebuffer[0] == 15 &&
              framebuffer[(199 * 320) + 319] == 15);

    memset(&plan, 0x55, sizeof(plan));
    CHECK("DM2 weather plan is null-state safe",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              NULL, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_NONE);
    CHECK("DM2 weather command plan is null-plan safe",
          dm2_v1_viewport_build_weather_overlay_commands(
              NULL, &commands) == 1 &&
              commands.command_count == 0);
    CHECK("DM2 weather command plan rejects null output",
          dm2_v1_viewport_build_weather_overlay_commands(
              &plan, NULL) == 0);

    memset(framebuffer, 7, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.weather = DM2_V1_WEATHER_OVERLAY_RAIN;
    viewport.rain_intensity = 60;
    viewport.tick_count = 3;
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport,
        1,
        0x32475357u,
        10,
        0x0004,
        3,
        12,
        3,
        0);
    CHECK("GRAPHICSSET void/flags bias weather plan intensity",
          dm2_v1_viewport_build_weather_overlay_render_plan(
              &viewport, &plan) == 1 &&
              plan.kind == DM2_V1_WEATHER_OVERLAY_RAIN &&
              plan.intensity == 67 &&
              plan.density == 7 &&
              plan.rain_color == 12);
    dm2_v1_render_weather_overlay(&viewport);
    CHECK("GRAPHICSSET weather fields are consumed by overlay render",
          viewport.gdat_scene_weather_consumed_count == 1);
    memset(&receipt, 0, sizeof(receipt));
    CHECK("viewport scene receipt captures weather GRAPHICSSET consumption",
          dm2_v1_viewport_scene_consumption_receipt(&viewport, &receipt) == 1 &&
              receipt.ready == 1 &&
              receipt.consumed_mask == 0x8u &&
              receipt.weather_consumed == 1 &&
              receipt.source_hash == 0x32475357u &&
              receipt.void_random_fall == 3 &&
              receipt.consumption_hash != 0u);
}

static void test_floor_ceiling_asset_provider(void)
{
    uint8_t framebuffer[320 * 200];
    DM2_V1_ViewportState viewport;
    DM2_V1_WallPanelRenderPlan wall_plan;
    DM2_V1_DoorRenderPlan door_plan;
    DM2_V1_ViewportSceneConsumptionReceipt scene_receipt;

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
    memset(&wall_plan, 0, sizeof(wall_plan));
    CHECK("wall panel render plan builds explicit asset-backed cells",
          dm2_v1_viewport_build_wall_panel_render_plan(&viewport,
                                                       &wall_plan) == 1 &&
              wall_plan.panel_count == 10 &&
              wall_plan.panels[0].render_step == 0 &&
              wall_plan.panels[0].view_square == DM2_SQ_D3L &&
              wall_plan.panels[0].gdat_index ==
                  dm2_v1_viewport_wall_graphic_index_for_square(
                      DM2_SQ_D3L) &&
              wall_plan.panels[8].view_square == DM2_SQ_D0L &&
              rect_equals(&wall_plan.panels[8].src_rect, 0, 0, 16, 136) &&
              rect_equals(&wall_plan.panels[8].dst_rect, 0, 0, 32, 136) &&
              wall_plan.panels[9].view_square == DM2_SQ_D0R &&
              rect_equals(&wall_plan.panels[9].dst_rect, 192, 0, 32, 136));
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
    viewport.tick_count = 5;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport,
        1,
        0x32475343u,
        10,
        0x0004,
        3,
        12,
        0,
        0x000a);
    dm2_v1_render_floor_ceiling(&viewport);
    CHECK("GRAPHICSSET scene light is consumed by floor/ceiling pass",
          viewport.gdat_scene_light_consumed_count == 1 &&
              viewport.gdat_scene_floor_anim_consumed_count == 1);
    memset(&scene_receipt, 0, sizeof(scene_receipt));
    CHECK("viewport scene receipt captures light/floor GRAPHICSSET consumption",
          dm2_v1_viewport_scene_consumption_receipt(&viewport,
                                                    &scene_receipt) == 1 &&
              scene_receipt.ready == 1 &&
              (scene_receipt.consumed_mask & 0x6u) == 0x6u &&
              scene_receipt.light_consumed == 1 &&
              scene_receipt.floor_anim_consumed == 1 &&
              scene_receipt.animated_floor == 0x000a &&
              scene_receipt.consumption_hash != 0u);
    CHECK("GRAPHICSSET animated floor offsets the live floor tile source",
          framebuffer[(66 * 320)] == 9 &&
              framebuffer[(66 * 320) + 1] == 8 &&
              framebuffer[(67 * 320)] == 7);

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
    s_asset_fetch_calls = 0;
    s_fail_asset_index =
        dm2_v1_viewport_wall_graphic_index_for_square(DM2_SQ_D1C);
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_walls(&viewport);
    CHECK("wall pass uses per-panel fallback when one asset is missing",
          s_asset_fetch_calls == 10 &&
              viewport.asset_wall_drawn_count == 9 &&
              viewport.fallback_wall_drawn_count == 1);
    s_fail_asset_index = 0;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    viewport.squares[DM2_SQ_D0C].door_button_state = 1;
    viewport.squares[DM2_SQ_D1C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D2C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0L].flags |= DM2_SQF_HAS_DOOR;
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan binds center door panel/frame/button rows",
          dm2_v1_viewport_build_door_render_plan(&viewport, &door_plan) == 1 &&
              door_plan.door_count == 3 &&
              door_plan.doors[0].view_square == DM2_SQ_D2C &&
              door_plan.doors[0].skproject_cell == 6 &&
              door_plan.doors[0].panel_gdat_index ==
                  dm2_v1_viewport_door_panel_graphic_index_for_square(
                      DM2_SQ_D2C) &&
              rect_equals(&door_plan.doors[0].panel_rect, 60, 20, 103, 71) &&
              rect_equals(&door_plan.doors[0].frame_rect, 60, 20, 104, 71) &&
              door_plan.doors[1].view_square == DM2_SQ_D1C &&
              door_plan.doors[1].frame_gdat_index ==
                  dm2_v1_viewport_door_frame_graphic_index_for_square(
                      DM2_SQ_D1C) &&
              door_plan.doors[2].view_square == DM2_SQ_D0C &&
              rect_equals(&door_plan.doors[2].panel_rect, 80, 0, 160, 135) &&
              rect_equals(&door_plan.doors[2].button_rect, 212, 58, 16, 18) &&
              door_plan.doors[2].button_gdat_index ==
                  dm2_v1_viewport_door_button_graphic_index_for_state(1));

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan owns front panel and frame routing",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 1 &&
              door_plan.doors[0].view_square == DM2_SQ_D0C &&
              door_plan.doors[0].skproject_cell == 0 &&
              door_plan.doors[0].panel_gdat_index ==
                  dm2_v1_viewport_door_panel_graphic_index_for_square(
                      DM2_SQ_D0C) &&
              door_plan.doors[0].frame_gdat_index ==
                  dm2_v1_viewport_door_frame_graphic_index_for_square(
                      DM2_SQ_D0C) &&
              door_plan.doors[0].door_open_pct == 0 &&
              rect_equals(&door_plan.doors[0].panel_rect, 80, 0, 160, 135) &&
              rect_equals(&door_plan.doors[0].panel_visible_rect,
                          80, 0, 160, 135) &&
              rect_equals(&door_plan.doors[0].frame_rect, 0, 0, 224, 136));

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_gfx_index = 7;
    viewport.squares[DM2_SQ_D0C].ornament_index = 2;
    viewport.squares[DM2_SQ_D0C].door_record_type = 1;
    viewport.squares[DM2_SQ_D0C].door_opening_dir = 1;
    viewport.squares[DM2_SQ_D0C].door_state = 5;
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan routes DB0 door type and overlays into GDAT indices",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 1 &&
              door_plan.doors[0].panel_gdat_index ==
                  dm2_v1_viewport_door_panel_graphic_index_for_record(
                      DM2_SQ_D0C, 7, 1) &&
              door_plan.doors[0].door_record_type == 1 &&
              door_plan.doors[0].door_gfx_index == 7 &&
              door_plan.doors[0].door_opening_dir == 1 &&
              door_plan.doors[0].ornament_index == 2 &&
              door_plan.doors[0].panel_gdat_index !=
                  dm2_v1_viewport_door_panel_graphic_index_for_square(
                      DM2_SQ_D0C) &&
              door_plan.doors[0].ornate_gdat_index ==
                  dm2_v1_viewport_door_ornate_graphic_index(2, DM2_SQ_D0C) &&
              door_plan.doors[0].destroyed_mask_gdat_index ==
                  dm2_v1_viewport_door_destroyed_mask_graphic_index(
                      7, DM2_SQ_D0C));
    dm2_v1_render_doors(&viewport);
    CHECK("door fallback counts when no asset provider is installed",
          viewport.asset_door_frame_drawn_count == 0 &&
              viewport.fallback_door_drawn_count == 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 50;
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan clips partially open panels from the top",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 1 &&
              door_plan.doors[0].door_open_pct == 50 &&
              rect_equals(&door_plan.doors[0].panel_visible_rect,
                          80, 67, 160, 68));

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_open_pct = 100;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("fully open door skips panel blit but keeps frame routing",
          s_asset_fetch_calls == 1 &&
              viewport.asset_door_panel_drawn_count == 0 &&
              viewport.asset_door_frame_drawn_count == 1);

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
    s_asset_fetch_calls = 0;
    s_fail_asset_index =
        dm2_v1_viewport_door_panel_graphic_index_for_square(DM2_SQ_D0C);
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_doors(&viewport);
    CHECK("door pass records panel fallback when frame GDAT succeeds",
          s_asset_fetch_calls == 2 &&
              viewport.asset_door_panel_drawn_count == 0 &&
              viewport.asset_door_frame_drawn_count == 1 &&
              viewport.fallback_door_drawn_count == 1);
    s_fail_asset_index = 0;

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.squares[DM2_SQ_D0C].flags |= DM2_SQF_HAS_DOOR;
    viewport.squares[DM2_SQ_D0C].door_button = 1;
    viewport.squares[DM2_SQ_D0C].door_button_state = 1;
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan owns pushed default button routing",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 1 &&
              door_plan.doors[0].button_gdat_index ==
                  dm2_v1_viewport_door_button_graphic_index_for_state(1) &&
              rect_equals(&door_plan.doors[0].button_rect, 212, 58, 16, 18));
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
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan owns custom wall-gfx button routing",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 1 &&
              door_plan.doors[0].button_gdat_index ==
                  dm2_v1_viewport_wall_button_graphic_index(0x2a, 0x07) &&
              rect_equals(&door_plan.doors[0].button_rect, 212, 58, 16, 18));
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
    memset(&door_plan, 0, sizeof(door_plan));
    CHECK("door render plan preserves D0/D1/D2 center-cell order",
          dm2_v1_viewport_build_door_render_plan(&viewport,
                                                 &door_plan) == 1 &&
              door_plan.door_count == 3 &&
              door_plan.doors[0].view_square == DM2_SQ_D2C &&
              door_plan.doors[0].skproject_cell == 6 &&
              rect_equals(&door_plan.doors[0].panel_rect, 60, 20, 103, 71) &&
              door_plan.doors[1].view_square == DM2_SQ_D1C &&
              door_plan.doors[1].skproject_cell == 3 &&
              rect_equals(&door_plan.doors[1].panel_rect, 60, 9, 104, 110) &&
              door_plan.doors[2].view_square == DM2_SQ_D0C &&
              door_plan.doors[2].skproject_cell == 0);
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
    DM2_V1_CreatureRenderPlan creature_plan;
    DM2_V1_ItemRenderPlan item_plan;
    DM2_V1_CarriedItemRenderPlan carried_item_plan;
    DM2_V1_CreaturePossessionItemRenderPlan possession_plan;
    DM2_V1_ProjectileRenderPlan projectile_plan;

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
    CHECK("DM2 object map-chip flip shares skproject possession flip table",
          dm2_v1_viewport_map_chip_flip_for_object_direction(0, 0) == 0 &&
              dm2_v1_viewport_map_chip_flip_for_object_direction(1, 0) == 1 &&
              dm2_v1_viewport_map_chip_flip_for_object_direction(2, 0) == 3 &&
              dm2_v1_viewport_map_chip_flip_for_object_direction(3, 0) == 2);
    CHECK("DM2 cloud frame follows skproject tick alternation",
          dm2_v1_viewport_cloud_frame_for_tick(0, 7) == 1 &&
              dm2_v1_viewport_cloud_frame_for_tick(1, 7) == 2);
    {
        uint32_t seed = 0x0100u;
        CHECK("DM2 cloud flip follows skproject RAND02 LCG",
              dm2_v1_viewport_cloud_flip_for_seed(&seed) == 1 &&
                  seed == 0x40e62d0bu);
    }
    CHECK("DM2 creature directional frame keeps short atlases animated",
          dm2_v1_viewport_creature_frame_for_direction(3, 1, 0, 2) == 1);
    CHECK("DM2 creature directional frame follows view-relative parity frames",
          dm2_v1_viewport_creature_frame_for_direction(2, 2, 0, 4) == 2 &&
              dm2_v1_viewport_creature_frame_for_direction(2, 1, 0, 4) == 3);
    {
        DM2_V1_ViewportSpritePlacement p;
        DM2_V1_ViewportSpritePlacement slot;
        const uint8_t rect14[14] =
            { 7, 0xfe, 3, 4, 5, 6, 64, 48, 32, 16, 1, 2, 4, 8 };
        DM2_V1_InterfaceRect14Placement rect_plan;

        CHECK("DM2 viewport projects forward map coordinate to depth row",
              dm2_v1_viewport_project_map_to_sprite(10, 8, 0, 10, 10, &p) == 1 &&
                  p.visible == 1 &&
                  p.depth == 1 &&
                  p.screen_x == 112 &&
                  p.screen_y == 84);
        CHECK("DM2 viewport projects side map coordinate with depth lateral step",
              dm2_v1_viewport_project_map_to_sprite(11, 8, 0, 10, 10, &p) == 1 &&
                  p.depth == 1 &&
                  p.screen_x == 152 &&
                  p.screen_y == 84);
        CHECK("DM2 viewport rejects map coordinate outside visible lane",
              dm2_v1_viewport_project_map_to_sprite(13, 8, 0, 10, 10, &p) == 0 &&
                  p.visible == 0);
        CHECK("DM2 viewport possession slot placement applies stable overlay offset",
              dm2_v1_viewport_project_map_to_sprite(10, 8, 0, 10, 10, &p) == 1 &&
                  dm2_v1_viewport_possession_slot_placement(&p, 2, &slot) == 1 &&
                  slot.depth == p.depth &&
                  slot.screen_x == p.screen_x + 12 &&
                  slot.screen_y == p.screen_y + 8);
        CHECK("DM2 viewport rotates 5x5 positions like skproject",
              dm2_v1_viewport_rotate_5x5_pos(7, 0) == 7 &&
                  dm2_v1_viewport_rotate_5x5_pos(7, 1) == 11 &&
                  dm2_v1_viewport_rotate_5x5_pos(7, 2) == 17 &&
                  dm2_v1_viewport_rotate_5x5_pos(7, 3) == 13);
        CHECK("DM2 viewport builds skproject creature blit rect ids",
              dm2_v1_viewport_creature_blit_rect_id(2, 7, 1) == 5061);
        CHECK("DM2 viewport consumes rect14 row placement semantics",
              dm2_v1_viewport_interface_rect14_placement(
                  rect14, 2, 52, &rect_plan) == 1 &&
                  rect_plan.valid == 1 &&
                  rect_plan.base_5x5 == 7 &&
                  rect_plan.lateral_offset == -2 &&
                  rect_plan.blit_rect_id[0] == 5057 &&
                  rect_plan.blit_rect_id[1] == 5061 &&
                  rect_plan.image_field[3] == 6 &&
                  rect_plan.stretched_size[0] == 52 &&
                  rect_plan.flags[3] == 8);
    }
    {
        DM2_V1_HudChromeRenderPlan hud;

        CHECK("DM2 HUD chrome plan owns status and action strip bounds",
              dm2_v1_viewport_build_hud_chrome_plan(0, &hud) == 1 &&
                  hud.top_bar_rect.x == 0 &&
                  hud.top_bar_rect.y == 0 &&
                  hud.top_bar_rect.w == 320 &&
                  hud.top_bar_rect.h == 28 &&
                  hud.action_strip_rect.y == 172 &&
                  hud.action_strip_rect.h == 28);
        CHECK("DM2 HUD chrome plan owns gold and action icon bounds",
              hud.gold_box_rect.x == 280 &&
                  hud.gold_box_rect.y == 176 &&
                  hud.action_icon_count == DM2_V1_HUD_ACTION_ICON_COUNT &&
                  hud.action_icons[4].frame_rect.x == 220 &&
                  hud.action_icons[4].fill_color == 12);
        CHECK("DM2 indoor HUD chrome plan includes portrait panel slots",
              hud.champion_slot_count == DM2_V1_HUD_CHAMPION_SLOT_COUNT &&
                  hud.portrait_panel_rect.x == 242 &&
                  hud.champion_slots[3].frame_rect.y == 138 &&
                  hud.champion_slots[3].fill_color == 14);
        CHECK("DM2 outdoor HUD chrome plan omits portrait panel slots",
              dm2_v1_viewport_build_hud_chrome_plan(1, &hud) == 1 &&
                  hud.outdoor == 1 &&
                  hud.champion_slot_count == 0 &&
                  hud.portrait_panel_rect.w == 0);
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("DM2 UI chrome render consumes HUD plan regions",
          framebuffer[0] == 1 &&
              framebuffer[28 * 320] == 7 &&
              framebuffer[176 * 320 + 280] == 6 &&
              framebuffer[180 * 320 + 288] == 11 &&
              framebuffer[180 * 320 + 222] == 12 &&
              framebuffer[138 * 320 + 244] == 7 &&
              framebuffer[140 * 320 + 246] == 14);

    {
        DM2_V1_HudPartyState party;
        DM2_V1_InterfaceTheme theme;

        memset(&party, 0, sizeof(party));
        party.champion_count = 1;
        party.leader_index = 0;
        party.champions[0].occupied = 1;
        party.champions[0].leader = 1;
        party.champions[0].hp_pct = 50;
        party.champions[0].stamina_pct = 70;
        party.champions[0].mana_pct = 10;
        party.champions[0].portrait_index = 3;
        memcpy(party.champions[0].name, "Theron", 6);
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, 320);
        dm2_v1_viewport_set_hud_party(&viewport, &party);
        dm2_v1_render_ui_chrome(&viewport);
        CHECK("DM2 UI chrome renders bound champion HUD bars",
              framebuffer[33 * 320 + 246] == 15 &&
                  framebuffer[34 * 320 + 250] == 11 &&
                  framebuffer[32 * 320 + 250] == 9 &&
                  framebuffer[39 * 320 + 270] == 2 &&
                  framebuffer[39 * 320 + 287] == 0 &&
                  framebuffer[44 * 320 + 292] == 11 &&
                  framebuffer[44 * 320 + 294] == 0 &&
                  framebuffer[49 * 320 + 272] == 12 &&
                  framebuffer[49 * 320 + 274] == 0);
        CHECK("DM2 UI chrome tracks placeholder portrait fallback",
              viewport.asset_hud_portrait_drawn_count == 0 &&
                  viewport.fallback_hud_portrait_drawn_count == 1);

        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, 320);
        dm2_v1_viewport_set_hud_party(&viewport, &party);
        s_asset_fetch_calls = 0;
        dm2_v1_viewport_set_asset_provider(&viewport,
                                           test_dm2_asset_fetch,
                                           NULL);
        dm2_v1_render_ui_chrome(&viewport);
        CHECK("DM2 UI chrome fetches and scales HUD portrait assets",
              s_asset_fetch_calls == 10 &&
                  viewport.asset_hud_core_drawn_count == 9 &&
                  viewport.fallback_hud_core_drawn_count == 0 &&
                  viewport.last_hud_core_gdat_hash != 2166136261u &&
                  viewport.last_hud_core_pixel_count > 0u &&
                  viewport.asset_hud_portrait_drawn_count == 1 &&
                  viewport.fallback_hud_portrait_drawn_count == 0 &&
                  s_last_asset_index ==
                      dm2_v1_viewport_hud_portrait_graphic_index(3) &&
                  framebuffer[34 * 320 + 250] == 3 &&
                  framebuffer[34 * 320 + 262] == 4 &&
                  framebuffer[44 * 320 + 250] == 5 &&
                  framebuffer[44 * 320 + 262] == 6);

        memset(&theme, 0, sizeof(theme));
        theme.valid = 1;
        theme.semantic_hash = 0x12345678u;
        theme.action_table_byte_count = 10u;
        theme.font_table_byte_count = 20u;
        theme.palette_byte_count = 30u;
        theme.rect14_ready = 1;
        theme.rect14_hash = 0x12345678u;
        theme.rect14_byte_count = 28u;
        theme.rect14_row_count = 2u;
        theme.chrome_divider_color = 5u;
        theme.action_icon_base_color = 6u;
        theme.champion_frame_color = 9u;
        theme.gold_coin_color = 11u;
        theme.gold_label_color = 12u;
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v1_viewport_init(&viewport, framebuffer, 320);
        dm2_v1_viewport_set_hud_party(&viewport, &party);
        dm2_v1_viewport_set_interface_theme(&viewport, &theme);
        dm2_v1_render_ui_chrome(&viewport);
        CHECK("DM2 UI chrome consumes rect14 placement semantics live",
              viewport.interface_semantics_consumed == 1 &&
                  viewport.interface_rect14_consumed == 1 &&
                  viewport.interface_semantics_hash == theme.semantic_hash &&
                  viewport.interface_semantics_byte_count == 88u &&
                  framebuffer[180 * 320 + 222] != 6 &&
                  framebuffer[180 * 320 + 223] == 6);
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    dm2_v1_viewport_set_outdoor(&viewport, 1);
    dm2_v1_render_ui_chrome(&viewport);
    CHECK("DM2 outdoor UI chrome skips portrait panel region",
          framebuffer[140 * 320 + 246] == 0 &&
              framebuffer[180 * 320 + 222] == 12);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.creature_count = 1;
    viewport.creatures[0].creature_type = 0x12;
    viewport.creatures[0].frame_index = 0x01;
    viewport.creatures[0].screen_x = 40;
    viewport.creatures[0].screen_y = 50;
    viewport.creatures[0].health_pct = 75;
    memset(&creature_plan, 0, sizeof(creature_plan));
    CHECK("DM2 creature render plan owns sprite identity and fallback bounds",
          dm2_v1_viewport_build_creature_render_plan(&viewport,
                                                     &creature_plan) == 1 &&
              creature_plan.creature_count == 1 &&
              creature_plan.creatures[0].creature_index == 0 &&
              creature_plan.creatures[0].gdat_index ==
                  dm2_v1_viewport_creature_graphic_index(0x12, 0x01) &&
              rect_equals(&creature_plan.creatures[0].fallback_rect,
                          36, 46, 8, 8) &&
              creature_plan.creatures[0].fallback_color ==
                  (uint8_t)(11 + (0x12 & 7)) &&
              rect_equals(&creature_plan.creatures[0].health_bg_rect,
                          32, 42, 16, 1) &&
              rect_equals(&creature_plan.creatures[0].health_fill_rect,
                          32, 42, 12, 1));
    dm2_v1_render_creatures(&viewport);
    CHECK("creature fallback draws when no sprite asset provider is installed",
          viewport.asset_creature_drawn_count == 0 &&
              viewport.fallback_creature_drawn_count == 1 &&
              framebuffer[(50 * 320) + 40] == (uint8_t)(11 + (0x12 & 7)) &&
              framebuffer[(42 * 320) + 32] == 2 &&
              framebuffer[(42 * 320) + 45] == 4);

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
    viewport.creature_count = 2;
    viewport.creatures[0].creature_type = 0x12;
    viewport.creatures[0].screen_x = -1;
    viewport.creatures[0].screen_y = 50;
    viewport.creatures[1].creature_type = 0x13;
    viewport.creatures[1].frame_index = 0x02;
    viewport.creatures[1].screen_x = 80;
    viewport.creatures[1].screen_y = 90;
    memset(&creature_plan, 0, sizeof(creature_plan));
    CHECK("DM2 creature render plan filters offscreen sprites before draw",
          dm2_v1_viewport_build_creature_render_plan(&viewport,
                                                     &creature_plan) == 1 &&
              creature_plan.creature_count == 1 &&
              creature_plan.creatures[0].creature_index == 1 &&
              creature_plan.creatures[0].center_x == 80 &&
              creature_plan.creatures[0].center_y == 90);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.item_count = 1;
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].frame_index = 0x04;
    viewport.items[0].screen_x = 80;
    viewport.items[0].screen_y = 90;
    memset(&item_plan, 0, sizeof(item_plan));
    CHECK("DM2 item render plan owns map-chip identity and fallback",
          dm2_v1_viewport_build_item_render_plan(&viewport,
                                                 &item_plan) == 1 &&
              item_plan.item_count == 1 &&
              item_plan.items[0].item_index == 0 &&
              item_plan.items[0].item_category == 0x10 &&
              item_plan.items[0].item_type == 0x22 &&
              item_plan.items[0].frame_index == 0x04 &&
              item_plan.items[0].center_x == 80 &&
              item_plan.items[0].center_y == 90 &&
              item_plan.items[0].gdat_index ==
                  dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0x04) &&
              item_plan.items[0].fallback_radius == 4 &&
              item_plan.items[0].fallback_color == 3);
    dm2_v1_render_items(&viewport);
    CHECK("item fallback draws when no sprite asset provider is installed",
          viewport.asset_item_drawn_count == 0 &&
              viewport.fallback_item_drawn_count == 1 &&
              framebuffer[(90 * 320) + 80] == 3);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.item_count = 2;
    viewport.items[0].item_category = 0x10;
    viewport.items[0].item_type = 0x22;
    viewport.items[0].screen_x = 80;
    viewport.items[0].screen_y = 200;
    viewport.items[1].item_type = 0x23;
    viewport.items[1].screen_x = 81;
    viewport.items[1].screen_y = 91;
    memset(&item_plan, 0, sizeof(item_plan));
    CHECK("DM2 item render plan filters offscreen floor objects",
          dm2_v1_viewport_build_item_render_plan(&viewport,
                                                 &item_plan) == 1 &&
              item_plan.item_count == 1 &&
              item_plan.items[0].item_index == 1 &&
              item_plan.items[0].item_category == 0x15 &&
              item_plan.items[0].center_x == 81 &&
              item_plan.items[0].center_y == 91);

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
    viewport.creature_possession_item_count = 1;
    viewport.creature_possession_items[0].item_category = 0x10;
    viewport.creature_possession_items[0].item_type = 0x22;
    viewport.creature_possession_items[0].frame_index = 0x04;
    viewport.creature_possession_items[0].screen_x = 100;
    viewport.creature_possession_items[0].screen_y = 80;
    memset(&possession_plan, 0, sizeof(possession_plan));
    CHECK("DM2 creature possession render plan owns map-chip identity",
          dm2_v1_viewport_build_creature_possession_item_render_plan(
              &viewport,
              &possession_plan) == 1 &&
              possession_plan.item_count == 1 &&
              possession_plan.items[0].item_index == 0 &&
              possession_plan.items[0].item_category == 0x10 &&
              possession_plan.items[0].item_type == 0x22 &&
              possession_plan.items[0].frame_index == 0 &&
              possession_plan.items[0].center_x == 100 &&
              possession_plan.items[0].center_y == 80 &&
              possession_plan.items[0].gdat_index ==
                  dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0) &&
              possession_plan.items[0].fallback_radius == 3 &&
              possession_plan.items[0].fallback_color ==
                  (uint8_t)(9 + (0x22 & 5)));
    dm2_v1_render_creature_possession_items(&viewport);
    CHECK("creature possession item fallback draws when no asset provider is installed",
          viewport.asset_creature_possession_item_drawn_count == 0 &&
              viewport.fallback_creature_possession_item_drawn_count == 1 &&
              framebuffer[(80 * 320) + 100] == (uint8_t)(9 + (0x22 & 5)));

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.creature_possession_item_count = 1;
    viewport.creature_possession_items[0].item_category = 0x10;
    viewport.creature_possession_items[0].item_type = 0x22;
    viewport.creature_possession_items[0].frame_index = 0x04;
    viewport.creature_possession_items[0].direction = 1;
    viewport.creature_possession_items[0].screen_x = 100;
    viewport.creature_possession_items[0].screen_y = 80;
    memset(&possession_plan, 0, sizeof(possession_plan));
    CHECK("DM2 creature possession render plan owns object flip",
          dm2_v1_viewport_build_creature_possession_item_render_plan(
              &viewport,
              &possession_plan) == 1 &&
              possession_plan.item_count == 1 &&
              possession_plan.items[0].flip_mirror ==
                  dm2_v1_viewport_map_chip_flip_for_object_direction(1, 0));
    s_item_flip_fixture = 1;
    s_asset_fetch_calls = 0;
    s_last_asset_index = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_creature_possession_items(&viewport);
    s_item_flip_fixture = 0;
    CHECK("creature possession overlay uses item map-chip asset path",
          s_asset_fetch_calls == 1 &&
              s_last_asset_index ==
                  dm2_v1_viewport_item_graphic_index(0x10, 0x22, 0) &&
              viewport.asset_creature_possession_item_drawn_count == 1 &&
              viewport.fallback_creature_possession_item_drawn_count == 0);
    CHECK("creature possession overlay applies skproject object flip mirror",
          framebuffer[((80 - 3) * 320) + (100 - 3)] == 36 &&
              framebuffer[((80 - 3) * 320) + (100 + 3)] == 30);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.carried_item_present = 1;
    viewport.carried_item.item_category = 0x15;
    viewport.carried_item.item_type = 0x22;
    viewport.carried_item.frame_index = 0x04;
    viewport.carried_item.screen_x = 120;
    viewport.carried_item.screen_y = 70;
    memset(&carried_item_plan, 0, sizeof(carried_item_plan));
    CHECK("DM2 carried item render plan owns leader-hand map-chip identity",
          dm2_v1_viewport_build_carried_item_render_plan(
              &viewport,
              &carried_item_plan) == 1 &&
              carried_item_plan.item_present == 1 &&
              carried_item_plan.item.item_category == 0x15 &&
              carried_item_plan.item.item_type == 0x22 &&
              carried_item_plan.item.gdat_index ==
                  dm2_v1_viewport_item_graphic_index(0x15, 0x22, 0x04) &&
              carried_item_plan.item.center_x == 120 &&
              carried_item_plan.item.center_y == 70 &&
              carried_item_plan.item.fallback_radius == 5 &&
              carried_item_plan.item.fallback_color ==
                  (uint8_t)(12 + (0x22 & 3)));
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
    viewport.carried_item_present = 1;
    viewport.carried_item.item_type = 0x22;
    viewport.carried_item.screen_x = 320;
    viewport.carried_item.screen_y = 70;
    memset(&carried_item_plan, 0, sizeof(carried_item_plan));
    CHECK("DM2 carried item render plan filters offscreen leader-hand overlays",
          dm2_v1_viewport_build_carried_item_render_plan(
              &viewport,
              &carried_item_plan) == 1 &&
              carried_item_plan.item_present == 0);

    memset(framebuffer, 0, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, 320);
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x02;
    viewport.projectiles[0].frame_index = 0x01;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    viewport.projectiles[0].velocity_x = 3;
    memset(&projectile_plan, 0, sizeof(projectile_plan));
    CHECK("DM2 projectile render plan owns missile identity and fallback",
          dm2_v1_viewport_build_projectile_render_plan(&viewport,
                                                       &projectile_plan) == 1 &&
              projectile_plan.projectile_count == 1 &&
              projectile_plan.projectiles[0].projectile_index == 0 &&
              projectile_plan.projectiles[0].projectile_category == 0x0d &&
              projectile_plan.projectiles[0].projectile_type == 0x02 &&
              projectile_plan.projectiles[0].frame_index == 0x01 &&
              projectile_plan.projectiles[0].center_x == 120 &&
              projectile_plan.projectiles[0].center_y == 70 &&
              projectile_plan.projectiles[0].gdat_index ==
                  dm2_v1_viewport_projectile_graphic_index(0x0d, 0x02, 0x01) &&
              projectile_plan.projectiles[0].flip_mirror ==
                  dm2_v1_viewport_projectile_flip_for_direction(0, 0) &&
              projectile_plan.projectiles[0].fallback_dx == 3 &&
              projectile_plan.projectiles[0].fallback_dy == 0 &&
              projectile_plan.projectiles[0].fallback_len == 3 &&
              projectile_plan.projectiles[0].fallback_color == 15);
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
    memset(&projectile_plan, 0, sizeof(projectile_plan));
    CHECK("DM2 projectile render plan owns directional frame inputs",
          dm2_v1_viewport_build_projectile_render_plan(&viewport,
                                                       &projectile_plan) == 1 &&
              projectile_plan.projectile_count == 1 &&
              projectile_plan.projectiles[0].direction == 1 &&
              projectile_plan.projectiles[0].object_direction == 1 &&
              projectile_plan.projectiles[0].frame_class ==
                  DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL &&
              projectile_plan.projectiles[0].render_kind ==
                  DM2_V1_PROJECTILE_RENDER_MISSILE &&
              projectile_plan.projectiles[0].flip_mirror ==
                  dm2_v1_viewport_projectile_flip_for_direction(1, 0) &&
              projectile_plan.projectiles[0].cloud_flip_from_seed == 0);
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
    viewport.random_seed = 0x0100u;
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x87;
    viewport.projectiles[0].frame_index = 0x00;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].render_kind = DM2_V1_PROJECTILE_RENDER_CLOUD;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    memset(&projectile_plan, 0, sizeof(projectile_plan));
    CHECK("DM2 projectile render plan marks clouds for seeded flip",
          dm2_v1_viewport_build_projectile_render_plan(&viewport,
                                                       &projectile_plan) == 1 &&
              projectile_plan.projectile_count == 1 &&
              projectile_plan.projectiles[0].render_kind ==
                  DM2_V1_PROJECTILE_RENDER_CLOUD &&
              projectile_plan.projectiles[0].cloud_flip_from_seed == 1 &&
              viewport.random_seed == 0x0100u);
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
    viewport.tick_count = 0;
    viewport.random_seed = 0x0100u;
    viewport.projectile_count = 1;
    viewport.projectiles[0].projectile_category = 0x0d;
    viewport.projectiles[0].projectile_type = 0x87;
    viewport.projectiles[0].frame_index = 0x00;
    viewport.projectiles[0].direction = 1;
    viewport.projectiles[0].render_kind = DM2_V1_PROJECTILE_RENDER_CLOUD;
    viewport.projectiles[0].screen_x = 120;
    viewport.projectiles[0].screen_y = 70;
    s_projectile_flip_fixture = 1;
    s_asset_fetch_calls = 0;
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       test_dm2_asset_fetch,
                                       NULL);
    dm2_v1_render_projectiles(&viewport);
    s_projectile_flip_fixture = 0;
    CHECK("cloud render applies skproject RAND02 flip mirror",
          viewport.asset_projectile_drawn_count == 1 &&
              viewport.random_seed == 0x40e62d0bu &&
              framebuffer[((70 - 3) * 320) + (120 - 3)] == 27 &&
              framebuffer[((70 - 3) * 320) + (120 + 3)] == 21);

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
        CHECK("source evidence cites skproject creature possession overlays",
              e != NULL && strstr(e, "creature possession item overlays") != NULL);
        CHECK("source evidence cites DM2 palette documentation",
              e != NULL && strstr(e, "docs/dm2_palette.md") != NULL);
    }
    test_door_rect_contracts();
    test_hud_chrome_render_plan();
    test_weather_overlay_render_plan();
    test_floor_ceiling_asset_provider();
    test_sprite_asset_provider();

    printf("\nDM2 V1 Lighting/Palette Runtime Gate: %d/%d passed\n",
           s_tests_passed, s_tests_run);
    return (s_tests_passed == s_tests_run) ? 0 : 1;
}
