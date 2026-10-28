/* DM2 V1 creature occupancy grid and DRAW_FLYING_ITEM selection rules.
 * Source: skproject/SKWIN/SkWinCore.cpp QUERY_CREATURE_5x5_POS,
 * DRAW_STATIC_OBJECT (_4976_5aa4 occupancy grid), DRAW_FLYING_ITEM and
 * SkGlobal.cpp _4976_43f5/_4976_4415/_4976_41a9/tlbDisplayOrder*. */

#include "dm2_v1_viewport_renderer.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int checks;
static int passed;

static int fetch_test_sprite(void *user,
                             int gdat_index,
                             const uint8_t **out_pixels,
                             int *out_w,
                             int *out_h,
                             int *out_stride)
{
    static const uint8_t pixels[4] = { 1u, 0u, 2u, 3u };
    (void)user;
    if (gdat_index == 0 || !out_pixels || !out_w || !out_h || !out_stride) {
        return -1;
    }
    *out_pixels = pixels;
    *out_w = 2;
    *out_h = 2;
    *out_stride = 2;
    return 0;
}

#define CHECK(label, condition) do { \
    ++checks; \
    if (condition) ++passed; \
    else fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); \
} while (0)

static void test_creature_occupancy_5x5(void)
{
    /* QUERY_CREATURE_5x5_POS: info slot 0xff centres at 12. */
    CHECK("unallocated info slot centres the creature",
          dm2_v1_viewport_creature_occupancy_5x5(0xff, 0, 0) == 12);
    /* The anchor rotates by (party_dir - creature_dir) & 3. */
    CHECK("anchor 6 with matching directions stays",
          dm2_v1_viewport_creature_occupancy_5x5(6, 1, 1) == 6);
    CHECK("anchor 6 viewed from the east rotates west",
          dm2_v1_viewport_creature_occupancy_5x5(6, 0, 1) ==
              dm2_v1_viewport_rotate_5x5_pos(6, 1));
    CHECK("anchor 18 viewed from the south becomes 6",
          dm2_v1_viewport_creature_occupancy_5x5(18, 3, 1) == 6);
    CHECK("invalid anchor is rejected",
          dm2_v1_viewport_creature_occupancy_5x5(25, 0, 0) == -1 &&
          dm2_v1_viewport_creature_occupancy_5x5(-1, 0, 0) == -1 &&
          dm2_v1_viewport_creature_occupancy_5x5(6, -1, 0) == -1);
}

static void test_occupancy_grid_coords(void)
{
    int x = -1;
    int y = -1;

    /* SkGlobal.cpp _4976_43f5 cell bases + identity _4976_4415 split. */
    CHECK("party cell grid base is (8,4)",
          dm2_v1_viewport_occupancy_grid_coords(0, 6, &x, &y) == 1 &&
          x == 9 && y == 3);
    CHECK("D1C grid base is (8,8)",
          dm2_v1_viewport_occupancy_grid_coords(3, 6, &x, &y) == 1 &&
          x == 9 && y == 7);
    CHECK("D2C grid base is (8,12)",
          dm2_v1_viewport_occupancy_grid_coords(6, 18, &x, &y) == 1 &&
          x == 11 && y == 9);
    CHECK("D3C grid base is (8,16)",
          dm2_v1_viewport_occupancy_grid_coords(11, 0, &x, &y) == 1 &&
          x == 8 && y == 16);
    CHECK("far-left cell grid base is (0,12)",
          dm2_v1_viewport_occupancy_grid_coords(9, 24, &x, &y) == 1 &&
          x == 4 && y == 8);
    CHECK("invalid cell or position has no grid coordinate",
          dm2_v1_viewport_occupancy_grid_coords(16, 6, &x, &y) == 0 &&
          dm2_v1_viewport_occupancy_grid_coords(3, 25, &x, &y) == 0 &&
          dm2_v1_viewport_occupancy_grid_coords(3, 6, NULL, &y) == 0);
}

