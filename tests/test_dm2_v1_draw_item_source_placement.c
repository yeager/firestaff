/* DM2 V1 DRAW_ITEM source placement: 5x5 anchor rotation, visibility mask,
 * display order, source-plan view rotation, render-plan placement fill and
 * asset-blit scale/flip/slot rules.
 * Source: skproject/SKWIN/SkWinCore.cpp DRAW_ITEM (_32cb_3672),
 * DRAW_PUT_DOWN_ITEM (_32cb_3991), DRAW_STATIC_OBJECT (_32cb_3b9d),
 * QUERY_OBJECT_5x5_POS (_48ae_07fd), DIR_FROM_5x5_POS (_48ae_07bf),
 * ROTATE_5x5_POS (_098d_0c50) and SkGlobal.cpp _4976_4a04/_4976_41b0/
 * _4976_41de/_4976_418e/tlbDisplayOrder*. */

#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); \
} while (0)

static void test_object_5x5_pos(void)
{
    static const uint8_t anchor[4] = { 6, 8, 18, 16 };
    int dir;
    int view;

    for (dir = 0; dir < 4; ++dir) {
        for (view = 0; view < 4; ++view) {
            /* QUERY_OBJECT_5x5_POS == ROTATE_5x5_POS(_4976_4a04[dir], view);
             * for the corner anchors this is _4976_4a04[(dir - view) & 3]. */
            CHECK("object 5x5 pos matches ROTATE_5x5_POS",
                  dm2_v1_viewport_object_5x5_pos(dir, view) ==
                      dm2_v1_viewport_rotate_5x5_pos(anchor[dir], view));
            CHECK("object 5x5 pos matches relative-direction anchor",
                  dm2_v1_viewport_object_5x5_pos(dir, view) ==
                      anchor[(dir - view) & 3]);
            CHECK("visibility bit matches 5x5 pos",
                  dm2_v1_viewport_static_object_visibility_bit(dir, view) ==
                      (1u << (unsigned)anchor[(dir - view) & 3]));
        }
    }
    CHECK("negative direction is rejected",
          dm2_v1_viewport_object_5x5_pos(-1, 0) == -1);
    CHECK("negative view is rejected",
          dm2_v1_viewport_object_5x5_pos(0, -1) == -1);
    CHECK("invalid visibility bit is zero",
          dm2_v1_viewport_static_object_visibility_bit(-1, 0) == 0u);
}

static void test_dir_from_5x5_pos(void)
{
    CHECK("DIR_FROM_5x5_POS corners",
          dm2_v1_viewport_dir_from_5x5_pos(6) == 0 &&
          dm2_v1_viewport_dir_from_5x5_pos(8) == 1 &&
          dm2_v1_viewport_dir_from_5x5_pos(18) == 2 &&
          dm2_v1_viewport_dir_from_5x5_pos(16) == 3);
    CHECK("DIR_FROM_5x5_POS center",
          dm2_v1_viewport_dir_from_5x5_pos(12) == 4);
    CHECK("DIR_FROM_5x5_POS rejects non-anchor positions",
          dm2_v1_viewport_dir_from_5x5_pos(0) == -1 &&
          dm2_v1_viewport_dir_from_5x5_pos(25) == -1 &&
          dm2_v1_viewport_dir_from_5x5_pos(7) == -1);
}

static void test_display_order(void)
{
    static const uint8_t expect_left[25] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24
    };
    static const uint8_t expect_right[25] = {
        4, 3, 2, 1, 0, 9, 8, 7, 6, 5, 14, 13, 12,
        11, 10, 19, 18, 17, 16, 15, 24, 23, 22, 21, 20
    };
    static const uint8_t expect_center[25] = {
        0, 4, 1, 3, 2, 5, 9, 6, 8, 7, 10, 14, 11,
        13, 12, 15, 19, 16, 18, 17, 20, 24, 21, 23, 22
    };
    uint8_t order[25];

    memset(order, 0xff, sizeof(order));
    CHECK("cell 0 iterates only the first 15 center display-order entries",
          dm2_v1_viewport_static_object_display_order(0, order) == 15 &&
          memcmp(order, expect_center, 15) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D1L uses the source left display order",
          dm2_v1_viewport_static_object_display_order(1, order) == 25 &&
          memcmp(order, expect_left, 25) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D1R uses the source right display order",
          dm2_v1_viewport_static_object_display_order(2, order) == 25 &&
          memcmp(order, expect_right, 25) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D1C uses the source center display order",
          dm2_v1_viewport_static_object_display_order(3, order) == 25 &&
          memcmp(order, expect_center, 25) == 0);
    CHECK("D2R uses the source right display order",
          dm2_v1_viewport_static_object_display_order(5, order) == 25 &&
          memcmp(order, expect_right, 25) == 0);
    CHECK("D2C uses the source center display order",
          dm2_v1_viewport_static_object_display_order(6, order) == 25 &&
          memcmp(order, expect_center, 25) == 0);
    CHECK("unknown cells have no display order",
          dm2_v1_viewport_static_object_display_order(-1, order) == 0 &&
          dm2_v1_viewport_static_object_display_order(16, order) == 0 &&
          dm2_v1_viewport_static_object_display_order(3, NULL) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D2L far-side cell uses the source left display order",
          dm2_v1_viewport_static_object_display_order(9, order) == 25 &&
          memcmp(order, expect_left, 25) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D3 far-left cell uses the source left display order",
          dm2_v1_viewport_static_object_display_order(14, order) == 25 &&
          memcmp(order, expect_left, 25) == 0);
    memset(order, 0xff, sizeof(order));
    CHECK("D3 far-right cell uses the source right display order",
          dm2_v1_viewport_static_object_display_order(15, order) == 25 &&
          memcmp(order, expect_right, 25) == 0);
}

