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
    s.creatures[0].object_id = 0x4401u;
    s.creatures[0].map_x = 10;
    s.creatures[0].map_y = 8;

    CHECK("V5 field row selects the direct dtImage route",
          dm2_v1_viewport_build_creature_render_plan(&s, &plan) == 1 &&
          plan.creature_count == 1 &&
          plan.creatures[0].source_v5_field == 1 &&
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
    s.creatures[0].map_x = 10;
    s.creatures[0].map_y = 8;
    s.creatures[1] = s.creatures[0];
    s.creatures[1].frame_index = 1;
    s.creatures[1].object_id = 0x4402u;

    CHECK("occupancy proven rows sort by the source display order",
          dm2_v1_viewport_build_creature_render_plan(&s, &plan) == 1 &&
          plan.creature_count == 2 &&
          plan.creatures[0].occupancy_5x5 == 6 &&
          plan.creatures[0].occupancy_display_index == 7 &&
          plan.creatures[0].source_pass == 17 &&
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

int main(void)
{
    test_creature_occupancy_5x5();
    test_occupancy_grid_coords();
    test_display_index();
    test_flying_item_scale();
    test_flying_item_field();
    test_v5_render_route();
    test_occupancy_ordering();
    printf("DM2 V1 creature occupancy and flying item: %d/%d passed\n",
           passed, checks);
    return passed == checks ? 0 : 1;
}
