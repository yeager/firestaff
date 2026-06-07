#include "csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions = 0;

static int expect_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char *label, const char *haystack,
                           const char *needle, const char *anchor)
{
    const int got = haystack && needle && strstr(haystack, needle) != NULL;
    return expect_int(label, got, 1, anchor);
}

static int test_d2_route_metadata(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(10);
    /* ReDMCSB: DEFS.H:2605-2606 names C09/C10; DUNVIEW.C:371-377 maps
     * them to depth 2, lanes -2/+2, missing F0115 rows, and fields 5/6. */
    ok &= expect_int("route.count",
                     (int)csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_count_pc34(), 2,
                     "ReDMCSB DUNVIEW.C:371-377; DEFS.H:2605-2606");
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.view_square", d2l2 ? d2l2->view_square : -1, 9,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.view_square", d2r2 ? d2r2->view_square : -1, 10,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:372 G2027[9]");
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->f0128_relative_depth : -1, 2,
                     "ReDMCSB DUNVIEW.C:372 G2027[10]");
    ok &= expect_int("d2l2.lateral", d2l2 ? d2l2->f0128_relative_lateral : 0, -2,
                     "ReDMCSB DUNVIEW.C:8503 F0128 relative -2");
    ok &= expect_int("d2r2.lateral", d2r2 ? d2r2->f0128_relative_lateral : 0, 2,
                     "ReDMCSB DUNVIEW.C:8507 F0128 relative +2");
    ok &= expect_int("d2l2.lane", d2l2 ? d2l2->view_lane : 0, 254,
                     "ReDMCSB DUNVIEW.C:371 G2026[9] stores -2 as 254");
    ok &= expect_int("d2r2.lane", d2r2 ? d2r2->view_lane : 0, 2,
                     "ReDMCSB DUNVIEW.C:371 G2026[10]");
    ok &= expect_int("d2l2.object_row", d2l2 ? d2l2->object_g2028_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:373/4811 G2028[9]");
    ok &= expect_int("d2r2.object_row", d2r2 ? d2r2->object_g2028_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:373/4811 G2028[10]");
    ok &= expect_int("d2l2.explosion_row", d2l2 ? d2l2->explosion_g2034_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:376/5920 G2034[9]");
    ok &= expect_int("d2r2.explosion_row", d2r2 ? d2r2->explosion_g2034_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:376/5920 G2034[10]");
    ok &= expect_int("d2l2.field_aspect", d2l2 ? d2l2->field_aspect_index : -1, 5,
                     "ReDMCSB DUNVIEW.C:377 G2035[9]");
    ok &= expect_int("d2r2.field_aspect", d2r2 ? d2r2->field_aspect_index : -1, 6,
                     "ReDMCSB DUNVIEW.C:377 G2035[10]");
    ok &= expect_int("d2l2.field_zone", d2l2 ? d2l2->wall_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6863-6865; DEFS.H:4047");
    ok &= expect_int("d2r2.field_zone", d2r2 ? d2r2->wall_zone : -1, 708,
                     "ReDMCSB DUNVIEW.C:6894-6896; DEFS.H:4048");
    ok &= expect_int("unknown.square",
                     csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(14) == NULL, 1,
                     "ReDMCSB DEFS.H:2605-2606 D2L2/D2R2-only gate");

    return ok;
}

static int test_item_zone_contract(void)
{
    int ok = 1;
    CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec synthetic;
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(0);
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(1);

    if (d2l2) synthetic = *d2l2;

    /* ReDMCSB: DUNVIEW.C:4923 gates item things, lines 5071-5079 bind
     * C2500 | MASK0x8000 + G2028*4 + ViewCell and update pile shifts. */
    ok &= expect_int("item.zone_base", d2l2 ? d2l2->object_zone_base : -1, 2500,
                     "ReDMCSB DEFS.H:4228 C2500_ZONE_; DUNVIEW.C:5075");
    ok &= expect_int("item.zone_stride", d2l2 ? d2l2->object_zone_cell_stride : -1, 4,
                     "ReDMCSB DUNVIEW.C:5075 row*4+ViewCell");
    ok &= expect_int("item.shift_mask", d2l2 ? d2l2->object_shift_mask : -1, 0x8000,
                     "ReDMCSB DEFS.H:3517; DUNVIEW.C:5075");
    ok &= expect_int("item.type_range", d2l2 ? d2l2->object_requires_type_weapon_to_junk : -1, 1,
                     "ReDMCSB DUNVIEW.C:4923 C05..C10 item range");
    ok &= expect_int("item.cell_match", d2l2 ? d2l2->object_requires_cell_match : -1, 1,
                     "ReDMCSB DUNVIEW.C:4923 M011_CELL == L0139");
    ok &= expect_int("item.rejects_missing_row", d2l2 ? d2l2->object_rejects_missing_row : -1, 1,
                     "ReDMCSB DUNVIEW.C:4923 L2476_i_ >= 0");
    ok &= expect_int("item.d2l2.zone_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2l2, 2), -1,
                     "ReDMCSB DUNVIEW.C:373/4923 rejects G2028[9] < 0");
    ok &= expect_int("item.d2r2.zone_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2r2, 3), -1,
                     "ReDMCSB DUNVIEW.C:373/4923 rejects G2028[10] < 0");
    ok &= expect_int("item.bad_cell",
                     csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(d2l2, 4), -1,
                     "ReDMCSB DUNVIEW.C:5075 uses four view cells");
    ok &= expect_int("item.null_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(NULL, 0), -1,
                     "ReDMCSB DUNVIEW.C:4923 requires resolved view metadata");
    ok &= expect_int("item.pile_shift_advances",
                     d2l2 ? d2l2->object_pile_shift_advances : -1, 1,
                     "ReDMCSB DUNVIEW.C:5077-5089 pile-shift advancement");
    ok &= expect_int("item.f0791", d2l2 ? d2l2->object_uses_f0791_blit : -1, 1,
                     "ReDMCSB DUNVIEW.C:5109 F0791 item blit");

    if (d2l2) synthetic.object_g2028_row = 8;
    ok &= expect_int("item.synthetic_layout_zone",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_item_layout_zone_pc34(&synthetic, 3) : -1,
                     2535,
                     "ReDMCSB DUNVIEW.C:5075 C2500 + G2028*4 + ViewCell");
    ok &= expect_int("item.synthetic_shifted_zone",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_item_zone_pc34(&synthetic, 3) : -1,
                     (0x8000 | 2535),
                     "ReDMCSB DEFS.H:3517; DUNVIEW.C:5075 C2500|MASK0x8000");

    return ok;
}

static int test_explosion_zone_contract(void)
{
    int ok = 1;
    CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec synthetic;
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(0);
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_at_pc34(1);

    if (d2l2) synthetic = *d2l2;

    /* ReDMCSB: DUNVIEW.C:5915-5933 restarts explosions, 5920-5924 maps
     * G2034/G2035, and 5998-6122 selects the C3000/C3007/C3014/C3031 rows. */
    ok &= expect_int("explosion.restart",
                     d2l2 ? d2l2->explosion_restarts_thing_list_after_cells : -1, 1,
                     "ReDMCSB DUNVIEW.C:5915-5933");
    ok &= expect_int("explosion.rebirth_missing_row_reject",
                     d2l2 ? d2l2->explosion_rejects_missing_row_for_rebirth : -1, 1,
                     "ReDMCSB DUNVIEW.C:5948 L2476_i_ < 0");
    ok &= expect_int("explosion.step1_base",
                     d2l2 ? d2l2->explosion_rebirth_step1_zone_base : -1, 3000,
                     "ReDMCSB DEFS.H:4232; DUNVIEW.C:5998-5999");
    ok &= expect_int("explosion.step2_base",
                     d2l2 ? d2l2->explosion_rebirth_step2_zone_base : -1, 3007,
                     "ReDMCSB DEFS.H:4233; DUNVIEW.C:6094-6096");
    ok &= expect_int("explosion.center_base",
                     d2l2 ? d2l2->explosion_centered_zone_base : -1, 3014,
                     "ReDMCSB DEFS.H:4234; DUNVIEW.C:6106-6107");
    ok &= expect_int("explosion.side_base",
                     d2l2 ? d2l2->explosion_side_zone_base : -1, 3031,
                     "ReDMCSB DEFS.H:4235; DUNVIEW.C:6121-6122");
    ok &= expect_int("explosion.side_stride",
                     d2l2 ? d2l2->explosion_side_zone_cell_stride : -1, 2,
                     "ReDMCSB DUNVIEW.C:6121-6122 row*2+ViewCell");
    ok &= expect_int("explosion.d2l2.step1_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step1_zone_pc34(d2l2), -1,
                     "ReDMCSB DUNVIEW.C:376/5948 rejects G2034[9] < 0");
    ok &= expect_int("explosion.d2r2.step2_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step2_zone_pc34(d2r2), -1,
                     "ReDMCSB DUNVIEW.C:376/6094-6096 rejects G2034[10] < 0");
    ok &= expect_int("explosion.center_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_explosion_centered_zone_pc34(d2l2), -1,
                     "ReDMCSB DUNVIEW.C:376/6106-6107 rejects G2034[9] < 0");
    ok &= expect_int("explosion.side_rejected",
                     csb_v1_viewport_d2l2_d2r2_f0115_explosion_side_zone_pc34(d2r2, 1), -1,
                     "ReDMCSB DUNVIEW.C:376/6121-6122 rejects G2034[10] < 0");
    ok &= expect_int("explosion.bad_cell",
                     csb_v1_viewport_d2l2_d2r2_f0115_explosion_side_zone_pc34(d2l2, 2), -1,
                     "ReDMCSB DUNVIEW.C:6121-6122 has side cells 0/1");
    ok &= expect_int("explosion.f0791",
                     d2l2 ? d2l2->explosion_uses_f0791_blit : -1, 1,
                     "ReDMCSB DUNVIEW.C:6192-6193 F0791 explosion blit");

    if (d2l2) synthetic.explosion_g2034_row = 5;
    ok &= expect_int("explosion.synthetic_step1",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step1_zone_pc34(&synthetic) : -1,
                     3005,
                     "ReDMCSB DUNVIEW.C:5998-5999 C3000 + row");
    ok &= expect_int("explosion.synthetic_step2",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_explosion_rebirth_step2_zone_pc34(&synthetic) : -1,
                     3012,
                     "ReDMCSB DUNVIEW.C:6094-6096 C3007 + row");
    ok &= expect_int("explosion.synthetic_center",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_explosion_centered_zone_pc34(&synthetic) : -1,
                     3019,
                     "ReDMCSB DUNVIEW.C:6106-6107 C3014 + row");
    ok &= expect_int("explosion.synthetic_side",
                     d2l2 ? csb_v1_viewport_d2l2_d2r2_f0115_explosion_side_zone_pc34(&synthetic, 1) : -1,
                     3042,
                     "ReDMCSB DUNVIEW.C:6121-6122 C3031 + row*2 + ViewCell");

    return ok;
}

static int test_f0111_f0107_fluxcage_dispatch(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(10);
    const CSB_V1_ViewportObjectBlitSpec *shared_d2 =
        csb_v1_viewport_get_object_blit_spec_for_square(9);
    const CSB_V1_ViewportExplosionBlitSpec *shared_exp_d2 =
        csb_v1_viewport_get_explosion_blit_spec_for_square(9);

    /* ReDMCSB: DUNVIEW.C:6837-6896 F0678/F0679 have wall/teleporter cases
     * only, so D2L2/D2R2 skip F0115 and F0111 door-front item/explosion work. */
    ok &= expect_int("d2l2.no_f0115_dispatch",
                     d2l2 ? d2l2->f0678_f0679_has_f0115_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6872 no F0115 call");
    ok &= expect_int("d2r2.no_f0115_dispatch",
                     d2r2 ? d2r2->f0678_f0679_has_f0115_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6868-6896 no F0115 call");
    ok &= expect_int("d2l2.no_f0111_dispatch",
                     d2l2 ? d2l2->f0678_f0679_has_f0111_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6837-6872 no F0111 call");
    ok &= expect_int("d2r2.no_f0111_dispatch",
                     d2r2 ? d2r2->f0678_f0679_has_f0111_route : -1, 0,
                     "ReDMCSB DUNVIEW.C:6868-6896 no F0111 call");
    ok &= expect_int("door_front_skip_items_d2l2",
                     d2l2 ? d2l2->f0111_door_front_skips_item_explosion : -1, 1,
                     "ReDMCSB DUNVIEW.C:4218-4334 F0111; 6837-6872 no door-front F0115");
    ok &= expect_int("door_front_skip_items_d2r2",
                     d2r2 ? d2r2->f0111_door_front_skips_item_explosion : -1, 1,
                     "ReDMCSB DUNVIEW.C:4218-4334 F0111; 6868-6896 no door-front F0115");
    ok &= expect_int("d2l2.wall_zone",
                     d2l2 ? d2l2->wall_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6851/6858; DEFS.H:4047");
    ok &= expect_int("d2r2.wall_zone",
                     d2r2 ? d2r2->wall_zone : -1, 708,
                     "ReDMCSB DUNVIEW.C:6882/6889; DEFS.H:4048");
    ok &= expect_int("non_f0107_d2l2",
                     d2l2 ? d2l2->non_f0107_back_wall_ornament_contract : -1, 1,
                     "ReDMCSB DUNVIEW.C:6837-6872 returns/skips F0107");
    ok &= expect_int("non_f0107_d2r2",
                     d2r2 ? d2r2->non_f0107_back_wall_ornament_contract : -1, 1,
                     "ReDMCSB DUNVIEW.C:6868-6896 returns/skips F0107");
    ok &= expect_int("shared.object.no_d2_spec", shared_d2 == NULL, 1,
                     "ReDMCSB DUNVIEW.C:6837-6896 D2 dispatcher has no F0115 object route");
    ok &= expect_int("shared.explosion.no_d2_spec", shared_exp_d2 == NULL, 1,
                     "ReDMCSB DUNVIEW.C:6837-6896 D2 dispatcher has no F0115 explosion route");
    ok &= expect_int("fluxcage.defers",
                     d2l2 ? d2l2->fluxcage_defers_to_field : -1, 1,
                     "ReDMCSB DUNVIEW.C:6202-6219 fluxcage F0113 deferral");
    ok &= expect_int("fluxcage.d2l2.field_zone",
                     d2l2 ? d2l2->fluxcage_field_zone : -1, 707,
                     "ReDMCSB DUNVIEW.C:6219 C702 + G2035[9]; DEFS.H:4047");
    ok &= expect_int("fluxcage.d2r2.field_zone",
                     d2r2 ? d2r2->fluxcage_field_zone : -1, 708,
                     "ReDMCSB DUNVIEW.C:6219 C702 + G2035[10]; DEFS.H:4048");

    return ok;
}

static int test_c10_blit_and_lineage(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2l2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(9);
    const CSB_V1_ViewportD2L2D2R2F0115ItemExplosionSpec *d2r2 =
        csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_spec_for_square_pc34(10);
    uint8_t source[8] = { 1, 10, 2, 10, 3, 4, 10, 5 };
    uint8_t destination[8] = { 77, 77, 77, 77, 77, 77, 77, 77 };

    /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH is the F0791 transparent color
     * used by DUNVIEW.C:5109 for items and 6192-6193 for explosions. */
    ok &= expect_int("c10.value", d2l2 ? d2l2->transparent_color : -1, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");
    ok &= expect_int("c10.blit_copied",
                     csb_v1_viewport_d2l2_d2r2_f0115_apply_c10_blit_pc34(
                         d2l2, source, 4, destination, 4, 4, 2),
                     5,
                     "ReDMCSB DUNVIEW.C:5109/6192-6193 F0791 C10 blit");
    ok &= expect_int("c10.pixel0", destination[0], 1,
                     "ReDMCSB DUNVIEW.C:5109 item F0791 pixel copy");
    ok &= expect_int("c10.transparent1", destination[1], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent preservation");
    ok &= expect_int("c10.pixel2", destination[2], 2,
                     "ReDMCSB DUNVIEW.C:6192-6193 explosion F0791 pixel copy");
    ok &= expect_int("c10.transparent3", destination[3], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent preservation");
    ok &= expect_int("c10.pixel5", destination[5], 4,
                     "ReDMCSB DUNVIEW.C:5109/6192-6193 F0791 pixel copy");
    ok &= expect_int("c10.transparent6", destination[6], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent preservation");
    ok &= expect_int("c10.reject_null",
                     csb_v1_viewport_d2l2_d2r2_f0115_apply_c10_blit_pc34(
                         NULL, source, 4, destination, 4, 4, 2),
                     -1,
                     "ReDMCSB DUNVIEW.C:5109 requires resolved F0115 metadata");

    /* CSB-lineage bindings are recorded alongside the ReDMCSB dispatch they
     * represent, not used as a replacement for the source-lock constants. */
    ok &= expect_int("lineage.rf2l2", d2l2 ? d2l2->csb_lineage_relative_cell : -1, 3,
                     "ReDMCSB DEFS.H:2605; CSB Viewport.cpp:334 RF2L2");
    ok &= expect_int("lineage.rf2r2", d2r2 ? d2r2->csb_lineage_relative_cell : -1, 11,
                     "ReDMCSB DEFS.H:2606; CSB Viewport.cpp:339 RF2R2");
    ok &= expect_int("lineage.f2l_contents",
                     d2l2 ? d2l2->csb_lineage_contents_opcode : -1, 60122,
                     "ReDMCSB DUNVIEW.C:8503; CSB Viewport.cpp:508 F2L1Contents");
    ok &= expect_int("lineage.f2r_contents",
                     d2r2 ? d2r2->csb_lineage_contents_opcode : -1, 60124,
                     "ReDMCSB DUNVIEW.C:8507; CSB Viewport.cpp:510 F2R1Contents");
    ok &= expect_int("lineage.draw_room_objects",
                     d2l2 ? d2l2->csb_lineage_std_draw_room_objects_opcode : -1, 60006,
                     "ReDMCSB DUNVIEW.C:4567-4581; CSB Viewport.cpp:379");
    ok &= expect_int("lineage.rear_order",
                     d2l2 ? d2l2->csb_lineage_door_front_rear_order_opcode : -1, 60279,
                     "ReDMCSB DUNVIEW.C:6271/6338 contrast; CSB Viewport.cpp:681 DrawOrder218");
    ok &= expect_int("lineage.front_order",
                     d2r2 ? d2r2->csb_lineage_door_front_front_order_opcode : -1, 60283,
                     "ReDMCSB DUNVIEW.C:6273/6340 contrast; CSB Viewport.cpp:685 DrawOrder128");

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_source_evidence_pc34();

    ok &= expect_contains("evidence.item_rows", e, "4806-4811",
                          "ReDMCSB DUNVIEW.C:4806-4811");
    ok &= expect_contains("evidence.c2500", e, "C2500_ZONE_ | MASK0x8000",
                          "ReDMCSB DUNVIEW.C:5075; DEFS.H:3517/4228");
    ok &= expect_contains("evidence.pile_shift", e, "advance/wrap the pile shift",
                          "ReDMCSB DUNVIEW.C:5082-5089");
    ok &= expect_contains("evidence.explosion_restart", e, "5915-5933",
                          "ReDMCSB DUNVIEW.C:5915-5933");
    ok &= expect_contains("evidence.g2034", e, "G2034/G2035",
                          "ReDMCSB DUNVIEW.C:5920-5924");
    ok &= expect_contains("evidence.explosion_zones", e, "C3000/C3007/C3014/C3031",
                          "ReDMCSB DEFS.H:4232-4235");
    ok &= expect_contains("evidence.fluxcage", e, "defer fluxcage to F0113",
                          "ReDMCSB DUNVIEW.C:6202-6219");
    ok &= expect_contains("evidence.no_d2_dispatch", e, "no D2L2/D2R2 F0115/F0111",
                          "ReDMCSB DUNVIEW.C:6837-6896");
    ok &= expect_contains("evidence.f0111", e, "4218-4334 F0111",
                          "ReDMCSB DUNVIEW.C:4218-4334");
    ok &= expect_contains("evidence.c10", e, "DEFS.H:2088",
                          "ReDMCSB DEFS.H:2088");
    ok &= expect_contains("evidence.coord_item", e, "COORD.C:1058-1123/1129-1193",
                          "ReDMCSB COORD.C:1058-1193");
    ok &= expect_contains("evidence.lineage", e, "CSB Viewport.cpp",
                          "ReDMCSB DUNVIEW.C:8503-8508; CSB Viewport.cpp:334/339");

    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=csb_v1_viewport_d2l2_d2r2_f0115_item_explosion_pc34_compat\n");
    ok &= test_d2_route_metadata();
    ok &= test_item_zone_contract();
    ok &= test_explosion_zone_contract();
    ok &= test_f0111_f0107_fluxcage_dispatch();
    ok &= test_c10_blit_and_lineage();
    ok &= test_source_evidence();

    printf("%s assertions=%d\n", ok ? "PASS" : "FAIL", g_assertions);
    return ok ? 0 : 1;
}