static void test_source_plan_view_rotation(void)
{
    DM2_V1_StaticObjectSourcePlan plan;
    DM2_V1_StaticObjectSourcePlan rotated;

    CHECK("D1C DB5 north view 0 keeps the cycle-13 anchor",
          dm2_v1_viewport_static_object_source_plan(
              3, 17, 0x10, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 1 &&
          plan.position_5x5 == 6 &&
          plan.clip_rect_id == (0x8000 | 5081) &&
          plan.stretch_factor64 == 0x40 &&
          plan.slot_x_offset == 2 &&
          plan.slot_y_offset == -3);
    CHECK("record dir 1 with view 1 rotates to the same view anchor",
          dm2_v1_viewport_static_object_source_plan(
              3, 17, 0x10, 1, 0, 0, 1, 1u, 1u << 6, &rotated) == 1 &&
          rotated.position_5x5 == plan.position_5x5 &&
          rotated.clip_rect_id == plan.clip_rect_id &&
          rotated.object_direction == 1);
    CHECK("record dir 0 with view 1 rotates to the west anchor",
          dm2_v1_viewport_static_object_source_plan(
              3, 17, 0x10, 0, 0, 0, 1, 1u, 1u << 16, &rotated) == 1 &&
          rotated.position_5x5 == 16 &&
          rotated.clip_rect_id == (0x8000 | (5075 + 16)));
    CHECK("the source visibility bit always covers the plan anchor",
          dm2_v1_viewport_static_object_source_plan(
              6, 14, 0x14, 2, 0, 0, 3, 1u,
              dm2_v1_viewport_static_object_visibility_bit(2, 3),
              &rotated) == 1 &&
          rotated.position_5x5 ==
              dm2_v1_viewport_object_5x5_pos(2, 3) &&
          (rotated.visibility_mask_5x5 &
           (1u << (unsigned)rotated.position_5x5)) != 0u);
}

static void test_render_plan_placement_fill(void)
{
    DM2_V1_ViewportState s;
    DM2_V1_ItemRenderPlan plan;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&s, 0, 10, 10);

    /* D1C DB5 weapon admitted through the source cell/pass/clip route. */
    s.item_count = 2;
    memset(s.items, 0, sizeof(s.items));
    s.items[0].item_category = 0x10;
    s.items[0].item_type = 0x22;
    s.items[0].screen_x = 96;
    s.items[0].screen_y = 88;
    s.items[0].direction = 0;
    s.items[0].source_gdat_field = 0xf9;
    s.items[0].source_g1_weapon = 1;
    s.items[0].source_static_object_admitted = 1;
    s.items[0].source_static_object_cell = 3;
    s.items[0].source_static_object_pass = 17;
    s.items[0].source_static_object_clip_rect_id = 5081;

    /* D2C DB9 container admitted through the source route, record dir 1. */
    s.items[1].item_category = 0x14;
    s.items[1].item_type = 3;
    s.items[1].screen_x = 100;
    s.items[1].screen_y = 90;
    s.items[1].direction = 1;
    s.items[1].source_gdat_field = 0xf9;
    s.items[1].source_g1_container = 1;
    s.items[1].source_static_object_admitted = 1;
    s.items[1].source_static_object_cell = 6;
    s.items[1].source_static_object_pass = 14;
    s.items[1].source_static_object_clip_rect_id = 5158;

    CHECK("render plan builds",
          dm2_v1_viewport_build_item_render_plan(&s, &plan) == 1 &&
          plan.item_count == 2);

    /* table1d7029 pass order draws the D2C container (pass 14) before the
     * D1C weapon (pass 17). */
    CHECK("source pass ordering is preserved",
          plan.items[0].item_category == 0x14 &&
          plan.items[1].item_category == 0x10);

    CHECK("DB9 row carries the source DRAW_ITEM placement",
          plan.items[0].source_static_object_placement_valid == 1 &&
          plan.items[0].source_static_object_position_5x5 == 8 &&
          plan.items[0].source_static_object_stretch_factor64 == 0x2b &&
          plan.items[0].source_static_object_image_field == 0 &&
          plan.items[0].source_static_object_flip_mirror == 1 &&
          plan.items[0].source_static_object_slot_x_offset == 2 &&
          plan.items[0].source_static_object_slot_y_offset == -3);
    CHECK("DB5 row carries the source DRAW_ITEM placement",
          plan.items[1].source_static_object_placement_valid == 1 &&
          plan.items[1].source_static_object_position_5x5 == 6 &&
          plan.items[1].source_static_object_stretch_factor64 == 0x40 &&
          plan.items[1].source_static_object_image_field == 0 &&
          plan.items[1].source_static_object_flip_mirror == 0);

    /* A stale clip rectangle must keep the row fail-closed. */
    s.items[0].source_static_object_clip_rect_id = 5099;
    CHECK("clip-rect mismatch blocks the placement fill",
          dm2_v1_viewport_build_item_render_plan(&s, &plan) == 1 &&
          plan.items[1].source_static_object_placement_valid == 0);
    s.items[0].source_static_object_clip_rect_id = 5081;

    /* Non-admitted items never receive source placement.  With the weapon
     * unadmitted the pass sort no longer applies, so it stays at row 0. */
    s.items[0].source_static_object_admitted = 0;
    CHECK("unadmitted item stays without source placement",
          dm2_v1_viewport_build_item_render_plan(&s, &plan) == 1 &&
          plan.items[0].source_static_object_placement_valid == 0 &&
          plan.items[1].source_static_object_placement_valid == 1);
}

