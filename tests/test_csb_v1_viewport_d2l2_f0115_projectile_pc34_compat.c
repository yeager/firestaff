#include "csb_v1_viewport_d2l2_f0115_projectile_pc34_compat.h"

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

static int test_route_rows_and_teleporters(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2l2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(9);
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2r2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(10);

    /* ReDMCSB: DEFS.H lines 2605-2606 define C09/C10; DUNVIEW.C lines
     * 371-377 map D2L2/D2R2 to depth 2, missing F0115 rows, and field
     * aspects 5/6; F0678/F0679 lines 6863-6865/6894-6896 draw only F0113. */
    ok &= expect_int("route.count",
                     (int)M11_GameView_D2L2F0115ProjectileRouteSpecCount(), 2,
                     "ReDMCSB DUNVIEW.C:6847-6896");
    ok &= expect_int("d2l2.present", d2l2 != NULL, 1,
                     "ReDMCSB DEFS.H:2605 C09_VIEW_SQUARE_D2L2");
    ok &= expect_int("d2r2.present", d2r2 != NULL, 1,
                     "ReDMCSB DEFS.H:2606 C10_VIEW_SQUARE_D2R2");
    ok &= expect_int("d2l2.depth", d2l2 ? d2l2->view_depth : -99, 2,
                     "ReDMCSB DUNVIEW.C:372 G2027[9]");
    ok &= expect_int("d2r2.depth", d2r2 ? d2r2->view_depth : -99, 2,
                     "ReDMCSB DUNVIEW.C:372 G2027[10]");
    ok &= expect_int("d2l2.field_aspect", d2l2 ? d2l2->field_aspect_index : -99, 5,
                     "ReDMCSB DUNVIEW.C:377 G2035[9]");
    ok &= expect_int("d2r2.field_aspect", d2r2 ? d2r2->field_aspect_index : -99, 6,
                     "ReDMCSB DUNVIEW.C:377 G2035[10]");
    ok &= expect_int("d2l2.field_zone",
                     M11_GameView_D2L2F0115ProjectileTeleporterFieldZone(d2l2), 707,
                     "ReDMCSB DUNVIEW.C:6863-6865 C707_ZONE_WALL_D2L2");
    ok &= expect_int("d2r2.field_zone",
                     M11_GameView_D2L2F0115ProjectileTeleporterFieldZone(d2r2), 708,
                     "ReDMCSB DUNVIEW.C:6894-6896 C708_ZONE_WALL_D2R2");
    ok &= expect_int("unknown.square",
                     M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(14) == NULL, 1,
                     "D2L2/D2R2-only gate");

    return ok;
}