static void test_display_index(void)
{
    /* tlbDisplayOrderCenter: 0,4,1,3,2,5,9,6,8,7,10,14,11,13,12,15,19,16,18
     * -> 6 at index 7, 18 at index 18. */
    CHECK("center display index of anchor 6 is 7",
          dm2_v1_viewport_static_object_display_index(3, 6) == 7);
    CHECK("center display index of anchor 18 is 18",
          dm2_v1_viewport_static_object_display_index(3, 18) == 18);
    /* tlbDisplayOrderRight: 4,3,2,1,0,9,8 -> 8 at index 6. */
    CHECK("right display index of anchor 8 is 6",
          dm2_v1_viewport_static_object_display_index(2, 8) == 6);
    /* Cell 0 uses the center order bounded to 15 entries: 14 at index 11. */
    CHECK("party cell iterates only the first 15 center entries",
          dm2_v1_viewport_static_object_display_index(0, 14) == 11 &&
          dm2_v1_viewport_static_object_display_index(0, 15) == -1);
    CHECK("unknown cell has no display index",
          dm2_v1_viewport_static_object_display_index(-1, 6) == -1);
}

static void test_flying_item_scale(void)
{
    /* _4976_41a9[(y << 1) - (dir >> 1)] = {0x40,0x34,0x2B,0x23,0x1C,0x17,0x13}. */
    CHECK("party row full scale",
          dm2_v1_viewport_flying_item_scale64(0, 0) == 0x40);
    CHECK("D1 scale rows",
          dm2_v1_viewport_flying_item_scale64(1, 0) == 0x2b &&
          dm2_v1_viewport_flying_item_scale64(1, 2) == 0x34 &&
          dm2_v1_viewport_flying_item_scale64(1, 3) == 0x34);
    CHECK("D2/D3 scale rows",
          dm2_v1_viewport_flying_item_scale64(2, 1) == 0x1c &&
          dm2_v1_viewport_flying_item_scale64(3, 0) == 0x13);
    CHECK("negative band blocks the source draw",
          dm2_v1_viewport_flying_item_scale64(0, 2) == -1 &&
          dm2_v1_viewport_flying_item_scale64(4, 0) == -1);
}

static void test_flying_item_field(void)
{
    int flip = -1;

    CHECK("frame class 3 always selects field 8",
          dm2_v1_viewport_flying_item_image_field(3, 0, 0, 0, 0, 0, 1, 0,
                                                  &flip) == 8 && flip == 0);
    /* Timer parity differs from the view: side-on 0x0c. */
    CHECK("parity mismatch selects side-on field 12",
          dm2_v1_viewport_flying_item_image_field(0, 1, 0, 5, 6, 0, 2, 0,
                                                  &flip) == 0xc);
    CHECK("class 0 di 0 mirrors and even tile parity flips it",
          dm2_v1_viewport_flying_item_image_field(0, 1, 0, 4, 6, 0, 0, 0,
                                                  &flip) == 0xc && flip == 0 &&
          dm2_v1_viewport_flying_item_image_field(0, 1, 0, 5, 6, 0, 0, 0,
                                                  &flip) == 0xc && flip == 1);
    /* Same parity, directional class: odd tile -> si|2, di<2 -> 8 else 9. */
    CHECK("odd tile parity selects fields 8/9 with mirror bit 2",
          dm2_v1_viewport_flying_item_image_field(0, 0, 0, 3, 4, 0, 1, 0,
                                                  &flip) == 8 && flip == 2 &&
          dm2_v1_viewport_flying_item_image_field(0, 0, 0, 3, 4, 0, 2, 0,
                                                  &flip) == 9 && flip == 2);
    CHECK("even tile parity keeps front fields 8/9",
          dm2_v1_viewport_flying_item_image_field(0, 0, 0, 2, 4, 0, 1, 0,
                                                  &flip) == 9 && flip == 0 &&
          dm2_v1_viewport_flying_item_image_field(0, 0, 0, 2, 4, 0, 2, 0,
                                                  &flip) == 8 && flip == 0);
    CHECK("class 2 and off-view class 1 select field 8, else 10",
          dm2_v1_viewport_flying_item_image_field(2, 0, 0, 2, 4, 0, 1, 0,
                                                  &flip) == 8 &&
          dm2_v1_viewport_flying_item_image_field(1, 2, 0, 2, 4, 0, 1, 0,
                                                  &flip) == 8 &&
          dm2_v1_viewport_flying_item_image_field(1, 0, 0, 2, 4, 0, 1, 0,
                                                  &flip) == 10);
    CHECK("left cell mirrors, centre cell with di 1/2 does not",
          dm2_v1_viewport_flying_item_image_field(1, 0, 0, 2, 4, -1, 1, 0,
                                                  &flip) == 10 && flip == 1 &&
          dm2_v1_viewport_flying_item_image_field(1, 0, 0, 2, 4, 0, 1, 0,
                                                  &flip) == 10 && flip == 0);
    CHECK("spell class with odd direction adds mirror bit 2",
          dm2_v1_viewport_flying_item_image_field(1, 0, 0, 2, 4, 0, 1, 1,
                                                  &flip) == 10 && flip == 2);
    CHECK("negative direction has no source field",
          dm2_v1_viewport_flying_item_image_field(0, 0, 0, 0, 0, 0, -1, 0,
                                                  &flip) == -1);
}