static void test_asset_blit_scale_flip_slot(void)
{
    DM2_V1_ItemRender row;
    DM2_V1_ItemAssetBlit blit;

    memset(&row, 0, sizeof(row));
    row.gdat_index = 7;
    row.center_x = 100;
    row.center_y = 90;
    row.source_static_object_placement_valid = 1;
    row.source_static_object_stretch_factor64 = 0x40;
    row.source_static_object_slot_x_offset = 2;
    row.source_static_object_slot_y_offset = -3;
    row.source_static_object_position_5x5 = 6;

    CHECK("DRAW_ITEM stretch factor 64 is identity with slot deltas",
          dm2_v1_viewport_item_asset_blit(&row, 8, 8, 8, 0, 4, 32,
                                          &blit) == 1 &&
          blit.dst_rect.w == 8 && blit.dst_rect.h == 8 &&
          blit.dst_rect.x == 100 - 4 + 2 &&
          blit.dst_rect.y == 90 - 4 - 3 &&
          blit.flip_mirror == 0);

    /* DRAW_ITEM lines 23973-23977: the record's dtImageOffset adds its signed
     * high byte to x and its signed low byte to y. */
    row.source_static_object_image_offset = 0x02feu;
    CHECK("dtImageOffset shifts the DRAW_ITEM anchor",
          dm2_v1_viewport_item_asset_blit(&row, 8, 8, 8, 0, 4, 32,
                                          &blit) == 1 &&
          blit.dst_rect.x == 100 - 4 + 2 + 2 &&
          blit.dst_rect.y == 90 - 4 - 3 - 2);
    row.source_static_object_image_offset = 0;

    /* D2C anchor: _4976_418e[2][0] = 0x2b stretches 8 -> (8*43+21)>>6 = 5. */
    row.source_static_object_stretch_factor64 = 0x2b;
    row.source_static_object_flip_mirror = 1;
    CHECK("DRAW_ITEM distance stretch and chest mirror apply",
          dm2_v1_viewport_item_asset_blit(&row, 8, 8, 8, 0, 4, 32,
                                          &blit) == 1 &&
          blit.dst_rect.w == 5 && blit.dst_rect.h == 5 &&
          blit.flip_mirror == 1);

    /* A Rect14-governed row keeps priority over the static placement.
     * CALC_STRETCHED_SIZE(8, 0x20) = (8*32+16)>>6 = 4. */
    row.rect14_applied = 1;
    row.rect14_scale64 = 0x20;
    row.rect14_flip_mirror = 0;
    row.rect14_lateral_offset = 0;
    CHECK("Rect14 placement wins over static-object placement",
          dm2_v1_viewport_item_asset_blit(&row, 8, 8, 8, 0, 4, 32,
                                          &blit) == 1 &&
          blit.dst_rect.w == 4 && blit.dst_rect.h == 4 &&
          blit.flip_mirror == 0);
}