static int test_projectile_gate(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2l2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(9);
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2r2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(10);

    /* ReDMCSB: F0115 lines 5668-5683 select G2028 and then compute
     * C2900_ZONE_ + row*4 + ViewCell only when the row is non-negative. */
    ok &= expect_int("d2l2.g2028_row", d2l2 ? d2l2->projectile_g2028_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:373 G2028[9]");
    ok &= expect_int("d2r2.g2028_row", d2r2 ? d2r2->projectile_g2028_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:373 G2028[10]");
    ok &= expect_int("projectile.zone_base", d2l2 ? d2l2->projectile_zone_base : -99, 2900,
                     "ReDMCSB DEFS.H:4230 C2900_ZONE_");
    ok &= expect_int("projectile.zone_stride", d2l2 ? d2l2->projectile_zone_cell_stride : -99, 4,
                     "ReDMCSB DUNVIEW.C:5683 row*4+ViewCell");
    ok &= expect_int("projectile.restart", d2l2 ? d2l2->projectile_restarts_thing_list : -99, 1,
                     "ReDMCSB DUNVIEW.C:5679");
    ok &= expect_int("projectile.type_c14", d2l2 ? d2l2->projectile_requires_type_c14 : -99, 1,
                     "ReDMCSB DUNVIEW.C:5681 C14_THING_TYPE_PROJECTILE");
    ok &= expect_int("projectile.cell_match", d2l2 ? d2l2->projectile_requires_cell_match : -99, 1,
                     "ReDMCSB DUNVIEW.C:5681 M011_CELL == L0139");
    ok &= expect_int("projectile.depth3_front_skip",
                     d2l2 ? d2l2->projectile_suppresses_depth3_front_cells : -99, 0,
                     "ReDMCSB DUNVIEW.C:5672 applies only at depth 3");
    ok &= expect_int("projectile.depth0_back_skip",
                     d2l2 ? d2l2->projectile_suppresses_depth0_back_cells : -99, 0,
                     "ReDMCSB DUNVIEW.C:5675 applies only at depth 0");
    ok &= expect_int("projectile.d2_front_cell_gate",
                     M11_GameView_D2L2F0115ProjectileZone(d2l2, 0), -1,
                     "ReDMCSB DUNVIEW.C:5668-5670 rejects missing D2L2 row");
    ok &= expect_int("projectile.d2_back_cell_gate",
                     M11_GameView_D2L2F0115ProjectileZone(d2r2, 3), -1,
                     "ReDMCSB DUNVIEW.C:5668-5670 rejects missing D2R2 row");
    ok &= expect_int("projectile.bad_cell",
                     M11_GameView_D2L2F0115ProjectileZone(d2l2, 4), -1,
                     "ReDMCSB DUNVIEW.C:5683 has four view cells");
    ok &= expect_int("projectile.null_zone",
                     M11_GameView_D2L2F0115ProjectileZone(NULL, 0), -1,
                     "route helper rejects unresolved spec");
    ok &= expect_int("projectile.cm1_none",
                     d2l2 ? d2l2->projectile_derived_bitmap_none : 0, -1,
                     "ReDMCSB DUNVIEW.C:5859 CM1_DERIVED_BITMAP_NONE");
    ok &= expect_int("projectile.f0791",
                     d2l2 ? d2l2->projectile_uses_f0791_blit : -99, 1,
                     "ReDMCSB DUNVIEW.C:5881-5882 F0791");
    ok &= expect_int("projectile.c10",
                     d2l2 ? d2l2->projectile_transparent_color : -99, 10,
                     "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH");

    return ok;
}

static int test_object_creature_explosion_gates(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2l2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(9);
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2r2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecForSquare(10);

    /* ReDMCSB: F0115 lines 4923 and 5071-5079 gate object blits through
     * C2500|MASK0x8000, while lines 5201-5214/5615-5627 gate creatures
     * through G2033 and C3200|MASK0x8000.  D2L2/D2R2 rows are absent. */
    ok &= expect_int("object.row", d2l2 ? d2l2->object_g2028_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:373/4811 G2028[9]");
    ok &= expect_int("object.zone_base", d2l2 ? d2l2->object_zone_base : -99, 2500,
                     "ReDMCSB DEFS.H:4228 C2500_ZONE_");
    ok &= expect_int("object.shift_mask", d2l2 ? d2l2->shift_objects_and_creatures_mask : -99, 0x8000,
                     "ReDMCSB DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES");
    ok &= expect_int("object.zone_rejected",
                     M11_GameView_D2L2F0115ProjectileObjectZone(d2l2, 2), -1,
                     "ReDMCSB DUNVIEW.C:4923 L2476_i_ >= 0");
    ok &= expect_int("object.pile_shift", d2l2 ? d2l2->object_pile_shift_advances : -99, 1,
                     "ReDMCSB DUNVIEW.C:5077-5082 pile shift");
    ok &= expect_int("creature.row", d2l2 ? d2l2->creature_g2033_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:375 G2033[9]");
    ok &= expect_int("creature.group_marker", d2l2 ? d2l2->creature_requires_group_marker : -99, 1,
                     "ReDMCSB DUNVIEW.C:4840-4842 C04_THING_TYPE_GROUP");
    ok &= expect_int("creature.zone_base", d2l2 ? d2l2->creature_zone_base : -99, 3200,
                     "ReDMCSB DEFS.H:4236 C3200_ZONE_");
    ok &= expect_int("creature.coord_stride", d2l2 ? d2l2->creature_coordinate_set_stride : -99, 65,
                     "ReDMCSB DUNVIEW.C:5616 CoordinateSet*65");
    ok &= expect_int("creature.zone_rejected",
                     M11_GameView_D2L2F0115ProjectileCreatureZone(d2l2, 1, 2), -1,
                     "ReDMCSB DUNVIEW.C:5211-5213 rejects G2033<0");

    /* ReDMCSB: F0115 lines 5915-5933 restart the explosion pass; lines
     * 5920-5924 keep field aspects but leave D2L2/D2R2 explosion rows -1. */
    ok &= expect_int("explosion.row", d2r2 ? d2r2->explosion_g2034_row : -99, -1,
                     "ReDMCSB DUNVIEW.C:376 G2034[10]");
    ok &= expect_int("explosion.restart", d2l2 ? d2l2->explosion_restarts_thing_list_after_cells : -99, 1,
                     "ReDMCSB DUNVIEW.C:5931");
    ok &= expect_int("explosion.step1_base", d2l2 ? d2l2->explosion_rebirth_step1_zone_base : -99, 3000,
                     "ReDMCSB DEFS.H:4232 C3000_ZONE_");
    ok &= expect_int("explosion.step2_base", d2l2 ? d2l2->explosion_rebirth_step2_zone_base : -99, 3007,
                     "ReDMCSB DEFS.H:4233 C3007_ZONE_");
    ok &= expect_int("explosion.centered_base", d2l2 ? d2l2->explosion_centered_zone_base : -99, 3014,
                     "ReDMCSB DEFS.H:4234 C3014_ZONE_");
    ok &= expect_int("explosion.side_base", d2l2 ? d2l2->explosion_side_zone_base : -99, 3031,
                     "ReDMCSB DEFS.H:4235 C3031_ZONE_");
    ok &= expect_int("explosion.step1_rejected",
                     M11_GameView_D2L2F0115ProjectileExplosionRebirthStep1Zone(d2l2), -1,
                     "ReDMCSB DUNVIEW.C:5948 rebirth rejects row <0");
    ok &= expect_int("explosion.step2_rejected",
                     M11_GameView_D2L2F0115ProjectileExplosionRebirthStep2Zone(d2l2), -1,
                     "ReDMCSB DUNVIEW.C:6094-6096 row unavailable");
    ok &= expect_int("explosion.center_rejected",
                     M11_GameView_D2L2F0115ProjectileExplosionCenteredZone(d2l2), -1,
                     "ReDMCSB DUNVIEW.C:6106-6107 row unavailable");
    ok &= expect_int("explosion.side_rejected",
                     M11_GameView_D2L2F0115ProjectileExplosionSideZone(d2l2, 1), -1,
                     "ReDMCSB DUNVIEW.C:6121-6122 row unavailable");
    ok &= expect_int("fluxcage.defers",
                     d2l2 ? d2l2->fluxcage_defers_to_field : -99, 1,
                     "ReDMCSB DUNVIEW.C:6202-6219");
    ok &= expect_int("fluxcage.field_zone",
                     d2r2 ? d2r2->fluxcage_field_zone : -99, 708,
                     "ReDMCSB DUNVIEW.C:6219 C702 + field aspect 6");

    return ok;
}

static int test_door_order_absence_and_synthetic_blit(void)
{
    int ok = 1;
    const CSB_V1_ViewportD2L2F0115ProjectileRouteSpec *d2l2 =
        M11_GameView_D2L2F0115ProjectileRouteSpecAt(0);
    uint8_t source[6] = { 1, 10, 2, 10, 3, 4 };
    uint8_t destination[6] = { 77, 77, 77, 77, 77, 77 };

    /* ReDMCSB: F0678/F0679 lines 6847-6896 have no C17 door-front case,
     * unlike F0676/F0677 lines 6271-6273/6338-6340 where F0115 brackets
     * F0111 for D3L2/D3R2. */
    ok &= expect_int("d2.no_f0115_route",
                     d2l2 ? d2l2->f0678_f0679_has_f0115_route : -99, 0,
                     "ReDMCSB DUNVIEW.C:6847-6896 no F0115 call");
    ok &= expect_int("d2.no_rear_pass",
                     d2l2 ? d2l2->door_front_rear_f0115_order : -99, 0,
                     "ReDMCSB DUNVIEW.C:6847-6896 no C17 door-front case");
    ok &= expect_int("d2.no_f0111",
                     d2l2 ? d2l2->door_front_f0111_order : -99, 0,
                     "ReDMCSB DUNVIEW.C:6847-6896 no F0111 call");
    ok &= expect_int("d2.no_front_pass",
                     d2l2 ? d2l2->door_front_front_f0115_order : -99, 0,
                     "ReDMCSB DUNVIEW.C:6847-6896 no F0115 pass2");

    /* ReDMCSB: F0115 lines 5881-5882 dispatch projectile bitmaps via F0791
     * with C10 transparency when a visible row exists; the D2 route gate keeps
     * the missing-row block separate from this synthetic C10 copy contract. */
    ok &= expect_int("blit.copied",
                     M11_GameView_D2L2F0115ProjectileApplySyntheticC10Blit(
                         d2l2, source, 3, destination, 3, 3, 2),
                     4,
                     "ReDMCSB DUNVIEW.C:5881-5882 F0791 C10");
    ok &= expect_int("blit.pixel0", destination[0], 1,
                     "synthetic F0791 pixel copy");
    ok &= expect_int("blit.transparent1", destination[1], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("blit.pixel2", destination[2], 2,
                     "synthetic F0791 pixel copy");
    ok &= expect_int("blit.transparent3", destination[3], 77,
                     "ReDMCSB DEFS.H:2088 C10 transparent");
    ok &= expect_int("blit.pixel4", destination[4], 3,
                     "synthetic F0791 pixel copy");
    ok &= expect_int("blit.reject_null",
                     M11_GameView_D2L2F0115ProjectileApplySyntheticC10Blit(
                         NULL, source, 3, destination, 3, 3, 2),
                     -1,
                     "route helper rejects unresolved spec");

    return ok;
}

static int test_source_evidence(void)
{
    int ok = 1;
    const char *e = M11_GameView_D2L2F0115ProjectileSourceEvidence();

    ok &= expect_contains("evidence.f0115", e, "F0115:5668-5683",
                          "ReDMCSB DUNVIEW.C F0115 projectile gate");
    ok &= expect_contains("evidence.c2900", e, "C2900_ZONE_",
                          "ReDMCSB DEFS.H:4230");
    ok &= expect_contains("evidence.c2500_mask", e, "C2500_ZONE_ | MASK0x8000",
                          "ReDMCSB DUNVIEW.C:5071-5079");
    ok &= expect_contains("evidence.c3200_mask", e, "C3200_ZONE_ | MASK0x8000",
                          "ReDMCSB DUNVIEW.C:5615-5627");
    ok &= expect_contains("evidence.explosion_zones", e, "C3000/C3007/C3014/C3031",
                          "ReDMCSB DEFS.H:4232-4235");
    ok &= expect_contains("evidence.fluxcage", e, "defers fluxcage to F0113",
                          "ReDMCSB DUNVIEW.C:6202-6219");
    ok &= expect_contains("evidence.no_d2_f0115", e, "no D2L2/D2R2 F0115",
                          "ReDMCSB DUNVIEW.C:6847-6896");

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_route_rows_and_teleporters();
    ok &= test_projectile_gate();
    ok &= test_object_creature_explosion_gates();
    ok &= test_door_order_absence_and_synthetic_blit();
    ok &= test_source_evidence();

    printf("%s assertions=%d\n", ok ? "PASS" : "FAIL", g_assertions);
    return ok ? 0 : 1;
}