static void test_v5_render_route(void)
{
    DM2_V1_ViewportState s;
    DM2_V1_CreatureRenderPlan plan;
    DM2_V1_G1CreatureV5RuntimeReceipt receipt;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&s, 0, 10, 10);
    s.creature_count = 1;
    memset(s.creatures, 0, sizeof(s.creatures));
    s.creatures[0].creature_type = 7;
    s.creatures[0].screen_x = 96;
    s.creatures[0].screen_y = 88;
    s.creatures[0].direction = 1;
    s.creatures[0].source_kind = 2;
    s.creatures[0].source_v5_field = 1;
    s.creatures[0].source_material_proven = 1;
    s.creatures[0].gdat_image_field = 0x12;
    s.creatures[0].source_animation_0958_valid = 1;
    s.creatures[0].source_animation_0958_frame_bit14 = 1;
    s.creatures[0].source_animation_0958_query_index = 3;
    s.creatures[0].source_animation_0958_blended_value = 0x12345678u;
    s.creatures[0].object_id = 0x4401u;
    s.creatures[0].map_x = 10;
    s.creatures[0].map_y = 8;

    CHECK("V5 field row selects the direct dtImage route",
          dm2_v1_viewport_build_creature_render_plan(&s, &plan) == 1 &&
          plan.creature_count == 1 &&
          plan.creatures[0].source_v5_field == 1 &&
          plan.creatures[0].source_animation_0958_valid == 1 &&
          plan.creatures[0].source_animation_0958_frame_bit14 == 1 &&
          plan.creatures[0].source_animation_0958_query_index == 3 &&
          plan.creatures[0].source_animation_0958_blended_value ==
              0x12345678u &&
          plan.creatures[0].material_frame_index == 0 &&
          plan.creatures[0].gdat_index ==
              dm2_v1_viewport_creature_field_graphic_index(7, 0x12));

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.map = 7;
    receipt.count = 1;
    receipt.materials[0].object_id = 0x4401u;
    receipt.materials[0].map_x = 10;
    receipt.materials[0].map_y = 8;
    receipt.materials[0].creature_type = 7;
    receipt.materials[0].image_field = 0x12;
    receipt.materials[0].gdat_index = plan.creatures[0].gdat_index;
    receipt.materials[0].width = 105;
    receipt.materials[0].height = 87;
    receipt.materials[0].palette_hash = 0xabcu;
    receipt.materials[0].decoded_hash = 0xdefu;
    CHECK("V5 material evidence matches its decoded identity",
          dm2_v1_g1_creature_v5_material_matches(
              &receipt, 0x4401u, 10, 8, 7, 0x12, 105, 87,
              0xabcu, 0xdefu) == 1);
    CHECK("altered evidence stays fail-closed",
          dm2_v1_g1_creature_v5_material_matches(
              &receipt, 0x4401u, 10, 8, 7, 0x12, 105, 87,
              0xabcu, 0xde0u) == 0 &&
          dm2_v1_g1_creature_v5_material_matches(
              &receipt, 0x4401u, 10, 8, 7, 0x13, 105, 87,
              0xabcu, 0xdefu) == 0 &&
          dm2_v1_g1_creature_v5_material_matches(
              &receipt, 0x4402u, 10, 8, 7, 0x12, 105, 87,
              0xabcu, 0xdefu) == 0);
}

