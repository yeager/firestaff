#include "dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static const DM1_V1_D2LD2RF0098OrderStepPc34 *
find_step(const DM1_V1_D2LD2RF0098OrderStepPc34 *steps,
          size_t count,
          DM1_V1_D2LD2RF0098OpPc34 op)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (steps[i].op == op) return &steps[i];
    }
    return NULL;
}

static int graphics_dat_available(void)
{
    const char *env = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char *path = env ? env :
        "/Volumes/Extern-disk/openclaw-data/firestaff/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DATA/GRAPHICS.DAT";
    unsigned char first[4];
    FILE *f = fopen(path, "rb");
    long size = 0;
    size_t got;

    if (!f) return 0;
    got = fread(first, 1, sizeof(first), f);
    if (fseek(f, 0, SEEK_END) == 0) {
        size = ftell(f);
    }
    fclose(f);
    return got == sizeof(first) && size == 363417L &&
        first[0] == 0x01 && first[1] == 0x80 && first[2] == 0xc9 && first[3] == 0x02;
}

static void test_specs(void)
{
    const DM1_V1_D2LD2RF0098FallbackSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_f0098_fallback_spec_pc34(
            DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RF0098FallbackSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_f0098_fallback_spec_pc34(
            DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34);

    expect_int("spec.d2l.present", d2l != NULL, 1, "DUNVIEW.C:8512-8513 D2L dispatch");
    expect_int("spec.d2r.present", d2r != NULL, 1, "DUNVIEW.C:8516-8517 D2R dispatch");
    if (!d2l || !d2r) return;

    expect_int("spec.d2l.view_square", d2l->view_square_index, 4,
               "DEFS.H:2582 M604_VIEW_SQUARE_D2L");
    expect_int("spec.d2r.view_square", d2r->view_square_index, 5,
               "DEFS.H:2583 M605_VIEW_SQUARE_D2R");
    expect_int("spec.d2l.depth", d2l->depth, 2, "DUNVIEW.C:8512 relative depth");
    expect_int("spec.d2r.depth", d2r->depth, 2, "DUNVIEW.C:8516 relative depth");
    expect_int("spec.d2l.lateral", d2l->lateral, -1, "DUNVIEW.C:8512 relative lateral");
    expect_int("spec.d2r.lateral", d2r->lateral, 1, "DUNVIEW.C:8516 relative lateral");
    expect_int("spec.d2l.view_floor", d2l->view_floor_index, 5,
               "DEFS.H:2755 M591_VIEW_FLOOR_D2L");
    expect_int("spec.d2r.view_floor", d2r->view_floor_index, 7,
               "DEFS.H:2757 M593_VIEW_FLOOR_D2R");
    expect_int("spec.ceiling_zone", d2l->viewport_ceiling_zone, 700,
               "DEFS.H:4040 C700_ZONE_VIEWPORT_CEILING_AREA");
    expect_int("spec.floor_zone", d2l->viewport_floor_zone, 701,
               "DEFS.H:4041 C701_ZONE_VIEWPORT_FLOOR_AREA");
    expect_int("spec.d2l.wall_zone_ref", d2l->wall_zone_index, 710,
               "DEFS.H:4050 C710_ZONE_WALL_D2L");
    expect_int("spec.d2r.wall_zone_ref", d2r->wall_zone_index, 711,
               "DEFS.H:4051 C711_ZONE_WALL_D2R");
    expect_int("spec.d2l.floor_pit_zone", d2l->floor_pit_zone, 855,
               "DEFS.H:4202 C855_ZONE_FLOORPIT_D2L");
    expect_int("spec.d2r.floor_pit_zone", d2r->floor_pit_zone, 857,
               "DEFS.H:4204 C857_ZONE_FLOORPIT_D2R");
    expect_int("spec.d2l.ceiling_pit_zone", d2l->ceiling_pit_zone, 864,
               "DEFS.H:4211 C864_ZONE_CEILING_PIT_D2L");
    expect_int("spec.d2r.ceiling_pit_zone", d2r->ceiling_pit_zone, 866,
               "DEFS.H:4213 C866_ZONE_CEILING_PIT_D2R");
    expect_int("spec.d2l.open_cell_order", d2l->corridor_cell_order, 0x3421,
               "DUNVIEW.C:7018 C0x3421 open path");
    expect_int("spec.d2r.open_cell_order", d2r->corridor_cell_order, 0x4312,
               "DUNVIEW.C:7211 C0x4312 open path");
    expect_int("spec.non_wall_only", d2l->side_cell_must_not_be_wall &&
               d2r->side_cell_must_not_be_wall, 1,
               "DUNVIEW.C:6945-6973/7096-7166 wall cases are excluded");
    expect_int("spec.no_f0107_nonwall", d2l->f0107_wall_ornament_excluded_on_non_wall_path &&
               d2r->f0107_wall_ornament_excluded_on_non_wall_path, 1,
               "DUNVIEW.C F0119/F0120 non-wall branch avoids F0107");
    expect_contains("spec.graphics_sha", d2l->graphics_dat_sha256,
                    "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
                    "canonical DM1 PC34 GRAPHICS.DAT");
    expect_contains("spec.floor_bitmap", d2l->floor_bitmap_symbol, "G2108_Floor",
                    "DUNVIEW.C F0098 line 2996 floor source");
    expect_contains("spec.ceiling_bitmap", d2l->ceiling_bitmap_symbol, "G2109_Ceiling",
                    "DUNVIEW.C F0098 line 2995 ceiling source");
}

static void test_order_for_non_wall_corridor_and_pit(void)
{
    const DM1_V1_D2LD2RF0098OrderStepPc34 *d2l_steps;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *d2r_steps;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *f0098;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *f0099;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *dispatch;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *f0115;
    const DM1_V1_D2LD2RF0098OrderStepPc34 *f0097;
    size_t d2l_count = 0;
    size_t d2r_count = 0;

    d2l_steps = dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34(
        DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34,
        DM1_V1_D2L_D2R_F0098_ELEMENT_CORRIDOR_PC34,
        &d2l_count);
    d2r_steps = dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34(
        DM1_V1_D2L_D2R_F0098_SIDE_D2R_PC34,
        DM1_V1_D2L_D2R_F0098_ELEMENT_CORRIDOR_PC34,
        &d2r_count);
    expect_int("order.d2l.count", (int)d2l_count, 7, "DUNVIEW.C F0119 open path");
    expect_int("order.d2r.count", (int)d2r_count, 7, "DUNVIEW.C F0120 open path");

    f0098 = find_step(d2l_steps, d2l_count, DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34);
    f0099 = find_step(d2l_steps, d2l_count, DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34);
    dispatch = find_step(d2l_steps, d2l_count, DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34);
    f0115 = find_step(d2l_steps, d2l_count, DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34);
    f0097 = find_step(d2l_steps, d2l_count, DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34);
    expect_int("order.d2l.f0098.before_f0099", f0098 && f0099 &&
               f0098->order_index < f0099->order_index, 1,
               "DUNVIEW.C:8337-8431 F0098 before F0099 flip work");
    expect_int("order.d2l.f0099.before_dispatch", f0099 && dispatch &&
               f0099->order_index < dispatch->order_index, 1,
               "DUNVIEW.C:8431 then 8512-8513 D2L");
    expect_int("order.d2l.dispatch.before_f0115", dispatch && f0115 &&
               dispatch->order_index < f0115->order_index, 1,
               "DUNVIEW.C:7031 D2L F0115 after non-wall body");
    expect_int("order.d2l.f0115.before_f0097", f0115 && f0097 &&
               f0115->order_index < f0097->order_index, 1,
               "DUNVIEW.C:8606-8610 present after drawing");
    expect_int("order.d2l.no_f0107", find_step(d2l_steps, d2l_count,
               DM1_V1_D2L_D2R_F0098_OP_F0111_DOOR_PC34) == NULL, 1,
               "DUNVIEW.C:6968-6973 F0107 belongs to wall_return branch");

    f0098 = find_step(d2r_steps, d2r_count, DM1_V1_D2L_D2R_F0098_OP_F0098_FLOOR_CEILING_PC34);
    f0099 = find_step(d2r_steps, d2r_count, DM1_V1_D2L_D2R_F0098_OP_F0099_FLOOR_CEILING_FLIP_PC34);
    dispatch = find_step(d2r_steps, d2r_count, DM1_V1_D2L_D2R_F0098_OP_F0119_OR_F0120_NON_WALL_PC34);
    f0115 = find_step(d2r_steps, d2r_count, DM1_V1_D2L_D2R_F0098_OP_F0115_THINGS_PC34);
    f0097 = find_step(d2r_steps, d2r_count, DM1_V1_D2L_D2R_F0098_OP_F0097_PRESENT_PC34);
    expect_int("order.d2r.f0098.before_f0099", f0098 && f0099 &&
               f0098->order_index < f0099->order_index, 1,
               "DUNVIEW.C:8337-8431 F0098 before F0099 flip work");
    expect_int("order.d2r.dispatch.before_f0115", dispatch && f0115 &&
               dispatch->order_index < f0115->order_index, 1,
               "DUNVIEW.C:7224 D2R F0115 after non-wall body");
    expect_int("order.d2r.f0115.before_f0097", f0115 && f0097 &&
               f0115->order_index < f0097->order_index, 1,
               "DUNVIEW.C:8606-8610 present after drawing");

    d2l_steps = dm1_v1_viewport_d2l_d2r_f0098_fallback_order_pc34(
        DM1_V1_D2L_D2R_F0098_SIDE_D2L_PC34,
        DM1_V1_D2L_D2R_F0098_ELEMENT_PIT_PC34,
        &d2l_count);
    expect_int("order.d2l.pit.has_f0104", find_step(d2l_steps, d2l_count,
               DM1_V1_D2L_D2R_F0098_OP_F0104_FLOOR_PIT_OR_STAIRS_PC34) != NULL, 1,
               "DUNVIEW.C:7013 C855 floor pit before F0108");
}

static void test_f0098_precondition_and_evidence(void)
{
    const char *e = dm1_v1_viewport_d2l_d2r_f0098_fallback_source_evidence_pc34();

    expect_int("precondition.dirty_true",
               dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(true) ? 1 : 0,
               1, "DUNVIEW.C:8337 G0297 dirty flag");
    expect_int("precondition.dirty_false",
               dm1_v1_viewport_d2l_d2r_f0098_should_draw_pc34(false) ? 1 : 0,
               0, "DUNVIEW.C:8337 G0297 dirty flag");
    expect_contains("evidence.f0098", e, "F0098:2962-3002",
                    "DUNVIEW.C F0098 line 2962");
    expect_contains("evidence.f0128_dirty", e, "F0128:8337-8338",
                    "DUNVIEW.C F0128 dirty flag guard");
    expect_contains("evidence.f0128_order", e, "F0128:8512-8517",
                    "DUNVIEW.C F0128 D2L/D2R dispatch order");
    expect_contains("evidence.f0097", e, "F0128:8606-8610",
                    "DUNVIEW.C F0097 present");
    expect_contains("evidence.f0119_nonwall", e, "F0119:6900-7049 D2L non-wall",
                    "DUNVIEW.C F0119 non-wall branch");
    expect_contains("evidence.f0120_nonwall", e, "F0120:7051-7220 D2R",
                    "DUNVIEW.C F0120 non-wall branch");
    expect_contains("evidence.no_f0107", e, "without the C00 wall_return/F0107 branch",
                    "DUNVIEW.C F0119/F0120 wall branch excluded");
    expect_contains("evidence.defs_zones", e, "DEFS.H:4040-4051",
                    "DEFS.H zone ids for D2L/D2R");
}

static void test_real_asset_fixture_is_reachable(void)
{
    expect_int("asset.graphics_dat_reachable", graphics_dat_available(), 1,
               "canonical DM1 PC34 GRAPHICS.DAT real-asset fixture");
}

int main(void)
{
    test_specs();
    test_order_for_non_wall_corridor_and_pit();
    test_f0098_precondition_and_evidence();
    test_real_asset_fixture_is_reachable();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_d2l_d2r_f0098_fallback_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