static void test_static_object_rect14_handoff(void)
{
    DM2_V1_ViewportState s;
    DM2_V1_ItemRenderPlan plan;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&s, 0, 10, 10);
    s.item_count = 1;
    s.items[0].item_category = 0x10;
    s.items[0].item_type = 0x22;
    s.items[0].screen_x = 96;
    s.items[0].screen_y = 88;
    s.items[0].direction = 0;
    s.items[0].source_gdat_field = 0;
    s.items[0].source_g1_weapon = 1;
    s.items[0].source_static_object_admitted = 1;
    s.items[0].source_static_object_cell = 3;
    s.items[0].source_static_object_pass = 17;
    s.items[0].source_static_object_clip_rect_id = 5081;
    s.items[0].source_static_object_rect14_applied = 1;
    s.items[0].source_static_object_rect14_scale64 = 0x20;
    s.items[0].source_static_object_rect14_lateral_offset = -3;
    s.items[0].source_static_object_rect14_flip_mirror = 1;
    s.items[0].source_static_object_rect14_row_hash = 0x11111111u;
    s.items[0].source_static_object_rect14_placement_hash = 0x22222222u;

    CHECK("static object carries its admitted Rect14 placement",
          dm2_v1_viewport_build_item_render_plan(&s, &plan) == 1 &&
          plan.item_count == 1 && plan.items[0].rect14_applied == 1 &&
          plan.items[0].rect14_scale64 == 0x20 &&
          plan.items[0].rect14_lateral_offset == -3 &&
          plan.items[0].rect14_flip_mirror == 1 &&
          plan.items[0].rect14_row_hash == 0x11111111u &&
          plan.items[0].rect14_placement_hash == 0x22222222u &&
          plan.items[0].source_static_object_placement_valid == 0);
}