static void test_occupancy_ordering(void)
{
    static const uint8_t rect14_rows[2][14] = {
        { 18, 0, 0x10, 0x10, 0x10, 0x10, 64, 64, 64, 64, 0, 0, 0, 0 },
        { 6, 0, 0x11, 0x11, 0x11, 0x11, 64, 64, 64, 64, 0, 0, 0, 0 }
    };
    DM2_V1_ViewportState s;
    DM2_V1_CreatureRenderPlan plan;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&s, 0, 10, 10);
    dm2_v1_viewport_set_gdat_interface_rect14(&s, &rect14_rows[0][0], 2, 1u);
    s.creature_count = 2;
    memset(s.creatures, 0, sizeof(s.creatures));
    /* Both creatures at D1C (map (10,8)); the first anchors at 18
     * (display index 18), the second at 6 (display index 7). */
    s.creatures[0].creature_type = 7;
    s.creatures[0].frame_index = 0;
    s.creatures[0].screen_x = 96;
    s.creatures[0].screen_y = 88;
    s.creatures[0].direction = 0;
    s.creatures[0].source_kind = 2;
    s.creatures[0].object_id = 0x4401u;
    s.creatures[0].source_v5_field = 1;
    s.creatures[0].source_material_proven = 1;
    s.creatures[0].map_x = 10;
    s.creatures[0].map_y = 8;
    s.creatures[1] = s.creatures[0];
    s.creatures[1].frame_index = 1;
    s.creatures[1].object_id = 0x4402u;

    dm2_v1_viewport_build_creature_render_plan(&s, &plan);
    CHECK("occupancy proven rows sort by the source display order",
          plan.creature_count == 2 &&
          plan.creatures[0].occupancy_5x5 == 6 &&
          plan.creatures[0].occupancy_display_index == 7 &&
          plan.creatures[0].source_pass == 14 &&
          plan.creatures[1].occupancy_5x5 == 18 &&
          plan.creatures[1].occupancy_display_index == 18 &&
          plan.creatures[0].object_id == 0x4402u &&
          plan.creatures[1].object_id == 0x4401u);

    /* Without rect14 evidence the order stays as appended. */
    dm2_v1_viewport_set_gdat_interface_rect14(&s, NULL, 0, 0u);
    CHECK("unproven occupancy keeps the appended order",
          dm2_v1_viewport_build_creature_render_plan(&s, &plan) == 1 &&
          plan.creature_count == 2 &&
          plan.creatures[0].occupancy_5x5 == -1 &&
          plan.creatures[0].object_id == 0x4401u &&
          plan.creatures[1].object_id == 0x4402u);
}

static void setup_admitted_item(DM2_V1_ViewportState *s,
                                int item_index,
                                int cell,
                                int category,
                                int direction,
                                int draw_slot,
                                int record_ordinal,
                                int rect14)
{
    DM2_ItemSprite *item = &s->items[item_index];
    DM2_V1_StaticObjectSourcePlan source_plan;
    int pass = dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(cell);

    memset(item, 0, sizeof(*item));
    item->item_category = (uint8_t)category;
    item->item_type = (uint8_t)(0x20 + item_index);
    item->screen_x = (int16_t)(80 + item_index * 8);
    item->screen_y = 88;
    item->direction = (uint8_t)direction;
    item->source_g1_weapon = category == 0x10;
    item->source_g1_container = category == 0x14;
    item->source_gdat_field = 0;
    item->source_static_object_admitted = 1;
    item->source_static_object_cell = (uint8_t)cell;
    item->source_static_object_pass = (int8_t)pass;
    item->source_static_object_draw_slot = (uint8_t)draw_slot;
    item->source_static_object_record_ordinal = (uint16_t)record_ordinal;
    if (dm2_v1_viewport_static_object_source_plan(
            cell, pass, category, direction, 0, draw_slot, s->party_dir,
            (uint16_t)record_ordinal,
            dm2_v1_viewport_static_object_visibility_bit(direction,
                                                         s->party_dir),
            &source_plan)) {
        item->source_static_object_clip_rect_id =
            (uint16_t)(source_plan.clip_rect_id & 0x7fffu);
    }
    if (rect14) {
        item->source_static_object_rect14_applied = 1;
        item->source_static_object_rect14_scale64 = 0x20;
        item->source_static_object_rect14_row_hash = 0x11111111u;
        item->source_static_object_rect14_placement_hash = 0x22222222u;
    }
}

static void test_indoor_item_creature_draw_order(void)
{
    static const uint8_t rect14_rows[1][14] = {
        { 6, 0, 0x11, 0x11, 0x11, 0x11, 64, 64, 64, 64, 0, 0, 0, 0 }
    };
    DM2_V1_ViewportState s;
    DM2_V1_ItemRenderPlan items;
    DM2_V1_CreatureRenderPlan creatures;
    DM2_V1_IndoorDrawOrderReceipt receipt;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    dm2_v1_viewport_set_party(&s, 0, 10, 10);
    dm2_v1_viewport_set_gdat_interface_rect14(&s, &rect14_rows[0][0], 1,
                                               0x1234u);
    s.item_count = 6;
    /* This is Rect14-bound. Scheduler must still derive the static source
     * slot instead of relying on source_static_object_placement_valid. */
    setup_admitted_item(&s, 0, 6, 0x10, 0, 0, 7, 1);
    setup_admitted_item(&s, 1, 3, 0x10, 0, 2, 9, 0);
    setup_admitted_item(&s, 2, 3, 0x10, 0, 2, 4, 0);
    memset(&s.items[3], 0, sizeof(s.items[3]));
    s.items[3].item_category = 0x10;
    s.items[3].item_type = 0x30;
    s.items[3].screen_x = 120;
    s.items[3].screen_y = 88;
    memset(&s.items[4], 0, sizeof(s.items[4]));
    s.items[4].item_category = 0x10;
    s.items[4].item_type = 0x31;
    s.items[4].screen_x = 128;
    s.items[4].screen_y = 88;
    /* The low-level geometry helper accepts ordinal 0, but the authenticated
     * static-object delivery gate defines it as unavailable. Keep it in the
     * fallback lane rather than treating it as a source ordering key. */
    setup_admitted_item(&s, 5, 3, 0x10, 0, 1, 0, 0);

    s.creature_count = 2;
    memset(s.creatures, 0, sizeof(s.creatures));
    s.creatures[0].creature_type = 7;
    s.creatures[0].frame_index = 0;
    s.creatures[0].screen_x = 96;
    s.creatures[0].screen_y = 88;
    s.creatures[0].direction = 0;
    s.creatures[0].source_kind = 2;
    s.creatures[0].source_v5_field = 1;
    s.creatures[0].source_material_proven = 1;
    s.creatures[0].map_x = 10;
    s.creatures[0].map_y = 9;
    s.creatures[1] = s.creatures[0];
    s.creatures[1].frame_index = 1;
    s.creatures[1].map_x = 0;
    s.creatures[1].map_y = 0;

    CHECK("mixed-order plans build from the admitted source rows",
          dm2_v1_viewport_build_item_render_plan(&s, &items) == 1 &&
          dm2_v1_viewport_build_creature_render_plan(&s, &creatures) == 1 &&
          items.item_count == 6 && creatures.creature_count == 2);
    CHECK("Rect14 row does not erase the independently derived item slot",
          items.items[0].rect14_applied == 1 &&
          items.items[0].source_static_object_placement_valid == 0);

    CHECK("source scheduler returns a partial receipt for unknown rows",
          dm2_v1_viewport_build_indoor_draw_order(
              &s, &items, &creatures, &receipt) == 1 &&
          receipt.valid == 1 && receipt.partial == 1 &&
          receipt.event_count == 8 && receipt.source_ordered_count == 4 &&
          receipt.fallback_item_count == 3 &&
          receipt.fallback_creature_count == 1);
    CHECK("pass, display slot, item draw slot/ordinal and shared-slot order",
          receipt.events[0].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[0].source_pass == 14 &&
          receipt.events[0].display_index == 7 &&
          receipt.events[1].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[1].source_pass == 17 &&
          receipt.events[1].source_record_ordinal == 4 &&
          receipt.events[2].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[2].source_record_ordinal == 9 &&
          receipt.events[3].kind == DM2_V1_INDOOR_DRAW_ORDER_CREATURE &&
          receipt.events[3].source_pass == 17 &&
          receipt.events[3].display_index == 7);
    CHECK("unknown rows keep stable item-then-creature fallback lanes",
          receipt.events[4].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[4].plan_index == 3 &&
          receipt.events[4].source_key_proven == 0 &&
          receipt.events[4].source_pass == -1 &&
          receipt.events[5].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[5].plan_index == 4 &&
          receipt.events[5].source_key_proven == 0 &&
          receipt.events[6].kind == DM2_V1_INDOOR_DRAW_ORDER_ITEM &&
          receipt.events[6].plan_index == 5 &&
          receipt.events[6].source_key_proven == 0 &&
          receipt.events[6].source_pass == -1 &&
          receipt.events[7].kind == DM2_V1_INDOOR_DRAW_ORDER_CREATURE &&
          receipt.events[7].plan_index == 1 &&
          receipt.events[7].source_key_proven == 0);
}