static void test_side_deep_cell_plans(void)
{
    DM2_V1_StaticObjectSourcePlan plan;
    int cell;

    /* Every table1d7029 cell 1..15 is admitted: y-distance, stretch row and
     * display order are all source-owned.  Spot-check the geometry rules per
     * row; the loop below covers admission for the whole range. */
    CHECK("D0L derives row-0 stretch and left-side no-mirror",
          dm2_v1_viewport_static_object_source_plan(
              1, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(1),
              0x14, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 1 &&
          plan.position_5x5 == 6 &&
          plan.clip_rect_id == (0x8000 | (5000 + 25 + 6)) &&
          plan.y_distance == 0 &&
          plan.stretch_factor64 == 0x60 &&
          plan.flip_mirror == 0);
    CHECK("D0R right-side chest always mirrors",
          dm2_v1_viewport_static_object_source_plan(
              2, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(2),
              0x14, 1, 0, 0, 0, 1u, 1u << 8, &plan) == 1 &&
          plan.position_5x5 == 8 &&
          plan.stretch_factor64 == 0x60 &&
          plan.flip_mirror == 1);
    CHECK("D1L left-side chest never mirrors",
          dm2_v1_viewport_static_object_source_plan(
              4, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(4),
              0x14, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 1 &&
          plan.y_distance == 1 &&
          plan.stretch_factor64 == 0x40 &&
          plan.flip_mirror == 0);
    CHECK("D1R chest mirrors from the side rule, not the anchor column",
          dm2_v1_viewport_static_object_source_plan(
              5, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(5),
              0x14, 2, 0, 0, 0, 1u, 1u << 18, &plan) == 1 &&
          plan.position_5x5 == 18 &&
          plan.stretch_factor64 == 0x34 &&
          plan.flip_mirror == 1);
    CHECK("D2R far anchor mirrors only through the side rule",
          dm2_v1_viewport_static_object_source_plan(
              8, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(8),
              0x14, 3, 0, 0, 0, 1u, 1u << 16, &plan) == 1 &&
          plan.position_5x5 == 16 &&
          plan.stretch_factor64 == 0x23 &&
          plan.flip_mirror == 1);
    CHECK("D2 far-right side cell never mirrors",
          dm2_v1_viewport_static_object_source_plan(
              10, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(10),
              0x14, 1, 0, 0, 0, 1u, 1u << 8, &plan) == 1 &&
          plan.y_distance == 2 &&
          plan.stretch_factor64 == 0x2b &&
          plan.flip_mirror == 0);
    CHECK("D3C derives row-3 stretch",
          dm2_v1_viewport_static_object_source_plan(
              11, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(11),
              0x10, 3, 0, 0, 0, 1u, 1u << 16, &plan) == 1 &&
          plan.position_5x5 == 16 &&
          plan.y_distance == 3 &&
          plan.stretch_factor64 == 0x17);
    CHECK("D3R2 far side derives row-3 stretch without mirror",
          dm2_v1_viewport_static_object_source_plan(
              15, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(15),
              0x14, 2, 0, 0, 0, 1u, 1u << 18, &plan) == 1 &&
          plan.position_5x5 == 18 &&
          plan.y_distance == 3 &&
          plan.stretch_factor64 == 0x17 &&
          plan.flip_mirror == 0);

    for (cell = 1; cell <= 15; ++cell) {
        CHECK("every side/deep cell 1..15 is admitted",
              dm2_v1_viewport_static_object_source_plan(
                  cell, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(cell),
                  0x10, 0, 0, 0, 0, 1u,
                  dm2_v1_viewport_static_object_visibility_bit(0, 0),
                  &plan) == 1 &&
              plan.source_cell == cell &&
              plan.position_5x5 == 6 &&
              plan.record_list_ordinal == 1u &&
              plan.stretch_factor64 > 0);
    }
    CHECK("party cell and D4 cells stay fail-closed",
          dm2_v1_viewport_static_object_source_plan(
              0, 0, 0x10, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 0 &&
          dm2_v1_viewport_static_object_source_plan(
              16, dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(16),
              0x10, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 0 &&
          dm2_v1_viewport_static_object_source_plan(
              22, -1, 0x10, 0, 0, 0, 0, 1u, 1u << 6, &plan) == 0);
}

static void test_delivery_plan_with_rotated_mask(void)
{
    DM2_V1_G1DirectWeaponRoot weapon = { 11, 8, 0x1401u, 1u, 0u, 0u, 0u, 0u };
    DM2_V1_G1StaticObjectMaterialReceipt material;
    DM2_V1_StaticObjectSourcePlan source;
    DM2_V1_StaticObjectM11DeliveryPlan delivery;
    uint8_t raw[4] = { 1, 2, 3, 4 };

    memset(&material, 0, sizeof(material));
    CHECK("selector builds",
          dm2_v1_g1_static_object_material_selector(&weapon, 0x1234u,
                                                    &material.selector) == 1);
    /* Party faces east (view 1); the record faces north (dir 0), so the
     * source anchor rotates to 16 and the real mask carries that bit. */
    CHECK("view-rotated plan accepts the record-owned visibility mask",
          dm2_v1_viewport_static_object_source_plan(
              3, 17, 0x10, 0, 0, 0, 1, 1u,
              dm2_v1_viewport_static_object_visibility_bit(0, 1),
              &source) == 1 &&
          source.position_5x5 == 16 &&
          (source.visibility_mask_5x5 & (1u << 16)) != 0u);
    material.raw_gfx256_bytes = raw;
    material.raw_gfx256_byte_count = sizeof(raw);
    material.raw_gfx256_hash = 11;
    material.raw_gfx256_receipt_hash = 12;
    material.local_palette_hash = 13;
    material.clip_rect_id = (uint16_t)(source.clip_rect_id & 0x7fffu);
    material.raw4_hash = 14;
    material.raw4_receipt_hash = 15;
    CHECK("M11 delivery unblocks with the real visibility mask",
          dm2_v1_viewport_build_static_object_m11_delivery_plan(
              &material, &source, 101, &delivery) == 1 &&
          delivery.valid && delivery.m11_delivery_ready &&
          delivery.visibility_mask_5x5 == (1u << 16));
    source.visibility_mask_5x5 = 1u << 6;
    CHECK("a mask that missed the record's rotated bit stays fail-closed",
          dm2_v1_viewport_build_static_object_m11_delivery_plan(
              &material, &source, 101, &delivery) == 0);
}

int main(void)
{
    test_object_5x5_pos();
    test_dir_from_5x5_pos();
    test_display_order();
    test_source_plan_view_rotation();
    test_side_deep_cell_plans();
    test_render_plan_placement_fill();
    test_asset_blit_scale_flip_slot();
    test_static_object_rect14_handoff();
    test_delivery_plan_with_rotated_mask();
    printf("DM2 V1 DRAW_ITEM source placement: %d/%d passed\n", passed, checks);
    return passed == checks ? 0 : 1;
}