static void test_indoor_render_receipts_accumulate(void)
{
    DM2_V1_ViewportState s;
    uint8_t fb[DM2_VP_WIDTH * DM2_VP_HEIGHT];

    memset(fb, 0, sizeof(fb));
    dm2_v1_viewport_init(&s, fb, DM2_VP_WIDTH);
    s.item_count = 1;
    s.items[0].item_category = 0x10;
    s.items[0].item_type = 0x22;
    s.items[0].screen_x = 96;
    s.items[0].screen_y = 88;
    s.creature_count = 1;
    s.creatures[0].creature_type = 7;
    s.creatures[0].source_kind = 1;
    s.creatures[0].source_material_proven = 1;
    s.creatures[0].gdat_image_field = 0x12;
    s.creatures[0].screen_x = 112;
    s.creatures[0].screen_y = 88;
    dm2_v1_viewport_set_asset_provider(&s, fetch_test_sprite, NULL);

    dm2_v1_render_indoor_items_and_creatures(&s);
    CHECK("item and creature material receipts survive one merged render",
          s.asset_item_drawn_count == 1 &&
          s.asset_creature_drawn_count == 1 &&
          s.item_material_drawn_count == 1 &&
          s.creature_material_drawn_count == 1 &&
          s.last_item_render_valid == 1 &&
          s.last_creature_render_valid == 1);
    CHECK("combined receipt exposes the partial fallback ordering",
          s.last_indoor_draw_order_receipt.valid == 1 &&
          s.last_indoor_draw_order_receipt.partial == 1 &&
          s.last_indoor_draw_order_receipt.event_count == 2 &&
          s.last_frame_composition.indoor_draw_order_valid == 1 &&
          s.last_frame_composition.indoor_draw_order_partial == 1 &&
          s.last_frame_composition.indoor_draw_order_fallback_item_count == 1 &&
          s.last_frame_composition.indoor_draw_order_fallback_creature_count == 1);
}

static void test_live_missile_draw_order_and_creature_replay(void)
{
    DM2_V1_IndoorLiveMissilePlan missiles;
    DM2_V1_IndoorLiveMissileDrawPlan draws;
    uint8_t creature_draw_success[
        DM2_V1_INDOOR_STATIC_OBJECT_PASS_COUNT]
        [DM2_V1_INDOOR_STATIC_OBJECT_SLOT_COUNT];

    memset(&missiles, 0, sizeof(missiles));
    memset(creature_draw_success, 0, sizeof(creature_draw_success));
    missiles.valid = 1;
    missiles.event_count = 4;
    /* Cell 3 uses the center display order. Missiles occupy iterations 1, 2,
     * 7, and 9. The successful creature at 9 must replay 2 and 7, excluding
     * the slot at 1 because it lies outside the seven-slot lookback. */
    missiles.events[0].source_pass = 17;
    missiles.events[0].source_cell = 3;
    missiles.events[0].display_index = 1;
    missiles.events[0].live.source.view_position_5x5 = 4;
    missiles.events[0].live.source.chain_ordinal = 1;
    missiles.events[1].source_pass = 17;
    missiles.events[1].source_cell = 3;
    missiles.events[1].display_index = 2;
    missiles.events[1].live.source.view_position_5x5 = 1;
    missiles.events[1].live.source.chain_ordinal = 2;
    missiles.events[2].source_pass = 17;
    missiles.events[2].source_cell = 3;
    missiles.events[2].display_index = 7;
    missiles.events[2].live.source.view_position_5x5 = 6;
    missiles.events[2].live.source.chain_ordinal = 3;
    missiles.events[3].source_pass = 17;
    missiles.events[3].source_cell = 3;
    missiles.events[3].display_index = 9;
    missiles.events[3].live.source.view_position_5x5 = 7;
    missiles.events[3].live.source.chain_ordinal = 4;
    creature_draw_success[17][9] = 1;
    CHECK("live DB14 draw schedule records seven-slot replay after creature success",
          dm2_v1_viewport_build_indoor_live_missile_draw_plan(
              &missiles, creature_draw_success, &draws) == 1 &&
          draws.valid == 1 && draws.event_count == 6 &&
          draws.events[0].missile_event_index == 0 &&
          draws.events[0].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[0].display_index == 1 &&
          draws.events[1].missile_event_index == 1 &&
          draws.events[1].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[1].display_index == 2 &&
          draws.events[2].missile_event_index == 2 &&
          draws.events[2].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[2].display_index == 7 &&
          draws.events[3].missile_event_index == 1 &&
          draws.events[3].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CREATURE_REPLAY &&
          draws.events[3].display_index == 9 &&
          draws.events[3].replay_slot_index == 2 &&
          draws.events[4].missile_event_index == 2 &&
          draws.events[4].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CREATURE_REPLAY &&
          draws.events[4].display_index == 9 &&
          draws.events[4].replay_slot_index == 7 &&
          draws.events[5].missile_event_index == 3 &&
          draws.events[5].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[5].display_index == 9 &&
          draws.events[5].replay_slot_index == 9);

    memset(creature_draw_success, 0, sizeof(creature_draw_success));
    CHECK("failed or absent creature draw does not replay prior missiles",
          dm2_v1_viewport_build_indoor_live_missile_draw_plan(
              &missiles, creature_draw_success, &draws) == 1 &&
          draws.valid == 1 && draws.event_count == 4 &&
          draws.events[0].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[1].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[2].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[3].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT);

    {
        DM2_V1_IndoorLiveMissileEvent swap = missiles.events[0];
        missiles.events[0] = missiles.events[1];
        missiles.events[1] = swap;
    }
    CHECK("unsorted live chains are rejected instead of inventing source order",
          dm2_v1_viewport_build_indoor_live_missile_draw_plan(
              &missiles, creature_draw_success, &draws) == 0 &&
          draws.valid == 0 && draws.event_count == 0);

    memset(&missiles, 0, sizeof(missiles));
    memset(creature_draw_success, 0, sizeof(creature_draw_success));
    missiles.valid = 1;
    missiles.event_count = 1;
    /* The player-tile DM2_DRAW_STATIC_OBJECT pass stops after source order
     * slot 14, even though the center order table has 25 entries. Positions
     * 6 resolves to slot 7; position 16 is outside the source iteration. */
    missiles.events[0].source_pass = 20;
    missiles.events[0].source_cell = 0;
    missiles.events[0].display_index =
        dm2_v1_viewport_static_object_display_index(0, 6);
    missiles.events[0].live.source.view_position_5x5 = 6;
    missiles.events[0].live.source.chain_ordinal = 1;
    creature_draw_success[20][14] = 1;
    CHECK("player-tile pass follows its 15 source iterations and replay window",
          missiles.events[0].display_index == 7 &&
          dm2_v1_viewport_static_object_display_index(0, 16) == -1 &&
          dm2_v1_viewport_build_indoor_live_missile_draw_plan(
              &missiles, creature_draw_success, &draws) == 1 &&
          draws.valid == 1 && draws.event_count == 2 &&
          draws.events[0].missile_event_index == 0 &&
          draws.events[0].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CURRENT &&
          draws.events[0].display_index == 7 &&
          draws.events[1].missile_event_index == 0 &&
          draws.events[1].draw_kind ==
              DM2_V1_INDOOR_LIVE_MISSILE_DRAW_CREATURE_REPLAY &&
          draws.events[1].display_index == 14 &&
          draws.events[1].replay_slot_index == 7);
}

int main(void)
{
    test_creature_occupancy_5x5();
    test_occupancy_grid_coords();
    test_display_index();
    test_flying_item_scale();
    test_flying_item_field();
    test_v5_render_route();
    test_occupancy_ordering();
    test_indoor_item_creature_draw_order();
    test_indoor_render_receipts_accumulate();
    test_live_missile_draw_order_and_creature_replay();
    printf("DM2 V1 creature occupancy and flying item: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
